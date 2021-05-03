#include "smtp.h"

SMTP::SMTP(){}


bool SMTP::setSSLtype(uint8_t sslType) {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=\"ssltype\",%d", CMD_SMTP_CONF, sslType);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool SMTP::setContextID(uint8_t contextid) {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=\"contextid\",%d", CMD_SMTP_CONF, contextid);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}


bool SMTP::setSSLctxID(uint8_t sslctxid) {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=\"sslctxid\",%d", CMD_SMTP_CONF, sslctxid);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}


bool SMTP::configServer(char *srvaddr, uint16_t srvport) {
	bool responseOK = false;
	char cmd[80];
	
	if (strlen(srvaddr) <= 50) {
		sprintf(cmd, "%s=\"smtpserver\",\"%s\",%d", CMD_SMTP_CONF, srvaddr, srvport);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		// Server address too long
	}
	
	return responseOK;
}


bool SMTP::setAccount(char *username, char *password) {
	bool responseOK = false;
	char cmd[130];
	
	if ((strlen(username) <= 50) && (strlen(password) <= 50)) {
		sprintf(cmd, "%s=\"account\",\"%s\",\"%s\"", CMD_SMTP_CONF, username, password);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		// Username or password too long
	}
	
	return responseOK;
}


bool SMTP::setSender(char *senderName, char *senderEmail) {
	bool responseOK = false;
	char cmd[130];
	
	if ((strlen(senderName) <= 50) && (strlen(senderEmail) <= 50)) {
		sprintf(cmd, "%s=\"sender\",\"%s\",\"%s\"", CMD_SMTP_CONF, senderName, senderEmail);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		// Sender or email address too long
	}
	
	return responseOK;
}


bool SMTP::addRecipient(char *emailaddr, uint8_t type) {
	bool responseOK = false;
	char cmd[80];
	
	if (strlen(emailaddr) <= 50) {
		sprintf(cmd, "%s=%d,%d,\"%s\"", CMD_SMTP_RECIPIENT, SMTP_ADD, type, emailaddr);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		// Email address too long
	}
	
	return responseOK;
}

bool SMTP::deleteRecipients() {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=%d", CMD_SMTP_RECIPIENT, SMTP_DELETE);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool SMTP::setSubject(char *subject, uint8_t charset) {
	bool responseOK = false;
	char cmd[130];
	
	if (strlen(subject) <= 100) {
		sprintf(cmd, "%s=%d,\"%s\"", CMD_SMTP_SUBJECT, charset, subject);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		// Subject too long
	}
	
	return responseOK;
}

bool SMTP::setBody(char *body, uint8_t charset, uint16_t inputTime) {
	bool responseOK = false;
	char cmd[30];
	
	if (strlen(body) <= 10000) {
		sprintf(cmd, "%s=%d,%d,%d", CMD_SMTP_BODY, charset, strlen(body), inputTime);
		responseOK = gsm.sendCheckReply(cmd, AVT1_CONNECT);
		if (responseOK) {
			responseOK = gsm.sendCheckReply(body, REPLY_SMTP_BODY);
			if (responseOK) {
				responseOK = gsm.waitReply(REPLY_OK);
			} else {
				// Command failed
			}	
		} else {
		// Command failed
		}		
	} else {
		// Body too long
	}
	
	return responseOK;
}


bool SMTP::addAttachment(char *filename, uint8_t fileIndex) {
	bool responseOK = false;
	char cmd[80];
	
	if (strlen(filename) <= 50) {
		sprintf(cmd, "%s=%d,%d,\"%s\"", CMD_SMTP_ATTACHMENT, SMTP_ADD, fileIndex, filename);
		responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	} else {
		// Filename too long
	}
	
	return responseOK;
}


bool SMTP::clearContent() {
	bool responseOK = false;
	char cmd[20];
	
	sprintf(cmd, "%s", CMD_SMTP_CLEAR);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool SMTP::sendEmail(uint16_t timeout) {
	bool responseOK = false;
	char cmd[20];
	
	sprintf(cmd, "%s=%d", CMD_SMTP_SEND, timeout);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}