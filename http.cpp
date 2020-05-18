#include "http.h"


HTTP::HTTP() {
	
}

bool HTTP::begin(unsigned char context_ID) {
	//AT+QHTTPCFG="contextid",1 
	gsm.print(F("AT+QHTTPCFG=\"contextid\","));
	gsm.print(context_ID,DEC);
	gsm.println("");
	if (!gsm.wait_ok(3000))
		return (false);
	//AT+QHTTPCFG="responseheader",1 
	//gsm.print(F("AT+QHTTPCFG=\"responseheader\","));
	//gsm.print(context_ID,DEC);
	//gsm.println("");
	//return (gsm.wait_ok(3000));
}

bool HTTP::url(String url) {
	gsm.print(F("AT+QHTTPURL="));
	gsm.print(url.length(),DEC);
	gsm.println(F(",80"));
	while(!gsm.available()) {
		
	}
	gsm.start_time_out();
	unsigned char flag=1;
	while (flag) {
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("CONNECT")) != -1) {
			flag = 0;
		}
		if(req.indexOf(F("ERROR")) != -1) {
			return (false);
		}		
	}
	gsm.println(url);
	return (gsm.wait_ok(10000));
	
}
int HTTP::get(unsigned int rspTime) {
	bool timedOut = false;
	gsm.print(F("AT+QHTTPGET="));
	gsm.println(String(rspTime));
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	
	while (!timedOut) {
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("+QHTTPGET:")) != -1) {
			char index1 = req.indexOf(F(","));
			return (req.substring(index1+1,index1+4).toInt());
			
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (-1);
		}
		if (gsm.time_out((rspTime * 1000) + 5000)) {
			timedOut = true;
			gsm.debug(F("\r\nHTTP GET timeout"));
		}
	}

	return -1;	
}

int HTTP::get() {
	return get(80);
}

int HTTP::post(String data, unsigned int inputTime, unsigned int rspTime) {
	bool timedOut = false;	
	gsm.print(F("AT+QHTTPPOST="));
	gsm.print(String(data.length()));
	gsm.print(F(","));
	gsm.print(String(inputTime));
	gsm.print(F(","));
	gsm.println(String(rspTime));
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	unsigned char flag=1;
	while (flag){
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("CONNECT")) != -1) {
			flag=0;
		}
		if (req.indexOf(F("ERROR")) != -1) {
			//return (-1);
		}		
	}		
	gsm.print(data);
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	while (!timedOut) {
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("+QHTTPPOST")) != -1) {
			char index1 = req.indexOf(F(","));
			return (req.substring(index1+1,index1+4).toInt());
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (-1);
		}
		if (gsm.time_out((rspTime * 1000) + 5000)) {
			timedOut = true;
			gsm.debug(F("\r\nHTTP POST timeout"));
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


int HTTP::read(char *recvBuffer, unsigned int maxLen, unsigned int waitTime) {
	bool timedOut = false;
	char lineBuffer[maxLen+1];
	char *bufPtr = lineBuffer;
	char nextByte;
	gsm.print(F("AT+QHTTPREAD="));
	gsm.println(String(waitTime));
	gsm.start_time_out();
	
	*recvBuffer = '\0';
	
	while (!timedOut) {
		bufPtr = lineBuffer;
		nextByte = '\0';
		do {
			if (gsm.available()) {
				nextByte = gsm.read();
				if (nextByte != '\n' && nextByte != -1) {
					*bufPtr++ = nextByte;
					*bufPtr = '\0';		// Terminate the string always
				}
			}
			if ((bufPtr - lineBuffer) >= maxLen) {
				// No more space in char array, abort
				return (-1);
			}
			if (gsm.time_out((waitTime * 1000) + 5000)) {
				timedOut = true;
				gsm.debug(F("\r\nHTTP READ timeout"));
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
		if (gsm.time_out((waitTime * 1000) + 5000)) {
			timedOut = true;
			gsm.debug(F("\r\nHTTP READ timeout"));
		}
	}
	return (-1);
}

int HTTP::read(char *recvBuffer, unsigned int maxLen) {
	return read(recvBuffer, maxLen, 80);
}

int HTTP::read(char *recvBuffer) {
	return read(recvBuffer, 3000, 80);
}
	
void HTTP::ReadData() {
	gsm.println(F("AT+QHTTPREAD=80"));
//+QHTTPREAD: 0	
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
	gsm.start_time_out();
	while (1) {
		String req = gsm.readStringUntil('\n');	
	    if (req.indexOf(F("+QHTTPREADFILE: 0")) != -1) {
			return (true);
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (false);
		}		
	}
	
}