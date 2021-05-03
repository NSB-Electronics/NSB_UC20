#include "http.h"


HTTP::HTTP() {
	
}

bool HTTP::begin(uint8_t contextid) {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=\"contextid\",%d", CMD_HTTP_CONF, contextid);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	sprintf(cmd, "%s=\"responseheader\",%d", CMD_HTTP_CONF, 0);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool HTTP::setURL(char *url, uint16_t urlLen, uint16_t timeout) {
	bool responseOK = false;
	char cmd[30];
	
	if ((urlLen > 0) && (urlLen <= 700)) {
		sprintf(cmd, "%s=%d,%d", CMD_HTTP_URL, urlLen, timeout);
		responseOK = gsm.sendCheckReply(cmd, AVT1_CONNECT);
		if (responseOK) {
			DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(url);
			gsm.println(url);
			responseOK = gsm.checkReply(REPLY_OK); // read OK
		} else {
			// Command failed
		}
	} else {
		// url length out of range
	}
	
  return responseOK;	
}
bool HTTP::get(uint16_t *err, uint16_t *httpRspCode, uint16_t *contentLen, uint16_t rspTime) {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=%d", CMD_HTTP_GET, rspTime);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	if (responseOK) {
		responseOK = gsm.waitReply(REPLY_HTTP_GET, rspTime*1000);
		if (responseOK) {
			responseOK = gsm.parseReply(REPLY_HTTP_GET, err, ',', 0);
			responseOK &= gsm.parseReply(REPLY_HTTP_GET, httpRspCode, ',', 1);
			responseOK &= gsm.parseReply(REPLY_HTTP_GET, contentLen, ',', 2);
		} else {
			// Response wrong
		}		
	} else {
		// Command failed
	}
	
  return responseOK;
}

int HTTP::post(String data, uint16_t inputTime, uint16_t rspTime) {
	bool timedOut = false;	
	gsm.print(F("AT+QHTTPPOST="));
	gsm.print(String(data.length()));
	gsm.print(F(","));
	gsm.print(String(inputTime));
	gsm.print(F(","));
	gsm.println(String(rspTime));
	while (!gsm.available()) {
		
	}
	unsigned long timeout = millis();
	unsigned char flag=1;
	while (flag){
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("CONNECT")) != -1) {
			flag=0;
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (-1);
		}
		
		if (millis() - timeout > 5000) {
			return (-1);
		}
	}		
	gsm.print(data);
	while (!gsm.available()) {
		
	}
	timeout = millis();
	while (!timedOut) {
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("+QHTTPPOST")) != -1) {
			char index1 = req.indexOf(F(","));
			return (req.substring(index1+1,index1+4).toInt());
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (-1);
		}
		if (millis() - timeout > ((rspTime * 1000) + 5000)) {
			timedOut = true;
			DEBUG_PRINTLN(F("\t---- HTTP POST timeout"));
		}
	}		
	
	return -1;
}

int HTTP::post(String data) {
	return (post(data, 80, 80));
}


int HTTP::post() {
	return (post("", 80, 80));
}


int16_t HTTP::read(char *recvBuffer, uint16_t maxLen, uint16_t waitTime) {
	bool timedOut = false;
	char lineBuffer[maxLen+1];
	char *bufPtr = lineBuffer;
	char nextByte;
	
	unsigned long timeout = millis();
	
	char cmd[30];
	
	sprintf(cmd, "%s=%d", CMD_HTTP_READ, waitTime);
	gsm.send(cmd);
	
	*recvBuffer = '\0';
	
	while (!timedOut) {
		if (timeout % 100 == 0) {
			gsm.resetWatchdog();
		}
		bufPtr = lineBuffer;
		nextByte = '\0';
		do {
			if (gsm.available()) {
				nextByte = gsm.read();
				//Serial.print(nextByte);
				if (nextByte != '\n' && nextByte != -1) {
					*bufPtr++ = nextByte;
					*bufPtr = '\0';		// Terminate the string always
				}
			}
			if ((bufPtr - lineBuffer) >= maxLen) {
				// No more space in char array, abort
				return (-1);
			}
			if (millis() - timeout > ((waitTime * 1000) + 5000)) {
				timedOut = true;
				DEBUG_PRINTLN(F("\t---- HTTP READ timeout"));
				return (-1);
			}
		} while (nextByte != '\n');

		if (strstr(lineBuffer, "ERROR") == lineBuffer) {
			return (-1);
		}
		
		if (strstr(lineBuffer, "OK") == lineBuffer) {
			lineBuffer[0] = '\0'; // Discard OK at start of response
			bufPtr = lineBuffer; // Go back to start of lineBuffer
			continue;
		}

		if (strstr(lineBuffer, "CONNECT") == lineBuffer) {
			lineBuffer[0] = '\0'; // Discard CONNECT at start of response
			bufPtr = lineBuffer; // Go back to start of lineBuffer
			continue;
		}

//		Serial.print("->");
//	    Serial.print(lineBuffer);
//		Serial.println("<-");

		if (strstr(lineBuffer,"+QHTTPREAD: 0") != NULL) {
			return (strlen(recvBuffer));
		} else {
			if (bufPtr > lineBuffer) {
				*bufPtr++ = '\r';
				*bufPtr++ = '\n';
				*bufPtr = '\0';		// Terminate the string always
			}
			strcat(recvBuffer,lineBuffer);
		}
		if (millis() - timeout > ((waitTime * 1000) + 5000)) {
			timedOut = true;
			DEBUG_PRINTLN(F("\t---- HTTP READ timeout"));
		}
	}
	return (-1);
	
	
}

bool HTTP::SaveResponseToMemory(String pattern,String Filename) {
	if (pattern == UFS)
		gsm.print(F("AT+QHTTPREADFILE=\""));
	if (pattern == RAM)
		gsm.print(F("AT+QHTTPREADFILE=\"RAM:"));	
	gsm.print(Filename);
	gsm.println(F("\",80"));
	
	while(!gsm.available())	{
		
	}
	unsigned long timeout = millis();
	while (1) {
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("+QHTTPREADFILE: 0")) != -1) {
			return (true);
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (false);
		}
		if (millis() - timeout > 5000) {
			return (false);
		}
	}
	
}

const __FlashStringHelper* HTTP::getHTTPErrorString(uint16_t errorCode) {
  switch (errorCode) {
		case   0: return F("Operation successful");
		case 701: return F("HTTP unknown error");
		case 702: return F("HTTP timeout");
		case 703: return F("HTTP busy");
		case 704: return F("HTTP UART busy");
		case 705: return F("HTTP no get/post request");
		case 706: return F("HTTP network busy");
		case 707: return F("HTTP network open fail");
		case 708: return F("HTTP network no config");
		case 709: return F("HTTP network deactivated");
		case 710: return F("HTTP network error");
		case 711: return F("HTTP URL error");
		case 712: return F("HTTP empty URL");
		case 713: return F("HTTP IP address error");
		case 714: return F("HTTP DNS error");
		case 715: return F("HTTP socket create error");
		case 716: return F("HTTP socket connect error");
		case 717: return F("HTTP socket read error");
		case 718: return F("HTTP socket write error");
		case 719: return F("HTTP socket close");
		case 720: return F("HTTP data encode error");
		case 721: return F("HTTP data decode error");
		case 722: return F("HTTP read timeout");
		case 723: return F("HTTP response fail");
		case 724: return F("Incoming call busy");
		case 725: return F("Voice call busy");
		case 726: return F("Input timeout");
		case 727: return F("Wait data timeout");
		case 728: return F("Wait HTTP response timeout");
		case 729: return F("Alloc memory fail");
		case 730: return F("Invalid parameter");
		default:  return F("Unknown error");
  }
}

const __FlashStringHelper* HTTP::getHTTPResponseString(uint16_t responseCode) {
  switch (responseCode) {
		case 200: return F("OK");
		case 403: return F("Forbidden");
		case 404: return F("Not found");
		case 409: return F("Conflict");
		case 411: return F("Length required");
		case 500: return F("Internal Server error");
		default:  return F("Unknown response");
  }
}