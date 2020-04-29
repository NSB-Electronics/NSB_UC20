#include "ssl.h"

SSL::SSL() {
	
}

bool SSL::setContextID(unsigned char contextid) {
	gsm.print(F("AT+QFTPCFG=\"sslctxid\","));
	gsm.println(String(contextid));
	return (gsm.wait_ok_ndb(3000));
}

bool SSL::setSSLversion(unsigned char contextid, unsigned char sslver) {
	//AT+QSSLCFG="sslversion",1,1
	gsm.print(F("AT+QSSLCFG=\"sslversion\","));
	gsm.println(String(contextid)+","+String(sslver));
	return (gsm.wait_ok_ndb(3000));
}

bool SSL::setCiphersuite(unsigned char contextid, String cipher) {
	gsm.print(F("AT+QSSLCFG=\"ciphersuite\","));
	gsm.println(String(contextid)+","+cipher);
	return (gsm.wait_ok_ndb(3000));
}

bool SSL:: setSeclevel(unsigned char contextid, unsigned char level) {
	gsm.print(F("AT+QSSLCFG=\"seclevel\","));
	gsm.println(String(contextid)+","+String(level));
	return (gsm.wait_ok_ndb(3000));
}

bool SSL::setCertificate(unsigned char contextid, String cacertpath) {
	gsm.print(F("AT+QSSLCFG=\"cacert\","));
	gsm.print(String(contextid));
	gsm.print(F(",\""));
	gsm.print(cacertpath);
	gsm.println("\"");
	return (gsm.wait_ok_ndb(3000));
}

bool SSL::setCertificate(unsigned char contextid) {
	return setCertificate(contextid, "UFS:cacert.pem");
}

bool SSL::setIgnorelocaltime(unsigned char contextid, bool ignoretime) {
	gsm.print(F("AT+QSSLCFG=\"ignorelocaltime\","));
	gsm.println(String(contextid)+","+String(ignoretime));
	return (gsm.wait_ok_ndb(3000));
}


int SSL::open(unsigned char pdpid, unsigned char contextid, unsigned char clientid, String serverid, String port, unsigned char acc_mode) {
	//AT+QSSLOPEN=1,1,4,"test-491ef.firebaseio.com",443,2
	gsm.print(F("AT+QSSLOPEN="));
	gsm.println(String(pdpid)+","+String(contextid)+","+String(clientid)+",\""+serverid+"\","+port+","+String(acc_mode));

	if (gsm.wait_ok_ndb(10000)) {
		const long interval = 5000; 
		unsigned long previousMillis = millis(); 
		unsigned char flag=1;
		
		while (flag) {
			if (gsm.available())	{
				String req = gsm.readStringUntil('\n');
				if (req.indexOf(F("+QSSLOPEN"))!= -1) {
					//Serial.println(req);
					int index = req.indexOf(F(","));
					int res = req.substring(index+1).toInt();
					if(res == 0)
						return (1);
					else
						return (res);
				}
			}
			unsigned long currentMillis = millis();
			if (currentMillis - previousMillis >= interval) {
				//Serial.println("out");
				previousMillis = currentMillis;
				return (-1);
			}			
		}
	}
	//Serial.print("open timeout");
	return (-2);
}

bool SSL::close(unsigned char clientid) {
	gsm.print(F("AT+QSSLCLOSE="));
	gsm.println(String(clientid));
	
	const long interval = 10000; 
	unsigned long previousMillis = millis(); 
	unsigned char flag=1;
	while (flag)	{
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//Serial.println(req);
			if (req.indexOf(F("OK"))!= -1) {
				String req = gsm.readStringUntil('\n');
				return (true);
			}			
		}
		
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			stopSend();
			//Serial.println("close time out");
			previousMillis = currentMillis;
			return (false);
		}			
	}
	//return(gsm.wait_ok_ndb(15000));
}

bool SSL::startSend(unsigned char clientid)
{
	const long interval = 3000; 
	gsm.print("AT+QSSLSEND=");
	gsm.println(String(clientid));
	
	unsigned long previousMillis = millis(); 
	while (1)	{
		if(gsm.available())	{
			if(gsm.read()=='>')	{
				//Serial.println("Send ready");
				gsm.debug("send ready\r\n");
				return (true);
			}	
		}
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			previousMillis = currentMillis;
			return (false);
		}			
		//String req = gsm.readStringUntil('>');
	}
}

bool SSL::startSend(unsigned char clientid, int len) {
	const long interval = 3000; 
	unsigned char flag_retry = 0;
	gsm.print(F("AT+QSSLSEND="));
	gsm.print(String(clientid));
	gsm.print(",");
	gsm.println(String(len));
	
	unsigned long previousMillis = millis(); 
	unsigned long currentMillis;
	while (1) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//Serial.println(req);
			if (req.indexOf(F(">")) != -1) {
				gsm.debug(F("START SEND OK\r\n"));
				//Serial.println(F("START SEND OK"));
				return (true);
			} else if (req.indexOf(F("ERROR")) != -1) {
				gsm.debug(F("START SEND ERROR\r\n"));
				//Serial.println(F("START SEND ERROR"));
				return (false);
			} else if (req.indexOf(F("SEND FAIL")) != -1) {
				gsm.debug(F("START SEND FAIL\r\n"));
				//Serial.println(F("START SEND FAIL"));
				return (false);
			}
		}
		currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			flag_retry++;
			gsm.println(F("AT"));
			gsm.wait_ok_ndb(1000);
			gsm.print(F("AT+QSSLSEND="));
			gsm.print(String(clientid));
			gsm.print(",");
			gsm.println(String(len));
			
			previousMillis = currentMillis;
			if (flag_retry >= 3)	{
				gsm.debug(F("startSend error (timeout)\r\n"));
				//Serial.println(F("startSend error (timeout)"));
				return (false);
			}
		}			
		//String req = gsm.readStringUntil('>');
	}
}

bool SSL::startSend() {
	return (startSend(0));
}

bool SSL::stopSend() {
	gsm.write(0x1A);
	const long interval = 10000; 
	unsigned long previousMillis = millis(); 
	unsigned char flag = 1;
	while (flag) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//Serial.println(req);
			if (req.indexOf(F("SEND OK"))!= -1) {
				return (true);
			}
			if(req.indexOf(F("SEND FAIL")) != -1) {
				return (false);
			}
		}
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			gsm.write(0x1A);
			delay(1000);
			//Serial.println("out");
			previousMillis = currentMillis;
			return (false);
		}
	}
}

bool SSL::waitSendFinish() {
	const long interval = 3000; 
	unsigned long previousMillis = millis(); 
	unsigned long currentMillis;
	unsigned char cnt = 0;
	
	while (1) {
		
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			if (req.indexOf(F("SEND OK")) != -1) {
				gsm.debug(F("SEND OK\r\n"));
				//Serial.println(F("WAIT SEND OK"));
				return (true);
			} else if (req.indexOf(F("ERROR")) != -1) {
				gsm.debug(F("WAIT SEND ERROR\r\n"));
				//Serial.println(F("WAIT SEND ERROR"));
				return (false);
			} else if (req.indexOf(F("SEND FAIL")) != -1) {
				gsm.debug(F("WAIT SEND FAIL\r\n"));
				//Serial.println(F("WAIT SEND FAIL"));
				return (false);
			}
		}
		
		currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			cnt++;
			if (cnt > 3) {
				gsm.debug("waitSend error (timeout)\r\n");
				Serial.println("waitSend error (timeout)");
				return (false);
			}
			previousMillis = currentMillis; 	
		}
			
	}
}

bool SSL::waitRead(long time) {
	const long interval = time; 
	unsigned long previousMillis = millis(); 
	unsigned char flag = 1;
	while (flag)	{
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//Serial.println(req);
			if (req.indexOf(F("+QSSLURC: \"recv\"")) != -1) {
				return (true);
			}
		
		}
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			//Serial.println("Read Timeout");
			previousMillis = currentMillis;
			return (false);
		}			
	}
	
}

int SSL::read(unsigned char contextid) {
	gsm.print(F("AT+QSSLRECV="));
	gsm.print(String(contextid));
	gsm.println(",1500");
	
	const long interval = 3000; 
	unsigned long previousMillis = millis(); 
	unsigned char flag = 1;
	while (flag) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//Serial.println(req);
			
			if (req.indexOf(F("+QSSLRECV:")) != -1) {	
				int index = req.indexOf(":");
				int len = req.substring(index+2).toInt();
				//Serial.println("ssl read len");
				//Serial.println(len);
				return (len);
			}
		
		}
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			//Serial.println("Read Timeout");
			previousMillis = currentMillis;
			return (false);
		}			
	}
	
}


int SSL::state() {
	gsm.println(F("AT+QSSLSTATE"));
	const long interval = 5000; 
	unsigned long previousMillis = millis(); 
	unsigned char flag = 1;
	int sslState = -1;
	while (flag) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//Serial.println(req);
			if (req.indexOf(F("+QSSLSTATE:")) != -1) {
				int idxComma = 0;
				for (byte k = 0; k < 5; k++) {
					idxComma = req.indexOf(',') + 1;
					req = req.substring(idxComma, req.length()+1);
				}
				idxComma = req.indexOf(',');
				sslState = req.substring(0, idxComma).toInt();
			}
			if (req.indexOf(F("OK")) != -1) {
				return sslState;
			}
		
		}
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			//Serial.println("out");
			previousMillis = currentMillis;
			return sslState;
		}			
	}
}



int SSL::clear_buf(unsigned char contextid) {
	gsm.print(F("AT+QSSLRECV="));
	gsm.println(String(contextid)+",1500");
	
	const long interval = 5000; 
	unsigned long previousMillis = millis(); 
	unsigned char flag = 1;
	while (flag) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			if(req.indexOf(F("OK")) != -1) {
					return (1);
			}	else {
				//Serial.print(req);
			}
		}
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			//Serial.println("out");
			previousMillis = currentMillis;
			return (0);
		}			
	}
	
}

bool SSL::receiveAvailable() {
	while (gsm.available()) {
		String req = gsm.readStringUntil('\n');
		if (req.indexOf(F("+QSSLURC: \"recv\"")) != -1) {
			unsigned char index = req.indexOf(F(","));
			receiveClientID = req.substring(index+1).toInt();
			return (true);
		}
		else {
			return (false);
		}
	}
}

int SSL::readBuffer() {
	return(readBuffer(1500));
}

int SSL::readBuffer(unsigned char contextid, int max_len) {
	bool timedOut = false;
	unsigned char flag = 0;
	int readlen = 0;
	
	gsm.print(F("AT+QSSLRECV="));
	gsm.print(String(contextid));
	gsm.print(F(","));
	gsm.println(String(max_len));
	
	gsm.start_time_out();
	while (!timedOut) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			if (req.indexOf(F("+QSSLRECV:")) != -1)	{
				readlen = req.substring(req.indexOf(F(" "))+1).toInt();
				return (readlen);
			}
		}
		if (gsm.time_out(3000)) {
			gsm.debug(F("SSL ReadBuffer timeout: "));
			gsm.debug(String(flag));
			gsm.debug(F("\r\n"));
			//Serial.print("SSL ReadBuffer timeout: ");
			//Serial.println(flag);
			if (flag++==3)
				timedOut = true;
			gsm.start_time_out();
		}
	}
	return (0);
}

int SSL::readBuffer(int max_len) {
	return (readBuffer(0, max_len));
}


void SSL::write(char data) {
	gsm.write(data);
}

void SSL::print(int data) {
	gsm.print(String(data));
}

void SSL::println(int data) {
	gsm.println(String(data));
}

void SSL::print(String data) {
	gsm.print(data);
}

void SSL::println(String data) {
	gsm.println(data);
}