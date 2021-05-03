#include "internet.h"

INTERNET::INTERNET() {
	
}

bool INTERNET::configure(uint8_t contextid, uint8_t context_type, char *apn, char *user, char *password, uint8_t auth) {
	bool responseOK = false;
	char cmd[100];
	
	sprintf(cmd, "%s=%d,%d,\"%s\",\"%s\",\"%s\",%d", CMD_TCP_CONF_CONTEXT, contextid, context_type, apn, user, password, auth);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	return responseOK;
}

bool INTERNET::configure(uint8_t contextid, char *apn, char *user, char *password) {
	return configure(contextid, 1, apn, user, password, 1);
}

bool INTERNET::configure(char *apn, char *user, char *password) {
	return configure(1, 1, apn, user, password, 1);
}

bool INTERNET::configDNS(uint8_t contextid, char *pridnsaddr, char *secdnsaddr) {	
	bool responseOK = false;
	char cmd[100];
	
	sprintf(cmd, "%s=%d,\"%s\",\"%s\"", CMD_TCP_CONF_DNS, contextid, pridnsaddr, secdnsaddr);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	return responseOK;
}

bool INTERNET::configDNS(char *pridnsaddr, char *secdnsaddr) {
	return configDNS(1, pridnsaddr, secdnsaddr);
}

bool INTERNET::configDNS(uint8_t contextid) {
	return configDNS(contextid, "8.8.8.8", "9.9.9.9");
}

bool INTERNET::configDNS() {
	return configDNS(1, "8.8.8.8", "9.9.9.9");
}

bool INTERNET::connect(uint8_t contextid) {
	bool responseOK = false;
	char cmd[20];
	
	sprintf(cmd, "%s=%d", CMD_TCP_ACT_PDP, contextid);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK, 150000);
	return responseOK;
}

bool INTERNET::connect() {
	return connect(1);
}

bool INTERNET::disconnect(uint8_t contextid) {
	bool responseOK = false;
	char cmd[20];
	
	sprintf(cmd, "%s=%d", CMD_TCP_DEACT_PDP, contextid);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK, 40000);
	return responseOK;
}

bool INTERNET::disconnect() {
	return disconnect(1);
}

bool INTERNET::getIP(char *ip, uint8_t ipLen) {
	bool responseOK = false;
	char cmd[20];
	uint8_t responseLenRecv = 0;
	
	sprintf(cmd, "%s?", CMD_TCP_ACT_PDP);
	responseLenRecv = gsm.getReply(cmd);
	if (responseLenRecv > 0) {
		responseOK = gsm.parseReplyQuoted(REPLY_TCP_ACT_PDP, ip, ipLen, ',', 3);
	}
	responseOK = gsm.waitReply(REPLY_OK); // Eat OK
	
	return responseOK;
}
