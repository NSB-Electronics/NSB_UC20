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
	String req;
	unsigned char flag=1;
	while (flag) {
		req = gsm.readStringUntil('\n');	
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
	String req;
	String str = "AT+QHTTPGET=";
			str += String(rspTime);
	
	gsm.println(str);
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	
	while (!timedOut) {
		req = gsm.readStringUntil('\n');	
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
	String req;	
	String str = "AT+QHTTPPOST=";
			str += (data.length(), DEC);
			str += ",";
			str += String(inputTime);
			str += ",";
			str += String(rspTime);
	gsm.println(str);
	
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	unsigned char flag=1;
	while (flag){
		req = gsm.readStringUntil('\n');	
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
		req = gsm.readStringUntil('\n');	
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


String HTTP::read(unsigned int waitTime) {
	bool timedOut = false;
	String req;
	String returnStr;
	String str = "AT+QHTTPREAD=";
			str += String(waitTime);
	
	gsm.println(str);
	gsm.start_time_out();
	
	while (!timedOut) {
		req = gsm.readStringUntil('\r\n');	
	    //Serial.println(req);
		
		if (req.indexOf(F("+QHTTPREAD: 0")) != -1) {
			return (returnStr);
		} else {
			if ((req.indexOf(F("OK")) == -1) && (req.indexOf(F("CONNECT")) == -1)) {
				// Ignores OK and CONNECT to only return the HTTP response information
				returnStr += req;
				returnStr += "\r\n";
			}
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (req);
		}
		
		if (gsm.time_out((waitTime * 1000) + 5000)) {
			timedOut = true;
			gsm.debug(F("\r\nHTTP READ timeout"));
		}
	}
	return ("HTTP READ timeout");
}


String HTTP::read() {
	return read(80);
}
	
void HTTP::ReadData() {
	gsm.println("AT+QHTTPREAD=80");
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

