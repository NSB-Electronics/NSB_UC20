#include "ssl.h"

SSL::SSL() {
	
}

bool SSL::setSSLversion(uint8_t contextid, uint8_t sslver) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=\"sslversion\",%d,%d", CMD_SSL_CONF, contextid, sslver);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool SSL::setCiphersuite(uint8_t contextid, char *cipher) {
	bool responseOK = false;
	char cmd[50];
	
	snprintf(cmd, sizeof(cmd), "%s=\"ciphersuite\",%d,%s", CMD_SSL_CONF, contextid, cipher);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool SSL:: setSeclevel(uint8_t contextid, uint8_t level) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=\"seclevel\",%d,%d", CMD_SSL_CONF, contextid, level);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool SSL::setCertificate(uint8_t contextid, char *cacertpath) {
	bool responseOK = false;
	char cmd[100];
	if (strlen(cacertpath) <= 50) {
		snprintf(cmd, sizeof(cmd), "%s=\"cacert\",%d,\"%s\"", CMD_SSL_CONF, contextid, cacertpath);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		// Certificate path too long
	}
	return responseOK;
}

bool SSL::setIgnorelocaltime(uint8_t contextid, bool ignoretime) {
	bool responseOK = false;
	char cmd[50];
	
	snprintf(cmd, sizeof(cmd), "%s=\"ignorelocaltime\",%d,%d", CMD_SSL_CONF, contextid, ignoretime);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}


uint16_t SSL::open(uint8_t pdpid, uint8_t contextid, uint8_t clientid, char *serverAddr, uint16_t port, uint8_t accessMode) {	
	bool responseOK = false;
	char cmd[100];
	uint16_t errorCode = 0;
	
	snprintf(cmd, sizeof(cmd), "%s=%d,%d,%d,\"%s\",%d,%d", CMD_SSL_OPEN, pdpid, contextid, clientid, serverAddr, port, accessMode);
	
	if ((accessMode == SSL_BUFFER_MODE) || (accessMode == SSL_DIRECT_MODE)) {
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else if (accessMode == SSL_TRANSP_MODE) {
		responseOK = gsm.sendCheckReply(cmd, AVT1_CONNECT);
	} else {
		responseOK = false;
	}
	
	if (responseOK) {
		responseOK = gsm.checkReply(REPLY_SSL_OPEN, 150000);
		if (responseOK) {
			responseOK = gsm.parseReply(REPLY_SSL_OPEN, &errorCode, ',', 1);
			if ((responseOK) && (errorCode == 0)) {
				responseOK = true;
			} else {
				responseOK = false;
			}
		} else {
			responseOK = false;
		}
	} else {
		responseOK = false;
	}
	return errorCode;
}

bool SSL::close(uint8_t clientid, uint16_t closeTimeout) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=%d,%d", CMD_SSL_CLOSE, clientid, closeTimeout);
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool SSL::send(uint8_t clientid, uint8_t *data, uint16_t sendLen) {
	bool sendOK = false;
	char cmd[30];
	char tmp;
	
	if (sendLen == 0) {
		snprintf(cmd, sizeof(cmd), "%s=%d", CMD_SSL_SEND, clientid);
	} else {
		snprintf(cmd, sizeof(cmd), "%s=%d,%d", CMD_SSL_SEND, clientid, sendLen);
	}
	
	sendOK = gsm.sendCheckReply(cmd, CMD_CLOSE_BRACKET);
	
	if (sendOK) {
		for (uint16_t len = 0; len < sendLen; len++) {
			gsm.write(*data++);
		}
		
		if (sendLen == 0) {
			gsm.write(0x1A);
		} else {
			// Should close as len sent with command
		}
		sendOK = gsm.checkReply(REPLY_SSL_SEND, 1000);
	} else {
		// Send failed
	}
	
	return sendOK;
}

int16_t SSL::state(uint8_t clientid) {
	bool responseOK = false;
	char cmd[30];
	int16_t socketState = -1;
	
	snprintf(cmd, sizeof(cmd), "%s=%d", CMD_SSL_STATE, clientid);
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_SSL_STATE);
	
	if (responseOK) {
		responseOK = gsm.parseReply(REPLY_SSL_RECV, &socketState, ',', 5);
	} else {
		socketState = -1;
	}
	
	return socketState;
}

bool SSL::receiveAvailable() {
	bool responseOK = false;
	char reply[30];
	snprintf(reply, sizeof(reply), "%s=\"recv\"", REPLY_SSL_URC);
	while ((gsm.available()) && (responseOK == false)) {
		responseOK = gsm.checkReply(reply);
		if (responseOK) {
			responseOK = gsm.parseReply(reply, &receiveClientID, ',', 0);
		} else {
			// Continue
		}
	}
	Serial.print("receiveClientID: ");
	Serial.println(receiveClientID);
	return responseOK;
}

uint16_t SSL::readBuffer(uint8_t clientid, uint16_t readLen) {
	bool responseOK = false;
	char cmd[30];
	uint16_t haveReadLen = 0;
	
	snprintf(cmd, sizeof(cmd), "%s=%d,%d", CMD_SSL_RECV, clientid, readLen);
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_SSL_RECV);
	
	if (responseOK) {
		responseOK = gsm.parseReply(REPLY_SSL_RECV, &haveReadLen, ',', 0);
	} else {
		haveReadLen = 0;
	}
	
	return haveReadLen;
}

uint16_t SSL::readBuffer(uint16_t readLen) {
	return readBuffer(0, readLen);
}

uint16_t SSL::readBuffer() {
	return readBuffer(0, 1500);
}
