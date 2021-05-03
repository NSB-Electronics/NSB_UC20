#include "MQTT.h"

MQTT::MQTT(){
	
}

void MQTT::setID(uint8_t tcpConnectID) {
	_tcpConnectID = tcpConnectID;
}

bool MQTT::configVersion(uint8_t version) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=\"version\",%d,%d", CMD_MQTT_CONF, _tcpConnectID, version);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool MQTT::configPDP(uint8_t cid) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=\"pdpcid\",%d,%d", CMD_MQTT_CONF, _tcpConnectID, cid);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool MQTT::configSSL(bool sslEnable, uint8_t sslctxidx) {
	bool responseOK = false;
	char cmd[30];
	
	if (sslEnable == MQTT_SSL_TCP) {
		snprintf(cmd, sizeof(cmd), "%s=\"ssl\",%d,%d,%d", CMD_MQTT_CONF, _tcpConnectID, sslEnable, sslctxidx);
	} else {
		snprintf(cmd, sizeof(cmd), "%s=\"ssl\",%d,%d", CMD_MQTT_CONF, _tcpConnectID, sslEnable);
	}
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool MQTT::configKeepAlive(uint16_t keepAliveTime) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=\"keepalive\",%d,%d", CMD_MQTT_CONF, _tcpConnectID, keepAliveTime);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool MQTT::configSession(bool cleanSession) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=\"session\",%d,%d", CMD_MQTT_CONF, _tcpConnectID, cleanSession);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool MQTT::configTimeout(uint8_t pktTimeout, uint8_t retryTimes, bool timeoutNotice) {
	bool responseOK = false;
	char cmd[30];
	
	_pktTimeout = pktTimeout;
	_retryTimes = retryTimes;
	
	snprintf(cmd, sizeof(cmd), "%s=\"timeout\",%d,%d,%d,%d", CMD_MQTT_CONF, _tcpConnectID, pktTimeout, retryTimes, timeoutNotice);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool MQTT::configWill(char *willTopic, char *willMsg, bool willFlag, uint8_t willQoS, uint8_t willRetain) {
	bool responseOK = false;
	char cmd[MQTT_MAX_BUF];
	
	if (strlen(willTopic) + strlen(willMsg) < MQTT_MAX_BUF - 30) {
		snprintf(cmd, sizeof(cmd), "%s=\"will\",%d,%d,%d,%d,\"%s\",\"%s\"", CMD_MQTT_CONF, _tcpConnectID, willFlag, willQoS, willRetain, willTopic, willMsg);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		responseOK = false;
		DEBUG_PRINTLN(F("\t<--- Will Topic or Message too long"));
	}
	
	return responseOK;
}

bool MQTT::configRecvMode(bool msgRecvMode) {
	bool responseOK = false;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=\"recv/mode\",%d,%d", CMD_MQTT_CONF, _tcpConnectID, msgRecvMode);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

int8_t MQTT::open(char *hostName, uint16_t port) {
	bool responseOK = false;
	int16_t responseCode = -1;
	char cmd[MQTT_MAX_BUF];
	
	if (strlen(hostName) < MQTT_MAX_BUF - 30) {
		snprintf(cmd, sizeof(cmd), "%s=%d,\"%s\",%d", CMD_MQTT_OPEN, _tcpConnectID, hostName, port);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
		
		if (responseOK) {
			responseOK = gsm.waitReply(REPLY_MQTT_OPEN, MQTT_OPEN_TIMEOUT_MS);
			if (responseOK) {
				responseOK = gsm.parseReply(REPLY_MQTT_OPEN, &responseCode, ',', 1);
			} else {
				responseCode = -1;
			}
		} else {
			// Response not ok
			responseCode = -1;
		}
		
	} else {
		// Hostname too long
		responseCode = -1;
		DEBUG_PRINTLN(F("\t<--- Hostname too long"));
	}
	
	return (int8_t)responseCode;
}

bool MQTT::close() {
	bool responseOK = false;
	int16_t responseCode = -1;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=%d", CMD_MQTT_CLOSE, _tcpConnectID);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	if (responseOK) {
		responseOK = gsm.waitReply(REPLY_MQTT_CLOSE, MQTT_CLOSE_TIMEOUT_MS);
		if (responseOK) {
			responseOK = gsm.parseReply(REPLY_MQTT_CLOSE, &responseCode, ',', 1);
		} else {
			responseCode = -1;
		}
	} else {
		// Response not ok
		responseCode = -1;
	}
	
	if (responseCode == 0) {
		responseOK = true;
	} else {
		responseOK = false;
	}
	
	return responseOK;
}

uint8_t MQTT::getState() {
	bool responseOK = false;
	uint16_t mqttState = 0;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s?", CMD_MQTT_CONNECT);
	responseOK = gsm.sendParseReply(cmd, REPLY_MQTT_CONNECT, &mqttState, ',', 1);
	if (responseOK) {
		
	} else {
		mqttState = 0;
	}
	
	return (uint8_t)mqttState;
}

int8_t MQTT::connectClient(char *clientID, char *username, char *password) {
	bool responseOK = false;
	uint8_t responseLen = 0;
	int16_t responseCode = -1;
	char cmd[MAX_REPLY_LEN];
	
	if (strlen(username) + strlen(password) < MQTT_MAX_BUF - 30) {
		snprintf(cmd, sizeof(cmd), "%s=%d,\"%s\",\"%s\",\"%s\"", CMD_MQTT_CONNECT, _tcpConnectID, clientID, username, password);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
		
		if (responseOK) {
			responseLen = gsm.waitReply(REPLY_MQTT_CONNECT, _pktTimeout*1000);
			if (responseLen > 0) {
				responseOK = gsm.parseReply(REPLY_MQTT_CONNECT, &responseCode, ',', 2);
			} else {
				responseCode = -1;
			}
		} else {
			// Response not ok
			responseCode = -1;
		}
		
	} else {
		responseCode = -1;
		DEBUG_PRINTLN(F("\t<--- Username or Password too long"));
	}
	return (int8_t)responseCode;
}

bool MQTT::disconnectClient() {
	bool responseOK = false;
	uint8_t responseLen = 0;
	int16_t result = -1;
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "%s=%d", CMD_MQTT_DISCONNECT, _tcpConnectID);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	if (responseOK) {
		responseLen = gsm.waitReply(REPLY_MQTT_DISCONNECT, _pktTimeout*1000);
		if (responseLen > 0) {
			responseOK = gsm.parseReply(REPLY_MQTT_DISCONNECT, &result, ',', 1);
			if (result == 0) {
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
	
	return responseOK;
}

bool MQTT::connected() {
	bool responseOK = false;
	uint8_t responseLen = 0;
	int16_t responseCode = -1;
	char cmd[30];
	
	gsm.flushInput(); // Make sure input is clear
	
	snprintf(cmd, sizeof(cmd), "%s?", CMD_MQTT_CONNECT);
	responseOK = gsm.sendCheckReply(cmd, REPLY_MQTT_CONNECT, _pktTimeout*1000);
	
	if (responseOK) {
		responseOK = gsm.parseReply(REPLY_MQTT_CONNECT, &responseCode, ',', 1);
		if (responseOK) {
			responseOK = gsm.waitReply(REPLY_OK);
		} else {
			responseCode = -1;
		}
	} else {
		// Response not ok
		responseCode = -1;
	}
	
	if (responseCode == MQTT_CONNECTED) {
		responseOK = true;
	} else {
		responseOK = false;
	}
	return responseOK;
}

bool MQTT::subscribe(char *topic, uint16_t msgID, uint8_t qos) {
	bool responseOK = false;
	uint8_t responseLen = 0;
	uint16_t result = 255;
	char cmd[MQTT_MAX_BUF];
	
	if (strlen(topic) < MQTT_MAX_BUF - 30) {
		snprintf(cmd, sizeof(cmd), "%s=%d,%u,\"%s\",%d", CMD_MQTT_SUBSCRIBE, _tcpConnectID, msgID, topic, qos);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
		
		if (responseOK) {
			responseLen = gsm.waitReply(REPLY_MQTT_SUBSCRIBE, _pktTimeout*1000);
			if (responseLen > 0) {
				responseOK = gsm.parseReply(REPLY_MQTT_SUBSCRIBE, &result, ',', 2);
				if ((result == 0) || (result == 1)) {
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
	} else {
		responseOK = false;
		DEBUG_PRINTLN(F("\t<--- Topic too long"));
	}
	return responseOK;
}

bool MQTT::unsubscribe(char *topic, uint16_t msgID) {
	bool responseOK = false;
	uint8_t responseLen = 0;
	uint16_t result = 255;
	char cmd[MQTT_MAX_BUF];
	
	if (strlen(topic) < MQTT_MAX_BUF - 30) {
		snprintf(cmd, sizeof(cmd), "%s=%d,%u,\"%s\"", CMD_MQTT_UNSUBSCRIBE, _tcpConnectID, msgID, topic);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
		
		if (responseOK) {
			responseLen = gsm.waitReply(REPLY_MQTT_UNSUBSCRIBE, _pktTimeout*1000);
			if (responseLen > 0) {
				responseOK = gsm.parseReply(REPLY_MQTT_UNSUBSCRIBE, &result, ',', 2);
				if ((result == 0) || (result == 1)) {
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
	} else {
		responseOK = false;
		DEBUG_PRINTLN(F("\t<--- Topic too long"));
	}
	
	return responseOK;
}

bool MQTT::publish(char *topic, char *msg, uint16_t msgID, uint8_t qos, uint8_t retain) {
	bool responseOK = false;
	uint8_t responseLen = 0;
	uint16_t result = 0;
	char cmd[MQTT_MAX_BUF];
	
	if (strlen(topic) < MQTT_MAX_BUF - 30) {
		snprintf(cmd, sizeof(cmd), "%s=%d,%u,%d,%d,\"%s\"", CMD_MQTT_PUBLISH, _tcpConnectID, msgID, qos, retain, topic);
		
		responseOK = gsm.sendCheckReply(cmd, ">");
		
		if (responseOK) {
			gsm.send(msg); // Send message
			snprintf(cmd, sizeof(cmd), "%c", CTRL_Z); // End of message
			responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
			if (responseOK) {
				responseLen = gsm.waitReply(REPLY_MQTT_PUBLISH, _pktTimeout*1000);
				if (responseLen > 0) {
					responseOK = gsm.parseReply(REPLY_MQTT_PUBLISH, &result, ',', 1);
					if ((result == 0) || (result == 1)) {
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
		} else {
			responseOK = false;
		}
	} else {
		// Topic or message too long
		responseOK = false;
		DEBUG_PRINTLN(F("\t<--- Topic or message too long"));
	}
		
	
	return responseOK;
}

bool MQTT::readRecvPacket(char *topic, uint16_t topicLen, char *msg, uint16_t msgLen) {
	bool responseOK = false;
	responseOK = gsm.parseReplyQuoted(REPLY_MQTT_RECV, topic, topicLen, ',', 2);
	
	// UC20 has extra payload_len which BG96 does not have
	responseOK = gsm.parseReplyQuoted(REPLY_MQTT_RECV, msg, msgLen, ',', 4);
	if (responseOK == false) {
		// Try one index before, might be BG96
		responseOK = gsm.parseReplyQuoted(REPLY_MQTT_RECV, msg, msgLen, ',', 3);
	} else {
	
	}
	
	return responseOK;
}

const __FlashStringHelper* MQTT::getOpenResultString(int8_t result) {
  switch (result) {
		case -1: return F("Failed to open network");
		case  0: return F("Network opened successfully");
		case  1: return F("Wrong parameter");
		case  2: return F("MQTT identifier is occupied");
		case  3: return F("Failed to activate PDP");
		case  4: return F("Failed to parse domain name");
		case  5: return F("Network disconnection error");
		default: return F("Reserved for future use");
  }
}

const __FlashStringHelper* MQTT::getConnectResultString(int8_t result) {
  switch (result) {
		case  0: return F("Connection Accepted");
		case  1: return F("Connection Refused: Unacceptable Protocol Version");
		case  2: return F("Connection Refused: Identifier Rejected");
		case  3: return F("Connection Refused: Server Unavailable");
		case  4: return F("Connection Refused: Bad User Name or Password");
		case  5: return F("Connection Refused: Not Authorized");
		default: return F("Unknown Error");
  }
}

const __FlashStringHelper* MQTT::getURCString(char *urcResponse) {
	uint16_t urcCode = 0;
	char tmpValue[4];
	char *chResponse = 0;
	chResponse = strstr(urcResponse, REPLY_MQTT_STAT);	
	
	if (chResponse != NULL) {
		strncpy(tmpValue, urcResponse + strlen(REPLY_MQTT_STAT), strlen(urcResponse) - strlen(REPLY_MQTT_STAT));
		urcCode = atoi(tmpValue);
	} else {
		urcCode = 550; // Unknown error code
	}
	
  switch (urcCode) {
		case 1: return F("Connection is closed or reset by peer");
		case 2: return F("Sending PINGREQ packet timed out or failed");
		case 3: return F("Sending CONNECT packet timed out or failed");
		case 4: return F("Receiving CONNECK packet timed out or failed");
		case 5: return F("Client sent DISCONNECT packet, normal process");
		case 6: return F("Client initiated to close MQTT due to packet sending failure");
		case 7: return F("The link is not alive or the server is unavailable");
		default: return F("Reserved for future use");
  }
}