// Based on TEE_UG20 by ThaiEasyElec
// https://github.com/ThaiEasyElec/TEE_UC20_Shield


#include "NSB_UC20.h"

int PWRKEY_PIN = 12;
int RST_PIN	   = 12;

unsigned long previousMillis_timeout = 0; 

UC20::UC20() {
	//_watchdogCallback = (void);
}

void UC20::setWatchdog(void (*watchdogCallback)(void)) {
	_watchdogCallback = watchdogCallback;
}


void UC20::begin(HardwareSerial *serial, long baud) {
	serial->begin(baud);
	_Serial = serial;
}

#if SOFTSERIAL 
void UC20:: begin(SoftwareSerial *serial, long baud) {
	serial->begin(baud);
	_Serial = serial;	
}
#endif

#if ATLSOFTSERIAL
void UC20::begin(AltSoftSerial *serial, long baud) {
	serial->begin(baud);
	_Serial = serial;
}
#endif

void UC20::setPowerKeyPin(int pin) {
	PWRKEY_PIN = pin;
}

bool UC20::powerOn(int pwrKeyPin) {
	bool timedOut = false;
	bool replyOK = false;
	bool powerOn = false;
	char *chReply = 0;
	uint16_t replyLen = 0;
	unsigned long startTime = millis();
	
	pinMode(pwrKeyPin, OUTPUT);
	
	digitalWrite(pwrKeyPin, HIGH);
	delay(500);
	digitalWrite(pwrKeyPin, LOW);
	delay(500);
	
	startTime = millis();
	while ((!powerOn) && (!timedOut)) {
		replyLen = readLine(1000);
		
		if (replyLen > 0) {
			chReply = strstr(replyBuffer, REPLY_READY);
			if (chReply != NULL) {
				DEBUG_PRINT(F("Power ON"));
				powerOn = true;
			} else {
				chReply = strstr(replyBuffer, REPLY_POWER_DOWN);
				if (chReply != NULL) {
					DEBUG_PRINT(F("Power OFF"));
					pinMode(pwrKeyPin, OUTPUT);
					digitalWrite(pwrKeyPin, HIGH);
					delay(500);
					digitalWrite(pwrKeyPin, LOW);
					delay(500);
				} else {
				}
			}
		}
		
		if (millis() - startTime > 10000) {
			timedOut = true;
			DEBUG_PRINT(F("powerOn timeout"));
		}
	}
	return powerOn;
}

bool UC20::powerOn() {
	return powerOn(PWRKEY_PIN);
}

bool UC20::powerOff(boolean usePwrKey, int pwrKeyPin) {
	bool timedOut = false;
	bool replyOK = false;
	bool powerOff = false;
	char *chReply = 0;
	uint16_t replyLen = 0;
	unsigned long startTime = millis();
	
	if (usePwrKey) {
		pinMode(pwrKeyPin, OUTPUT);
		digitalWrite(pwrKeyPin, HIGH);
		(*_watchdogCallback)();
		delay(500);
		digitalWrite(pwrKeyPin, LOW);
		delay(500);
		(*_watchdogCallback)();
	} else {
		flushInput();
		_Serial->println(CMD_POWER_OFF);
	}
	
	startTime = millis();
	while ((!powerOff) && (!timedOut)) {
		replyLen = readLine(1000);
		
		if (replyLen > 0) {
			chReply = strstr(replyBuffer, REPLY_READY);
			if (chReply != NULL) {
				DEBUG_PRINT(F("Power ON"));
				if (usePwrKey) {
					pinMode(pwrKeyPin, OUTPUT);
					digitalWrite(pwrKeyPin, HIGH);
					(*_watchdogCallback)();
					delay(500);
					digitalWrite(pwrKeyPin, LOW);
					delay(500);
					(*_watchdogCallback)();
				} else {
					flushInput();
					_Serial->println(CMD_POWER_OFF);
				}
			} else {
				chReply = strstr(replyBuffer, REPLY_POWER_DOWN);
				if (chReply != NULL) {
					DEBUG_PRINT(F("Power OFF"));
					powerOff = true;
				} else {
				}
			}
		}
		
		if (millis() - startTime > 60000) {
			timedOut = true;
			DEBUG_PRINT(F("powerOff timeout"));
		}
	}
	return powerOff;
}

bool UC20::powerOff(boolean usePwrKey) {
	return powerOff(usePwrKey, PWRKEY_PIN);
}

bool UC20::powerOff() {
	return powerOff(true, PWRKEY_PIN);
}

bool UC20::hardwareReset(int rstPin) {
	pinMode(rstPin, OUTPUT);
	(*_watchdogCallback)();
	digitalWrite(rstPin, HIGH);
	delay(250);
	digitalWrite(rstPin, LOW);
	(*_watchdogCallback)();
	
	return waitReady();
}

bool UC20::setAutoReset(uint8_t mode, uint32_t delay) {
	char send[20];
	
	snprintf(send, sizeof(send), "%s=%d,%Lu", CMD_RESET, mode, delay);
	flushInput();
	_Serial->println(send);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
	return waitReply(REPLY_OK, 3000);
}

uint32_t UC20::getAutoReset() {
	char send[20];
	uint32_t remainDelay = 0;
	snprintf(send, sizeof(send), "%s?", CMD_RESET);
	
	sendParseReply(send, REPLY_RESET, &remainDelay, ',', 2);
	
	return remainDelay;
}

bool UC20::waitSIMReady() {
	bool timedOut = false;
	bool simReady = false;
	char simStatus[20];
	char *chReply = 0;
	unsigned long startTime = millis();
	
	while ((simReady == false) && (!timedOut)) {
		(*_watchdogCallback)();
		bool statusOK = getSIMStatus(simStatus, sizeof(simStatus));
		if (statusOK == true) {
			chReply = strstr(simStatus, REPLY_SIM_READY);
			if (chReply != NULL) {
				// Found READY reply
				simReady = true;
			} else {
				DEBUG_PRINT(F("\t---- "));
				DEBUG_PRINT(simStatus);
				DEBUG_PRINT(F(" - "));
				DEBUG_PRINTLN(getCMEerrorString(simStatus));
				delay(1000); // Wait
			}
		} else {
			
		}
		if (millis() - startTime > TIMEOUT_SIM_MS) {
			timedOut = true;
			DEBUG_PRINTLN(F("waitSIMReady Timeout"));
		}
	}
	return simReady;
}

bool UC20::getSIMStatus(char *response, uint8_t responseLen) {
	
	bool responseOK = false;
	uint8_t responseLenRecv = 0;
	char *chReply = 0;
	
	responseLenRecv = sendGetResponse(CMD_SIM_STATUS, response, responseLen);
	
	chReply = strstr(response, REPLY_CPIN);
	if (chReply != NULL) {
		responseLenRecv -= 7; // Remove +CPIN:
		if (responseLenRecv > responseLen) {
			responseLenRecv = responseLen;
		} else {
			// Keep length
		}
		strncpy(response, replyBuffer+7, responseLenRecv); // Remove +CPIN: 
		response[responseLenRecv] = 0;
		responseOK = true;
	} else {
		if (responseLenRecv > responseLen) {
			responseLenRecv = responseLen;
		} else {
			// Keep length
		}
		strncpy(response, replyBuffer, responseLenRecv);
		response[responseLenRecv] = 0;
		responseOK = true;
	}
	
  waitOK();

  return responseOK;
}

bool UC20::getIMEI(char *response, uint8_t responseLen) {
	bool responseOK = false;
	uint8_t responseLenRecv = 0;
	
	responseLenRecv = getReply(CMD_IMEI);
	if (responseLenRecv > 0) {
		responseOK = parseReplyQuoted("", response, responseLen, ',', 0);
	}
	
	waitOK();
	
  return responseOK;
}

bool UC20::getIMSI(char *response, uint8_t responseLen) {
	bool responseOK = false;
	uint8_t responseLenRecv = 0;
	
	responseLenRecv = getReply(CMD_CIMI);
	if (responseLenRecv > 0) {
		responseOK = parseReplyQuoted("", response, responseLen, ',', 0);
	}
	
	waitOK();
	
  return responseOK;
}

bool UC20::getICCID(char *response, uint8_t responseLen) {
	bool responseOK = false;
	uint8_t responseLenRecv = 0;
	
	//responseLenRecv = sendGetResponse(CMD_QCCID, response, responseLen);
	responseLenRecv = getReply(CMD_QCCID);
	if (responseLenRecv > 0) {
		responseOK = parseReplyQuoted(REPLY_QCCID, response, responseLen, ',', 0);
	}
	
	waitOK();

  return responseOK;
}

bool UC20::waitOK(uint32_t delay) {
	return waitReply(REPLY_OK, delay);
}
	
bool UC20::waitReady() {		
	bool timedOut = false;
	bool replyOK = false;
	char *chReply = 0;
	uint16_t replyLen = 0;
	unsigned long startTime = millis();
	
	while ((replyOK == false) && (!timedOut)) {
		replyLen = readLine(1000);
		DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replyBuffer);
		
		if (replyLen > 0) {
			chReply = strstr(replyBuffer, REPLY_READY);
			if (chReply != NULL) {
				// Found RDY reply
				replyOK = true;
			} else {
				replyOK = false;
				chReply = strstr(replyBuffer, REPLY_POWER_DOWN);
				if (chReply != NULL) {
					powerOn();
				} else {
					
				}
			}
		} else {
			replyOK = false;
		}
		
		if (millis() - startTime > 30000) {
			timedOut = true;
			replyOK = false;
			DEBUG_PRINTLN(F("\t---- waitReady - No response"));
		}
	}
	return replyOK;
}

bool UC20::setEchoMode(bool echo) {
	bool response = false;
	char cmd[10];
	
	if (echo == true) {
		snprintf(cmd, sizeof(cmd), "%s", CMD_ECHO_ON);
	} else {
		snprintf(cmd, sizeof(cmd), "%s", CMD_ECHO_OFF);
	}
	response = sendCheckReply(cmd, REPLY_ATE0);
	return response;
}

bool UC20::moduleInfo(char *response, uint8_t responseLen) {
	uint8_t responseLenRecv = 0;
	bool responseOK = false;
	flushInput();
	
	_Serial->println(CMD_MODULE_INFO);
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(CMD_MODULE_INFO);
	
	responseLenRecv = waitReply(REPLY_REVISION, 1000);
	
	if (responseLenRecv > 10) {
		responseLenRecv -= 10; // Remove Revision: 
		if (responseLenRecv > responseLen) {
			responseLenRecv = responseLen;
		}
		strncpy(response, replyBuffer+10, responseLenRecv); // Remove Revision: 
		response[responseLenRecv] = 0;
		responseOK = true;
	}
  readLine(); // eat 'OK'
	
  return responseOK;
}

bool UC20::waitNetworkRegistered() {
	bool timedOut = false;
	bool foundNetwork = false;
	uint8_t status = 0;
	unsigned long startTime = millis();
	while (!timedOut) {
	  (*_watchdogCallback)();
		status = getNetworkStatus();
		if ((status == NETWORK_HOME) || (status == NETWORK_ROAMING)) {
			foundNetwork = true;
			return true;
		}	else {
			// Wait
			delay(1000);
		}
		if (millis() - startTime > (180000)) {
			timedOut = true;
			foundNetwork = false;
			DEBUG_PRINTLN(F("\tE,waitNetworkRegistered,Timeout,Check SIM/Signal"));
		}
	}
	return foundNetwork;
}

uint8_t UC20::getNetworkStatus() {
	
	bool responseOK = false;
	uint16_t networkStat = 0;
	
	responseOK = sendParseReply(GET_NETWORK_STAT, REPLY_CREG, &networkStat, ',', 1);
	
  if (responseOK == false) {
		networkStat = NETWORK_NONE;
	}

  return networkStat;
}

bool UC20::getOperator(char *response, uint8_t responseLen) {
	bool responseOK = false;
	
	responseOK = sendParseReply(GET_NETWORK_OPER, REPLY_COPS, response, responseLen, ',', 2);

  return responseOK;
}

uint8_t UC20::signalQuality() {
	
	bool responseOK = false;
	uint16_t signalQty = 0;
	
	responseOK = sendParseReply(CMD_SIGNAL_QUALITY, REPLY_CSQ, &signalQty, ',', 0);
	
  if (responseOK == false) {
		signalQty = 0;
	}

  return signalQty;
}

int UC20::signalQualitydBm(uint8_t rssi) {
	
	// 0 -113 dBm or less
	// 1 -111 dBm
	// 2...30 -109... -53 dBm
	// 31 -51 dBm or greater
	// 99 Not known or not detectable
	
	// y = mx +c
	int m = 2;
	int c = -113;
	
	if (rssi == 0) return -113;
	if (rssi == 1) return -111;
	if (rssi == 99) return -255;
	
	if ((rssi >= 2) && (rssi <= 30)) return (rssi*m + c);
	if (rssi >= 31) return -51;
	
	return 0;
	
}

uint8_t UC20::signalQualityPercentage(uint8_t rssi) {
	if (rssi == 0) return 0;
	if (rssi == 99) return 0;
	
	if ((rssi >= 1) && (rssi <= 31)) return (((float(rssi)*100)/31) + 0.5); // Round nearest integer
	if (rssi > 31) return 100;
	
	return 0;
}

bool UC20::resetDefaults() {
	setEchoMode(false);
	return sendCheckReply(CMD_DEFAULTS, REPLY_OK);
}


bool UC20::getNetworkTime(char *response, uint8_t responseLen) {
	bool responseOK = false;
	uint8_t responseLenRecv = 0;
	
	responseLenRecv = getReply(CMD_NETWORK_TIME);
	if (responseLenRecv > 0) {
		if (strncmp(replyBuffer, REPLY_CLTS, 8) != 0) {
		  char *p = replyBuffer+8; // Remove leading string
		  uint16_t lentocopy = min(responseLen-1, (int)strlen(p));
		  strncpy(response, p, lentocopy);
		  response[lentocopy-1] = 0;
		  responseOK = true;
	  } else {
			responseOK = false;
	  }
	}
	
	waitOK();
	
  return responseOK;
}

time_t UC20::networkTimeUTC(char *time) {
	// Excludes timezone, get it using getNetworkTimezone
	static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
	String strTime = time;
	
	int idxSlash1 = strTime.indexOf(F("/"));
	int idxSlash2 = strTime.indexOf(F("/"), idxSlash1+1);
	int idxComma1 = strTime.indexOf(F(","));
	int idxComma2 = strTime.indexOf(F(","), idxComma1+1);
	int idxColon1 = strTime.indexOf(F(":"));
	int idxColon2 = strTime.indexOf(F(":"), idxColon1+1);
	int idxPlus = strTime.indexOf(F("+"));
	
	String strYear = strTime.substring(0, idxSlash1);
	String strMonth = strTime.substring(idxSlash1+1, idxSlash2);
	String strDay = strTime.substring(idxSlash2+1, idxComma1);
	
	String strHour = strTime.substring(idxComma1+1, idxColon1);
	String strMinute = strTime.substring(idxColon1+1, idxColon2);
	String strSecond = strTime.substring(idxColon2+1, idxPlus);
	
	// Not used here, see getNetworkTimezone
	String strTimezone = strTime.substring(idxPlus+1, idxComma2);
	int offset = strTimezone.toInt() * 0.25;
	
	int year = strYear.toInt();
	if (year < 100) {
		year = year + 2000; // String does not include century
	}
	year = year - 1970; // offset from 1970
	int month = strMonth.toInt();
	int day = strDay.toInt();
	int hour = strHour.toInt();
	int minute = strMinute.toInt();
	int second = strSecond.toInt();
	
	uint32_t seconds = 0;
	
	seconds = year*(SECS_PER_DAY * 365);
	for (int i = 0; i < year; i++) {
		if (LEAP_YEAR(i)) {
			seconds += SECS_PER_DAY;   // add extra days for leap years
		}
	}
	  
	// add days for this year, months start from 1
	for (int i = 1; i < month; i++) {
		if ( (i == 2) && LEAP_YEAR(year)) { 
			seconds += SECS_PER_DAY * 29;
		} else {
			seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
		}
	}
	seconds += (day-1) * SECS_PER_DAY; // Not sure why the -1?
	seconds += hour * SECS_PER_HOUR;
	seconds += minute * SECS_PER_MIN;
	seconds += second;		
		
	return (time_t)seconds;
}

int UC20::networkTimezone(char *time) {
	String strTime = time;
	String strTimezone = "";
	
	int idxComma1 = strTime.indexOf(F(","));
	int idxComma2 = strTime.indexOf(F(","), idxComma1+1);
	int idxPlus = strTime.indexOf(F("+"));
	int idxMinus = strTime.indexOf(F("-"));
	
	if (idxPlus != -1) {
		strTimezone = strTime.substring(idxPlus+1, idxComma2);
	} else {
		strTimezone = strTime.substring(idxMinus+1, idxComma2);
	}
	
	int offset = strTimezone.toInt() * 0.25;
	
	return offset;
}


bool UC20::getBattVoltage(uint16_t *v, uint16_t *p) {
	// NOTE: On UC20 MiniPCIe there is a boost converter, thus value not true
	_Serial->println(F("AT+CBC"));
	
	unsigned long startTime = millis();
	bool timedOut = false;
	
	while (!timedOut) {
		String req = gsm.readStringUntil('\n');
	  if (req.indexOf(F("+CBC:")) != -1)	{
			String ret = req.substring(req.indexOf(F(":"))+2);
			char tmpBuf[20];
			uint8_t len = min((int)19, (int)ret.length());
			ret.toCharArray(tmpBuf, len);
			char *bcsp = strtok(tmpBuf, ",");
			if (!bcsp) return false;
			
			char *bclp = strtok(NULL, ",");
			if (!bclp) return false;
			if (*p != NULL) {
				*p = atof(bclp);
			}
			
			char *vp = strtok(NULL, ",");
			if (!vp) return false;
			*v = atof(vp);
			return true;
		}
		if (req.indexOf(F("+CME ERROR:")) != -1) {
			return false;
		}
		if ((millis() - startTime) % 500 == 0) {
			(*_watchdogCallback)();
		}
		if (millis() - startTime > 3000) {
			timedOut = true;
			DEBUG_PRINTLN(F("\tE,getBattVoltage,Timeout"));
		}
	}
	return false;
}

unsigned char UC20::checkURC() {
	if (_Serial->available()) {
//		while (_Serial->available()) {
			readLine(1000);
			DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replyBuffer);
			if (strstr(gsm.replyBuffer, REPLY_OK) != 0) {
				eventType = EVENT_OK;
			} else if (strstr(gsm.replyBuffer, REPLY_ERROR) != 0) {
				eventType = EVENT_ERROR;
			} else if (strstr(gsm.replyBuffer, REPLY_READY) != 0) {
			} else if (strstr(gsm.replyBuffer, AVT1_RING) != 0) {
				eventType = EVENT_RING;
			} else if (strstr(gsm.replyBuffer, AVT1_NO_CARRIER) != 0) {
				DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN("AVT1_NO_CARRIER");
			} else if (strstr(gsm.replyBuffer, REPLY_SMS_RECV) != 0) {
				eventType = EVENT_SMS;
				parseReply(REPLY_SMS_RECV, &indexNewSMS, ',', 1);
				DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINT("SMS Index: "); DEBUG_PRINTLN(indexNewSMS);
			} else if (strstr(gsm.replyBuffer, REPLY_SMS_USSD) != 0) {
				eventType = EVENT_USSD;
			} else if (strstr(gsm.replyBuffer, REPLY_CME_ERROR) != 0) {
				eventType = EVENT_SIM_ERR;
			} else if (strstr(gsm.replyBuffer, REPLY_SMTP_PUT) != 0) {
				parseReply(gsm.replyBuffer, &emailErr, ',', 0);
				eventType = EVENT_EMAIL;
			} else if (strstr(gsm.replyBuffer, REPLY_SSL_URC) != 0) {
				eventType = EVENT_SSLURC;
			} else if (strstr(gsm.replyBuffer, REPLY_SIM_NOT_READY) != 0) {
			  eventType = EVENT_SIM_ERR;
			} else if (strstr(gsm.replyBuffer, REPLY_URC_PDP_DEACT) != 0) {
				eventType = EVENT_URC_DEACT;
			} else if (strstr(gsm.replyBuffer, REPLY_MQTT_RECV) != 0) {
				eventType = EVENT_MQTT_RECV;
			} else if (strstr(gsm.replyBuffer, REPLY_MQTT_STAT) != 0) {
				eventType = EVENT_MQTT_STAT;
			} else {
				DEBUG_PRINT(F("\t<--- Unknown event type: "));
				DEBUG_PRINTLN(replyBuffer);
				eventType = EVENT_NONE;
			}
//		}
	} else {
		eventType = EVENT_NONE;
	}
	
	return (eventType); // EVENT_NONE
}

void UC20::send(const char *send) {
  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
  _Serial->println(send);
	
}
	
bool UC20::sendCheckReply(const char *send, const char *reply, uint32_t timeout) {
  bool replyFound = false;
	uint16_t replyLen = 0;
	
	flushInput();
	
  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
  _Serial->println(send);

  replyLen = readLine(timeout);

  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replyBuffer);
	
	char *chReply;

	chReply = strstr(replyBuffer, reply);

  if (replyLen == 0) {
	  replyFound = false;
	} else if (chReply != NULL) {
		replyFound = true;
	} else {
		replyFound = false;
	}
	
  return replyFound;
}

uint8_t UC20::sendGetResponse(const char *send, char *response, uint8_t responseLen, uint32_t timeout) {
  bool replyFound = false;
	uint16_t responseLenRecv = 0;
	
	flushInput();
	
  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
	
  _Serial->println(send);

  responseLenRecv = readLine(timeout);

  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replyBuffer);
	
	if (responseLenRecv > 0) {
		if (responseLenRecv > responseLen) {
			responseLenRecv = responseLen;
		}
		strncpy(response, replyBuffer, responseLenRecv);
		response[responseLenRecv] = 0;
	}
	
  return responseLenRecv;
}

bool UC20::checkReply(const char *reply, uint32_t timeout) {
  bool replyFound = false;
	uint16_t replyLen = 0;
  replyLen = readLine(timeout);
	
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replyBuffer);
	
	char *chReply = strstr(replyBuffer, reply);
	
  if ((replyLen == 0) || (chReply == NULL)) {
	  replyFound = false;
	} else {
		replyFound = true;
	}
	
  return replyFound;
}

uint8_t UC20::getReply(const char *send, uint32_t timeout) {
	uint8_t replyLen = 0;
  flushInput();
  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);
  _Serial->println(send);

  replyLen = readLine(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replyBuffer);

  return replyLen;
}

uint8_t UC20::waitReply(const char *reply, uint32_t timeout) {
	bool replyFound = false;
	uint16_t replyLen = 0;
	while (timeout -= TIMEOUT_MIN_MS) {
		replyLen = readLine(TIMEOUT_MIN_MS);
		
		if (replyLen > 0) {
			DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replyBuffer);
		}
		
		char *chReply = strstr(replyBuffer, reply);
		
		if ((replyLen == 0) || (chReply == NULL)) {
			replyFound = false;
			replyLen = 0;
		} else {
			replyFound = true;
			break;
		}
		
		if (timeout > TIMEOUT_MAX_MS) {
			replyFound = false;
			replyLen = 0;
			break; // Probably wrapped
		}
	}
	
  return replyLen;
}

uint16_t UC20::readRaw(uint16_t b) {
  uint16_t idx = 0;

  while (b && (idx < sizeof(replyBuffer)-1)) {
		// Reset watchdog
		(*_watchdogCallback)();
		
    if (_Serial->available()) {
      replyBuffer[idx] = _Serial->read();
      idx++;
      b--;
    }
  }
  replyBuffer[idx] = 0;

  return idx;
}

uint8_t UC20::readLine(uint32_t timeout, bool multiLine, uint8_t numLines) {
  uint8_t replyIdx = 0;
	uint8_t newLines = 0;
  while (timeout--) {
		if (timeout % 10 == 0) {
			// Reset watchdog at least every 10ms
			(*_watchdogCallback)();
		}
		
    if (replyIdx >= MAX_REPLY_LEN) {
      DEBUG_PRINTLN(F("E,readLine,Not Enough Space"));
      break;
    }

    while(_Serial->available()) {
      char c =  _Serial->read();
      if (c == '\r') continue;
      if (c == 0x0A) {
				newLines += 1;
        if (replyIdx == 0)   // the first 0x0A is ignored
          continue;

        if (!multiLine) {
          timeout = 0;         // the second 0x0A is the end of the line
          break;
        }
				
				if (newLines >= numLines+1) { // Ignore first
          timeout = 0;         // Enough lines read
          break;
        }
      }
      replyBuffer[replyIdx] = c;
      replyIdx++;
    }

    if (timeout == 0) {
      break;
    }
    delay(1);
  }
	if ((replyIdx == 0) && (timeout == 0)) {
		//DEBUG_PRINTLN(F("\tE,readLine,Timeout"));
	}
  replyBuffer[replyIdx] = 0;  // null term
  return replyIdx;
}

bool UC20::sendParseReply(const char *send, const char *reply, uint16_t *value, char divider, uint8_t index) {
	bool replyOK = false;
	getReply(send);
	
	replyOK = parseReply(reply, value, divider, index);
	
	if (replyOK) {
		readLine(); // eat 'OK'
	}
	
  return replyOK;
}

bool UC20::sendParseReply(const char *send,const char *reply, uint32_t *value, char divider, uint8_t index) {
	bool replyOK = false;
	getReply(send);
	
	replyOK = parseReply(reply, value, divider, index);
	
	if (replyOK) {
		readLine(); // eat 'OK'
	}
	
  return replyOK;
}

bool UC20::sendParseReply(const char *send,const char *reply, char *value, uint16_t maxLen, char divider, uint8_t index) {
	bool replyOK = false;
	getReply(send);
	
	replyOK = parseReply(reply, value, maxLen, divider, index);
	
	if (replyOK) {
		readLine(); // eat 'OK'
	}
	
  return replyOK;
}

bool UC20::parseReply(const char *reply, int16_t *value, char divider, uint8_t index) {
  char *p = strstr(replyBuffer, reply);  // get the pointer to the value
  if (p == 0) return false;
  p += strlen(reply);
  for (uint8_t i = 0; i < index; i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }
  *value = atoi(p);

  return true;
}

bool UC20::parseReply(const char *reply, uint16_t *value, char divider, uint8_t index) {
  char *p = strstr(replyBuffer, reply);  // get the pointer to the value
  if (p == 0) return false;
  p += strlen(reply);
  for (uint8_t i = 0; i < index; i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }
  *value = atoi(p);

  return true;
}

bool UC20::parseReply(const char *reply, uint32_t *value, char divider, uint8_t index) {
  char *p = strstr(replyBuffer, reply);  // get the pointer to the value
  if (p == 0) return false;
  p += strlen(reply);
  for (uint8_t i = 0; i < index; i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }
  *value = atol(p);

  return true;
}

bool UC20::parseReply(const char *reply, char *value, uint16_t maxLen, char divider, uint8_t index) {
  char *p = strstr(replyBuffer, reply);  // get the pointer to the value
  if (p == 0) return false;
  p += strlen(reply);
  for (uint8_t i = 0; i < index; i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }	
	
	strncpy(value, p, maxLen);

  return true;
}

bool UC20::parseReplyQuoted(const char *toreply, char *v, uint16_t maxLen, char divider, uint8_t index) {
  uint8_t i = 0, j = 0;
  // Verify response starts with toreply.
  char *p = prog_char_strstr(replyBuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p += prog_char_strlen((prog_char*)toreply);

  // Find location of desired response field.
  for (i = 0; i < index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  // Copy characters from response field into result string.
  for (i = 0, j = 0; j < maxLen && i < strlen(p); ++i) {
    // Stop if a divider is found.
    if (p[i] == divider)
      break;
    // Skip any quotation marks.
    else if (p[i] == '"')
      continue;
    v[j++] = p[i];
  }

  // Add a null terminator if result string buffer was not filled.
  if (j < maxLen)
    v[j] = '\0';

  return true;
}

const __FlashStringHelper* UC20::getCMEerrorString(char *errResponse) {
	uint8_t errCode = 0;
	char tmpValue[4];
	char *chResponse = 0;
	chResponse = strstr(errResponse, REPLY_CME_ERROR);	
	
	if (chResponse != NULL) {
		strncpy(tmpValue, errResponse + 12, strlen(errResponse) - 12);
		errCode = atoi(tmpValue);
	} else {
		errCode = 255; // Unknown error code
	}
	
  switch (errCode) {
		case  0: return F("Phone failure");
		case  1: return F("No connection to phone");
		case  2: return F("Phone-adaptor link reserved");
		case  3: return F("Operation not allowed");
		case  4: return F("Operation not supported");
		case  5: return F("PH-SIM PIN required");
		case  6: return F("PH-FSIM PIN required");
		case  7: return F("PH-FSIM PUN required");
		case 10: return F("SIM not inserted");
		case 11: return F("SIM PIN required");
		case 12: return F("SIM PUK required");
		case 13: return F("SIM failure");
		case 14: return F("SIM busy");
		case 15: return F("SIM wrong");
		case 16: return F("Incorrect password");
		case 17: return F("SIM PIN2 required");
		case 18: return F("SIM PUK2 required");
		case 20: return F("Memory full");
		case 21: return F("Invalid index");
		case 22: return F("Not found");
		case 23: return F("Memory failure");
		case 24: return F("Text string too long");
		case 25: return F("Invalid characters in text string");
		case 26: return F("Dial string too long");
		case 27: return F("Invalid characters in dial string");
		case 30: return F("No network service");
		case 31: return F("Network timeout");
		case 32: return F("Network not allowed - emergency calls only");
		case 40: return F("Network personalization PIN required");
		case 41: return F("Network personalization PUN required");
		case 42: return F("Network subset personalization PIN required");
		case 43: return F("Network subset personalization PUK required");
		case 44: return F("Service provider personalization PIN required");
		case 45: return F("Service provider personalization PUK required");
		case 46: return F("Corporate personalization PIN required");
		case 47: return F("Corporate personalization PUK required");
		default: return F("Unknown error");
  }
}

const __FlashStringHelper* UC20::getCMSerrorString(char *errResponse) {
	uint16_t errCode = 0;
	char tmpValue[4];
	char *chResponse = 0;
	chResponse = strstr(errResponse, REPLY_CMS_ERROR);	
	
	if (chResponse != NULL) {
		strncpy(tmpValue, errResponse + strlen(REPLY_CMS_ERROR), strlen(errResponse) - strlen(REPLY_CMS_ERROR));
		errCode = atoi(tmpValue);
	} else {
		errCode = 550; // Unknown error code
	}
	
  switch (errCode) {
		case 300: return F("ME failure");
		case 301: return F("SMS ME reserved");
		case 302: return F("Operation not allowed");
		case 303: return F("Operation not supported");
		case 304: return F("Invalid PDU mode");
		case 305: return F("Invalid text mode");
		case 310: return F("SIM not inserted");
		case 311: return F("SIM pin necessary");
		case 312: return F("PH SIM pin necessary");
		case 313: return F("SIM failure");
		case 314: return F("SIM busy");
		case 315: return F("SIM wrong");
		case 316: return F("SIM PUK required");
		case 317: return F("SIM PIN2 required");
		case 318: return F("SIM PUK2 required");
		case 320: return F("Memory failure");
		case 321: return F("Invalid memory index");
		case 322: return F("Memory full");
		case 500: return F("Unknown");
		case 512: return F("SIM not ready");
		case 513: return F("Message length exceeds");
		case 514: return F("Invalid request parameters");
		case 515: return F("ME storage failure");
		case 517: return F("Invalid service mode");
		case 528: return F("More message to send state error");
		case 529: return F("MO SMS is not allow");
		case 530: return F("GPRS is suspended");
		case 531: return F("ME storage full");
		default: return F("Unknown error");
  }
}

/* Code Serial */
String UC20::readStringUntil(char data) {
	 return _Serial->readStringUntil(data);
}

size_t UC20::print(String data) {
     return _Serial->print(data);
}

size_t UC20::println(String data) {
     return _Serial->println(data);
}

void UC20::print(unsigned char data, int type) {
	_Serial->print(data,type);
}


void UC20:: print(int data, int type) {
	_Serial->print(data,type);
}

void UC20:: print(unsigned int data, int type) {
	_Serial->print(data,type);
}

void UC20:: print(long data, int type) {
	_Serial->print(data,type);
}

size_t UC20::print(String data, int type) {
   int i=0;
   while (data[i]) {
		_Serial->print(data[i],type);
		i++;   
   }
   return (i-1);
   
   // Serial.print(data,type);
	// return _Serial->print(data,type);
}

size_t UC20::println(String data, int type) {
	int i=0;
	while (data[i]) {
		_Serial->print(data[i],type);
		i++;   
	}
	_Serial->println("");
  return (i+1);
	//return _Serial->println(data,type);
}

int UC20::peek() {
	return _Serial->peek();
}

size_t UC20::write(uint8_t byte) {
	return _Serial->write(byte);
}

int UC20::read() {
	return _Serial->read();
}

int UC20::available() {
	return _Serial->available();
}

void UC20::flush() {
	_Serial->flush();
}

void UC20::flushInput() {
	// Read all available serial input to flush pending data.
	uint16_t timeoutLoop = 0;
	while (timeoutLoop++ < 40) {
		while (available()) {
				read();
				timeoutLoop = 0;  // If char was received reset the timer
		}
		delay(1);
	}
}

void UC20::resetWatchdog() {
	(*_watchdogCallback)();
}