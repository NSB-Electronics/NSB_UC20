#include "internet.h"

INTERNET::INTERNET(){}

bool INTERNET::configure(unsigned char contextid, unsigned char context_type, String apn, String user, String password, unsigned char auth) {		
	gsm.print(F("AT+QICSGP="));
	gsm.print(String(contextid));
	gsm.print(F(","));
	gsm.print(String(context_type));
	gsm.print(F(",\""));
	gsm.print(apn);
	gsm.print(F("\",\""));
	gsm.print(user);
	gsm.print(F("\",\""));
	gsm.print(password);
	gsm.print(F("\","));
	gsm.println(String(auth));
	return (gsm.wait_ok(10000));
}

bool INTERNET::configure(unsigned char contextid, String apn, String user, String password) {
	return configure(contextid, 1, apn, user, password, 1);
}

bool INTERNET::configure(String apn, String user, String password) {
	return configure(1, 1, apn, user, password, 1);
}

bool INTERNET::configDNS(unsigned char contextid, String pridnsaddr, String secdnsaddr) {	
	gsm.print(F("AT+QIDNSCFG="));
	gsm.print(String(contextid));
	gsm.print(F(",\""));
	gsm.print(pridnsaddr);
	gsm.print(F("\",\""));
	gsm.print(secdnsaddr);
	gsm.println(F("\""));
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
	gsm.print(F("AT+QIACT="));
	gsm.println(String(contextid));
	return (gsm.wait_ok(150000));
}

bool INTERNET::connect() {
	return connect(1);
}

bool INTERNET::disconnect(unsigned char contextid) {
	gsm.print(F("AT+QIDEACT="));	
	gsm.println(String(contextid));
	return (gsm.wait_ok(40000));
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
		if (gsm.time_out(60000)) {
			return (F("GetIP Timeout"));
		}
	}
	return (F(""));
}
