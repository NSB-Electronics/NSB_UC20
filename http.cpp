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
	unsigned int pBuffer = 0;
	gsm.print(F("AT+QHTTPREAD="));
	gsm.println(String(waitTime));
	gsm.start_time_out();
	
	while (!timedOut) {
		String req = gsm.readStringUntil('\r\n');	
	    //Serial.println(req);
		if (req.indexOf(F("+QHTTPREAD: 0")) != -1) {
			recvBuffer[pBuffer] = '\0';
			return (pBuffer);
		} else {
			if ((req.indexOf(F("OK")) > 0)) {
				// If the OK is in the response function must still return it as it is part of the message
				if (req.length() > 1) req.concat("\r\n");
				for (int k = 0; k < req.length(); k++) {
					recvBuffer[pBuffer] = req.charAt(k);
					pBuffer++;
					if (pBuffer >= maxLen-1) {
						// No more space in char array, abort
						recvBuffer[pBuffer] = '\0';
						return (-1);
					}
				}
			} else if ((req.indexOf(F("OK")) == -1) && (req.indexOf(F("CONNECT")) == -1)) {
				// Ignores modem OK and CONNECT to only return the HTTP response information
				if (req.length() > 1) req.concat("\r\n");
				for (int k = 0; k < req.length(); k++) {
					recvBuffer[pBuffer] = req.charAt(k);
					pBuffer++;
					if (pBuffer >= maxLen-1) {
						// No more space in char array, abort
						recvBuffer[pBuffer] = '\0';
						return (-1);
					}
				}
			}
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (-1);
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