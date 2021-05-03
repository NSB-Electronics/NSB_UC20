#include "sms.h"

SMS::SMS(){}


bool SMS::defaultSettings() {	
	// Set SMS message format as text mode
	if (!setFormat(SMS_MODE_TXT)) return false;
	
	// Set char set as GSM
  if (!setCharSet(SMS_CHARSET_GSM)) return false;
	
	// Configure URC output to UART1
  if (!configURC(SMS_URC_UART1)) return false;
	
	// Read, write, receive SMS in SIM storage only
	if (!setMsgStorage(SMS_STORE_SM, SMS_STORE_SM, SMS_STORE_SM)) return false;
	
	// Do not show header values
	if (!showTextModeParam(false)) return false;
	
	return true;
}

bool SMS::setFormat(uint8_t mode) {
	char send[20];
	sprintf(send, "%s=%d", CMD_SMS_FORMAT, mode);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return false;
	return true;
}

bool SMS::setCharSet(const char *chset) {
	char send[20];
	sprintf(send, "%s=\"%s\"", CMD_SMS_CHAR_SET, chset);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return false;
	return true;
}

bool SMS::configURC(const char *urcPortValue) {
	char send[30];
	sprintf(send, "%s=\"URCPORT\",\"%s\"", CMD_SMS_URC_CFG, urcPortValue);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return false;
	return true;
}

bool SMS::setMsgStorage(const char *mem1, const char *mem2, const char *mem3) {
	char send[30];
	bool cmdOK = false;
	sprintf(send, "%s=\"%s\",\"%s\",\"%s\"", CMD_SMS_STORAGE, mem1, mem2, mem3);
	cmdOK = gsm.sendCheckReply(send, REPLY_SMS_STORAGE);
	gsm.flushInput(); // Flush OK
	return cmdOK;
}

bool SMS::showTextModeParam(bool show) {
	char send[30];
	bool cmdOK = false;
	uint8_t showParam = 0;
	if (show == false) {
		showParam = 0;
	} else {
		showParam = 1;
	}
	sprintf(send, "%s=%d", CMD_SMS_PARAM, showParam);
	cmdOK = gsm.sendCheckReply(send, REPLY_OK);
	return cmdOK;
}

uint16_t SMS::getStorageUsed(uint8_t mem) {
	char send[20];
		
	sprintf(send, "%s?", CMD_SMS_STORAGE);
	
	gsm.flushInput();
	gsm.println(send);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
	gsm.readLine(1000); // timeout
	gsm.flushInput(); // Flush OK
	
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(gsm.replyBuffer);
	
	// Extract used
	uint16_t smsUsed = 0;
	uint8_t pos = 0;
	switch (mem) {
		case 1:
		  pos = 1;
			break;
		case 2:
		  pos = 4;
			break;
	  case 3:
		  pos = 7;
			break;
		default:
		  return 255;
	}
	if (!gsm.parseReply(REPLY_SMS_STORAGE, &smsUsed, ',', pos)) {
		return 255;
  }
	
	return smsUsed;
}

uint16_t SMS::getStorageTotal(uint8_t mem) {
	char send[20];
		
	sprintf(send, "%s?", CMD_SMS_STORAGE);
	
	gsm.flushInput();
	gsm.println(send);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
	gsm.readLine(1000); // timeout
	gsm.flushInput(); // Flush OK
	
	DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(gsm.replyBuffer);
	
	// Extract total
	uint16_t smsTotal = 0;
	uint8_t pos = 0;
	switch (mem) {
		case 1:
		  pos = 2;
			break;
		case 2:
		  pos = 5;
			break;
	  case 3:
		  pos = 8;
			break;
		default:
		  return 0;
	}
	if (!gsm.parseReply(REPLY_SMS_STORAGE, &smsTotal, ',', pos)) {
		return 0;
  }
	
	return smsTotal;
}

unsigned char SMS::indexNewSMS() {
	return (gsm.indexNewSMS);
}

bool SMS::sendSMS(char *smsAddr, char *smsMsg) {
	char send[30];
	uint16_t smsLen = 0;
	
	// Text mode
	sprintf(send, "%s=1", CMD_SMS_FORMAT);
	if (!gsm.sendCheckReply(send, REPLY_OK)) return false;
	
	sprintf(send, "%s=\"", CMD_SEND_SMS);
  strncpy(send+9, smsAddr, 30-9-2);  // 9 bytes beginning, 2 bytes for close quote + null
  send[strlen(send)] = '\"';
	
  if (!gsm.sendCheckReply(send, CMD_CLOSE_BRACKET)) return false;

  gsm.println(smsMsg);
  gsm.println("");
  gsm.write(0x1A);
	
  gsm.readLine(10000); // read the +CMGS reply, wait up to 10 seconds!!!
  DEBUG_PRINT("\t---> "); DEBUG_PRINTLN(gsm.replyBuffer);
  if (strstr(gsm.replyBuffer, REPLY_SEND_SMS) == 0) {
    return false;
  }
  gsm.readLine(1000); // read OK
  //DEBUG_PRINT("* "); DEBUG_PRINTLN(gsm.replyBuffer);

  if (strcmp(gsm.replyBuffer, REPLY_OK) != 0) {
    return false;
  }

  return true;
}

uint16_t SMS::readSMS(uint8_t index, char *smsBuff, uint16_t maxLen) {
	char send[20];
	uint16_t smsLen = 0;
	
	// Text mode
	sprintf(send, "%s=1", CMD_SMS_FORMAT);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;

  // Show all text mode parameters
	sprintf(send, "%s=1", CMD_SMS_PARAM);
	
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;
	
	sprintf(send, "%s=%d", CMD_READ_SMS, index);
	gsm.flushInput();
	gsm.println(send);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
	gsm.readLine(1000);
	
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(gsm.replyBuffer);
	
	// Extract message length
	if (!gsm.parseReply(REPLY_READ_SMS, &smsLen, ',', 11)) {
		return 0;
  }
	
	gsm.readRaw(smsLen);
	
  gsm.flushInput();
	
  uint16_t finalLen = min(maxLen, (uint16_t)strlen(gsm.replyBuffer));
  strncpy(smsBuff, gsm.replyBuffer, finalLen);
  smsBuff[finalLen] = 0; // end the string
	
  //DEBUG_PRINTLN(smsBuff);
	
	return finalLen;
}

bool SMS::getSMSSender(uint8_t index, char *sender, int senderLen) {
	char send[20];
	
	// Text mode
	sprintf(send, "%s=1", CMD_SMS_FORMAT);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;

  // Show all text mode parameters
	sprintf(send, "%s=1", CMD_SMS_PARAM);
	
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;
	
	sprintf(send, "%s=%d", CMD_READ_SMS, index);
	gsm.flushInput();
	gsm.println(send);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
	gsm.readLine(1000);
	
	//DEBUG_PRINTLN(gsm.replyBuffer);
	
	// Extract sms number from buffer
	boolean result = gsm.parseReplyQuoted(REPLY_READ_SMS, sender, senderLen, ',', 1);
	
  gsm.flushInput();
		
	return result;
}

bool SMS::getSMSDate(uint8_t index, char *date, int dateLen) {
	char send[20];
	
	// Text mode
	sprintf(send, "%s=1", CMD_SMS_FORMAT);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;

  // Show all text mode parameters
	sprintf(send, "%s=1", CMD_SMS_PARAM);
	
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;
	
	sprintf(send, "%s=%d", CMD_READ_SMS, index);
	gsm.flushInput();
	gsm.println(send);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
	gsm.readLine(1000);
	
	//DEBUG_PRINTLN(gsm.replyBuffer);
	
	// Extract date from buffer
	boolean result = gsm.parseReplyQuoted(REPLY_READ_SMS, date, dateLen, ',', 3);
	
  gsm.flushInput();
		
	return result;
}

bool SMS::getSMSTime(uint8_t index, char *time, int timeLen) {
	char send[20];
	
	// Text mode
	sprintf(send, "%s=1", CMD_SMS_FORMAT);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;

  // Show all text mode parameters
	sprintf(send, "%s=1", CMD_SMS_PARAM);
	
  if (!gsm.sendCheckReply(send, REPLY_OK)) return 0;
	
	sprintf(send, "%s=%d", CMD_READ_SMS, index);
	gsm.flushInput();
	gsm.println(send);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
	gsm.readLine(1000);
	
	//DEBUG_PRINTLN(gsm.replyBuffer);
	
	// Extract time from buffer
	boolean result = gsm.parseReplyQuoted(REPLY_READ_SMS, time, timeLen, ',', 4);
	
  gsm.flushInput();
		
	return result;
}

bool SMS::sendUSSD(char *ussdMsg, char *ussdBuff, uint16_t maxLen) {
	char send[30];
	sprintf(send, "%s=1", CMD_SEND_USSD);
	
  if (!gsm.sendCheckReply(send, REPLY_OK)) return false;
	
	sprintf(send, "%s=1,\"", CMD_SEND_USSD);
  strncpy(send+11, ussdMsg, 30-11-2);  // 11 bytes beginning, 2 bytes for close quote + null
  send[strlen(send)] = '\"';

  if (!gsm.sendCheckReply(send, REPLY_OK)) {
    return false;
  } else {
		gsm.readLine(10000, true, 3); // Read the +CUSD reply, might be multiline, wait up to 10 seconds!!!
		//DEBUG_PRINT("---> ");
		//DEBUG_PRINTLN(gsm.replyBuffer);
		gsm.flushInput();
		if (!gsm.parseReplyQuoted(REPLY_SMS_USSD, ussdBuff, maxLen, ',', 1)) {
			return false;
		}
  }
  return true;
}

bool SMS::deleteSMS(uint8_t index) {
	char send[20];
	sprintf(send, "%s=%d", CMD_SIM_DEL_MSG, index);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return false;
	return true;
}

bool SMS::deleteAllSMS() {
	char send[20];
	sprintf(send, "%s=1,4", CMD_SIM_DEL_MSG);
  if (!gsm.sendCheckReply(send, REPLY_OK)) return false;
	return true;
}

int8_t SMS::getNumSMS(void) {
  char send[20];
	char storage[10];
	uint16_t numSMS;

  // Get into text mode
	sprintf(send, "%s=%d", CMD_SMS_FORMAT, SMS_MODE_TXT);
	if (!gsm.sendCheckReply(send, REPLY_OK)) return -1;

  // Ask how many sms are stored
	sprintf(send, "%s?", CMD_SMS_STORAGE);
	
	sprintf(storage, "\"%s\",", SMS_STORE_SM);
  if (gsm.sendParseReply(send, storage, &numSMS))
    return numSMS;
  sprintf(storage, "\"%s\",", SMS_STORE_ME);
	if (gsm.sendParseReply(send, storage, &numSMS))
    return numSMS;
  sprintf(storage, "\"%s\",", SMS_STORE_MT);
	if (gsm.sendParseReply(send, storage, &numSMS))
    return numSMS;
  return -1;
}

int8_t SMS::getNextSMSidx(void) {
  char send[20];
	int8_t idxReturn = 0;
	uint16_t idx = 0;
	bool responseOK = false;

  // Get into text mode
	sprintf(send, "%s=%d", CMD_SMS_FORMAT, SMS_MODE_TXT);
	if (!gsm.sendCheckReply(send, REPLY_OK)) return -1;
	
	sprintf(send, "%s=\"ALL\"", CMD_SMS_LIST);
	
  responseOK = gsm.sendParseReply(send, REPLY_SMS_LIST, &idx, ',', 0);
  
	if (!responseOK) {
		idxReturn = -1;
	} else {
		idxReturn = idx;
	}
  return idxReturn;
}
