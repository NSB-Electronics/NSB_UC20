#include "smtp.h"

SMTP::SMTP(){}


bool SMTP::setSSLtype(unsigned char sslType) {
	gsm.print(F("AT+QSMTPCFG=\"ssltype\","));
	gsm.print(sslType, DEC);
	gsm.println("");
	return (gsm.wait_ok_ndb(3000));
}

bool SMTP::setSSLtype() {
	return (setSSLtype(SMTP_NO_SSL));
}

bool SMTP::setContextID(unsigned char contextid) {
	gsm.print(F("AT+QSMTPCFG=\"contextid\","));
	gsm.print(contextid, DEC);
	gsm.println("");
	return (gsm.wait_ok_ndb(3000));
}

bool SMTP::setContextID() {
	return (setContextID(1));
}

bool SMTP::setSSLctxID(unsigned char sslctxid) {
	gsm.print(F("AT+QSMTPCFG=\"sslctxid\","));
	gsm.print(sslctxid, DEC);
	gsm.println("");
	return (gsm.wait_ok_ndb(3000));
}

bool SMTP::setSSLctxID() {
	return (setSSLctxID(1));
}


bool SMTP::configServer(String srvaddr, unsigned int srvport) {	
	gsm.print(F("AT+QSMTPCFG=\"smtpserver\",\""));
	gsm.print(srvaddr);
	gsm.print(F("\","));
	gsm.print(srvport, DEC);
	gsm.println("");
	return (gsm.wait_ok_ndb(3000));
}


bool SMTP::setAccount(String username, String password) {
	gsm.print(F("AT+QSMTPCFG=\"account\",\""));
	gsm.print(username);
	gsm.print(F("\",\""));
	gsm.print(password);
	gsm.println(F("\""));
	return (gsm.wait_ok_ndb(3000));
}


bool SMTP::setSender(String senderName, String senderEmail) {
	gsm.print(F("AT+QSMTPCFG=\"sender\",\""));
	gsm.print(senderName);
	gsm.print(F("\",\""));
	gsm.print(senderEmail);
	gsm.println(F("\""));
	return (gsm.wait_ok_ndb(3000));
}


bool SMTP::addRecipient(unsigned char type, String emailaddr) {
	gsm.print(F("AT+QSMTPDST=1,"));
	gsm.print(type, DEC);
	gsm.print(F(",\""));
	gsm.print(emailaddr);
	gsm.println(F("\""));
	return (gsm.wait_ok_ndb(3000));
}


bool SMTP::addRecipient(String emailaddr) {
	return (addRecipient(SMTP_TO, emailaddr));
}

bool SMTP::deleteRecipients() {
	gsm.println(F("AT+QSMTPDST=0"));
	return (gsm.wait_ok_ndb(3000));
}

bool SMTP::setSubject(unsigned char charset, String subject) {
	gsm.print(F("AT+QSMTPSUB="));
	gsm.print(charset, DEC);
	gsm.print(F(",\""));
	gsm.print(subject);
	gsm.println(F("\""));
	return (gsm.wait_ok_ndb(3000));
}

bool SMTP::setSubject(String subject) {
	return (setSubject(SMTP_ASCII, subject));
}


bool SMTP::startBody(unsigned char charset, unsigned int bodyLength, unsigned int inputTime) {
	bool timedOut = false;
	String req;
	
	gsm.print(F("AT+QSMTPBODY="));
	gsm.print(charset, DEC);
	gsm.print(F(","));
	gsm.print(bodyLength, DEC);
	gsm.print(F(","));
	gsm.print(inputTime, DEC);
	gsm.println(F(""));
	
	while(!gsm.available()) {}
	gsm.start_time_out();
	
	while(!timedOut) {
		req = gsm.readStringUntil('\r\n');
		
		if (req.indexOf(F("CONNECT")) != -1) {
			return true;
		}
		if (gsm.time_out(3000)) {
			timedOut = true;
			Serial.println(F("\r\SMTP Start Body timeout"));
			gsm.debug(F("\r\SMTP Start Body timeout"));
		}
	}
	
	return false;
}

bool SMTP::startBody(unsigned int bodyLength) {
	startBody(SMTP_ASCII, bodyLength, 3);
}

bool SMTP::startBody() {
	startBody(SMTP_ASCII, 10240, 3);
}

void SMTP::addBody(String data) {
	gsm.print(data);

}

void SMTP::addBodyln(String data) {
	addBody(data+"\r\n");
}

int SMTP::stopBody(unsigned int inputTime) {
	bool timedOut = false;
	String req;
	int bodyLength = 0;
	
	delay(1100); // Otherwise +++ is misinterpreted as data
	gsm.print(F("+++")); // Send end input
	delay(1100);
	
	//while (!gsm.available()) {}
	gsm.start_time_out();
	
	while(!timedOut) {
		req = gsm.readStringUntil('\r\n');
		
		if (req.indexOf(F("+QSMTPBODY")) != -1) {
			char index1 = req.indexOf(F(":"));
			bodyLength = req.substring(index1+2).toInt();
			
		}
		if (req.indexOf(F("OK")) != -1) {
			return bodyLength;
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (-1);
		}
		if (gsm.time_out(inputTime+1000)) {
			timedOut = true;
			Serial.println(F("\r\SMTP Stop Body timeout"));
			gsm.debug(F("\r\SMTP Stop Body timeout"));
		}
	}
	return -1;
}


int SMTP::stopBody() {
	stopBody(3);
}


bool SMTP::addAttachment(unsigned char fileIndex, String filename) {
	gsm.print(F("AT+QSMTPATT=1,"));
	gsm.print(fileIndex, DEC);
	gsm.print(F(","));
	gsm.print(F("\""));
	gsm.print(filename);
	gsm.println(F("\""));
	
	return (gsm.wait_ok_ndb(3000));
}

bool SMTP::addAttachment(String filename) {
	return addAttachment(1, filename);
}

bool SMTP::sendEmail(unsigned int timeout) {
	bool timedOut = false;
	String req;
	gsm.print(F("AT+QSMTPPUT="));
	gsm.print(timeout, DEC);
	gsm.println("");
	
	return (gsm.wait_ok_ndb(3000));
	
	/*while (!gsm.available()) {}
	gsm.start_time_out();
	
	while(!timedOut) {
		req = gsm.readStringUntil('\r\n');
		Serial.println(req);
		if (req.indexOf(F("+QSMTPPUT")) != -1) {
			char index1 = req.indexOf(F(":"));
			char index2 = req.indexOf(F(","));
			if (index2 == -1) {
				index2 = sizeof(req);
			}
			
			if (req.substring(index1+2, index2).toInt() == 0) {
				return 1;
			} else {
				return req.substring(index1+2, index2).toInt();
			}
		}
		
		if (gsm.time_out(timeout*1000 + 5000)) {
			timedOut = true;
			gsm.debug(F("\r\SMTP SENDEMAIL timeout"));
		}
	}
	
	return 0; 
	*/
}


bool SMTP::sendEmail() {
	return sendEmail(300);
}

bool SMTP::clearContent() {
	gsm.println(F("AT+QSMTPCLR"));
	return (gsm.wait_ok_ndb(3000));
}
