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

bool SSL::setCiphersuite(unsigned char contextid, String tls_rsa) {
	gsm.print(F("AT+QSSLCFG=\"ciphersuite\","));
	gsm.println(String(contextid)+","+tls_rsa);
	return (gsm.wait_ok_ndb(3000));
}

bool SSL:: setSeclevel(unsigned char contextid, unsigned char level) {
	gsm.print(F("AT+QSSLCFG=\"seclevel\","));
	gsm.println(String(contextid)+","+String(level));
	return (gsm.wait_ok_ndb(3000));
}

bool SSL::setCertificate(unsigned char contextid) {
	gsm.print(F("AT+QSSLCFG=\"cacert\","));
	gsm.print(String(contextid));
	gsm.println(F(",\"UFS:cacert.pem\""));
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

bool SSL::close(unsigned char contextid) {
	gsm.print(F("AT+QSSLCLOSE="));
	gsm.println(String(contextid));
	
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
	
	String buffer_="";
	unsigned long previousMillis = millis(); 
	unsigned long currentMillis;
	while (1) {
		if (gsm.available()) {
			char c = gsm.read();
			//Serial.write(c);
			if(c =='>')	{
				//gsm.debug("send ready");
				return (true);
			}	else {
				buffer_ += c;
				if (buffer_.indexOf(F("ERROR"))!=-1) {
					gsm.debug("Send Error\r\n");
					return (false);
				}
				if (buffer_.indexOf(F("NO CARRIER"))!=-1) {
					gsm.debug("NO CARRIER\r\n");
					return (false);
				}
			}
		/*	if(c =='E')
			{
				gsm.debug("send fail");
				return(false);
			}
*/			
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
				gsm.debug("send error (timeout)\r\n");
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
			if(req.indexOf(F("SEND FAIL"))!= -1) {
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
		currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			cnt++;
			if (cnt > 3) {
				gsm.debug("Error unfinish");
				return (false);
			}
			previousMillis = currentMillis; 	
		}
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//gsm.debug(req);
			if (req.indexOf(F("SEND OK")) != -1) {
				//Serial.println("SEND OK");
				return (true);	
			}
			if (req.indexOf(F("SEND FAIL")) != -1) {
				//Serial.println("SEND FAIL");
				gsm.debug(req);
				return (false);	
			}
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
			if (req.indexOf(F("+QSSLURC: \"recv\"")) != -1)
			{
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


bool SSL::state() {
	gsm.println(F("AT+QSSLSTATE"));
	const long interval = 5000; 
	unsigned long previousMillis = millis(); 
	unsigned char flag = 1;
	while (flag) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//Serial.println(req);
			if (req.indexOf(F("OK")) != -1) {
					return (true);
			}
		
		}
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			//Serial.println("out");
			previousMillis = currentMillis;
			return (false);
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
	const long interval = 3000; 
	unsigned long previousMillis = millis(); 
	unsigned long currentMillis; 
	unsigned char flag = 0;  
		
//	gsm.print(F("AT+QIRD="));
//	gsm.print(String(contextid));
//	gsm.print(",");
//	gsm.println(String(max_len));
	
	String str = "AT+QSSLRECV=";
		str += String(contextid);
		str	+= ",";
		str	+= String(max_len);
	gsm.debug(str);
	gsm.println(str);
	while (1) {
		currentMillis = millis();
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			if (req.indexOf(F("+QSSLRECV:")) != -1)	{
				int index = req.indexOf(F(" "));
				//Serial.println(req);
				int tmp = req.substring(index+1).toInt();
				//Serial.print("HERE: ");
				//Serial.println(tmp);
				//if (tmp != 0) {
					return (tmp);	
				//}
				//return(req.substring(index+1).toInt());	
			}
		}
		if (currentMillis - previousMillis >= interval) {
			if (flag++==3)
				return (0);
			gsm.debug(F("ReadBuffer timeout: "));
			gsm.debug(String(flag));
			gsm.debug(F("\r\n"));
			//Serial.print("ReadBuffer timeout: ");
			//Serial.println(flag);
			gsm.debug(str);
			gsm.println(str);
			previousMillis = millis(); 
		}
	}
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