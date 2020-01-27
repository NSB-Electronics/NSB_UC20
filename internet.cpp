#include "internet.h"

INTERNET::INTERNET(){}

bool INTERNET::configure(unsigned char contextid, unsigned char context_type, String apn, String user, String password, unsigned char auth) {
	String str = "AT+QICSGP=";
			str += String(contextid)+",";
			str += String(context_type)+",";
			str += "\""+apn+"\",";
			str	+= "\""+user+"\",";
			str	+= "\""+password+"\",";
			str	+= String(auth);
	gsm.println(str);
	return (gsm.wait_ok(10000));
}

bool INTERNET::configure(unsigned char contextid, String apn, String user, String password) {
	return configure(contextid, 1, apn, user, password, 1);
}

bool INTERNET::configure(String apn, String user, String password) {
	return configure(1, 1, apn, user, password, 1);
}

bool INTERNET::configDNS(unsigned char contextid, String pridnsaddr, String secdnsaddr) {
	String str = "AT+QIDNSCFG=";
			str += String(contextid)+",";
			str += "\""+pridnsaddr+"\",";
			str += "\""+secdnsaddr+"\"";
	gsm.println(str);
	return (gsm.wait_ok(10000));
}

bool INTERNET::configDNS(String pridnsaddr, String secdnsaddr) {
	return configDNS(1, pridnsaddr, secdnsaddr);
}

bool INTERNET::configDNS(unsigned char contextid) {
	return configDNS(contextid, "8.8.8.8", "9.9.9.9");
}

bool INTERNET::configDNS() {
	return configDNS(1, "8.8.8.8", "9.9.9.9");
}

bool INTERNET::connect(unsigned char contextid) {
	String 	str = "AT+QIACT=";
			str += String(contextid);
	gsm.println(str);
	return (gsm.wait_ok(10000));
}

bool INTERNET::connect() {
	return connect(1);
}

bool INTERNET::disconnect(unsigned char contextid) {
	String str = "AT+QIDEACT=";
			str += String(contextid);
	gsm.println(str);
	return (gsm.wait_ok(10000));
}

bool INTERNET::disconnect() {
	return disconnect(1);
}

String INTERNET::getIP() {
	gsm.println(F("AT+QIACT?"));
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	while (1)	{
		String req = gsm.readStringUntil('\n');	
	  
	  if (req.indexOf(F("+QIACT:")) != -1) {
			char index1 = req.indexOf(F("\""));
			char index2 = req.indexOf(F("\""),index1+1);
			gsm.wait_ok(1000);
			return (req.substring(index1+1,index2));
		}
		if (gsm.time_out(20000)) {
			return (F("GetIP Timeout"));
		}
	}
	return (F(""));
}
