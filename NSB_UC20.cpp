// Based on TEE_UG20 by ThaiEasyElec
// https://github.com/ThaiEasyElec/TEE_UC20_Shield


#include "NSB_UC20.h"

int START_PIN = 4;

unsigned long previousMillis_timeout = 0; 

void event_null(String data){}

UC20::UC20() {
	Event_debug = event_null;
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

void UC20::debug(String data) {
	(*Event_debug)(data);
}

void UC20::setPowerKeyPin(int pin) {
	START_PIN = pin;
}

bool UC20::powerOn() {
	
	String req="";
	//_Serial->println(F("AT"));
	
	pinMode(START_PIN, OUTPUT);
	
	digitalWrite(START_PIN, HIGH);
	delay(1000);
	digitalWrite(START_PIN, LOW);
	delay(1000);
				
	/*
	while(!_Serial->available())
	{
		delay(1);
	}
	start_time_out();
	*/
	
	unsigned long pv_out = millis(); 
	
	while (1) {
		if (_Serial->available()) {
			req = _Serial->readStringUntil('\n');	
			// debug(req);
			if (req.indexOf(F("RDY")) != -1) {
				start_time_out();
				debug(F("Power ON"));
				return true;
			}
			if (req.indexOf(F("POWERED DOWN")) != -1) {
				//start_time_out();
				debug(F("Power OFF"));
				pinMode(START_PIN, OUTPUT);
				digitalWrite(START_PIN, HIGH);
				delay(1000);
				digitalWrite(START_PIN, LOW);
				delay(1000);
			}				
	  }
	  unsigned long current_out = millis();
			//debug("x");
	  if (current_out - pv_out >= (10000)) {
			digitalWrite(START_PIN, HIGH);
			delay(1000);
			digitalWrite(START_PIN, LOW);
			delay(1000);
			debug(F("Power Retry"));
			pv_out = current_out;
		}
  }
	return false;
}

bool UC20::powerOff() {
	
	_Serial->println(F("AT+QPOWD"));
	
	pinMode(START_PIN, OUTPUT);
	digitalWrite(START_PIN, HIGH);
  delay(1000);
	digitalWrite(START_PIN, LOW);
	delay(1000);
	while (!_Serial->available()) {
		delay(1);
	}
	while (1) {
	  String req = _Serial->readStringUntil('\n');
	  //debug(req);		
	  if (req.indexOf(F("RDY")) != -1) {
			debug(F("Power ON"));
			pinMode(START_PIN, OUTPUT);
			digitalWrite(START_PIN, HIGH);
			delay(1000);
			digitalWrite(START_PIN, LOW);
			delay(1000);
		}
		if (req.indexOf(F("POWERED DOWN")) != -1)	{
			debug(F("Power OFF"));
			return true;
		}
	}
	return false;
}

bool UC20::setAutoReset(uint8_t mode, uint16_t delay) {
	_Serial->print(F("AT+QRST="));
	_Serial->print(mode, DEC);
	_Serial->print(F(","));
	_Serial->print(delay, DEC);
	_Serial->println(F(""));
	return wait_ok(1000);
}

bool UC20::setAutoReset() {
	return setAutoReset(RESET_ONETIME, 0); // Will reset modem once
}

uint16_t UC20::getAutoReset() {
	
}

bool UC20::waitSIMReady() {
	bool timedOut = false;
	start_time_out();
	while (!timedOut) {
		String req = getSIMStatus();
		debug(req);
		if (req.indexOf(F("READY")) != -1) {
			debug(F("\r\nPIN Ready..."));
			return true;
		} else if (req.indexOf(F("SIM PIN")) != -1) {
			debug(F("\r\nPIN required..."));
			return false;
		} else if (req.indexOf(F("SIM PUK")) != -1) {
			debug(F("\r\nPUK required..."));
			return false;
		}	else {
			// Wait
			delay(1000);
		}
		if (time_out(10000)) {
			timedOut = true;
			debug(F("\r\nSIM No response--> Please check Hardware"));
		}
	}
	return false;
}

String UC20::getSIMStatus() {
	bool timedOut = false;
	_Serial->println(F("AT+CPIN?"));
	unsigned long startTime = millis();
	while (!timedOut) {
		if (_Serial->available()) {
			String req = _Serial->readStringUntil('\n');
			
			if (req.indexOf(F("+CPIN")) != -1) {
				debug(req);
				String ret = req.substring(req.indexOf(F("\"")));
				debug(ret);
				wait_ok(1000);
				return req;
			} else if (req.indexOf(F("+CME ERROR")) != -1) {
				return req;
			} else {
			}
		}
		if (millis() - startTime > (5000)) {
			timedOut = true;
		}
	}
	return F("");
}

String UC20::getIMEI() {
	_Serial->println(F("AT+GSN"));
	delay(300);
	while (1) {
		if (_Serial->available()) {
			String req = _Serial->readStringUntil('\n');
			if (req.length() > 3) {
				wait_ok_ndb(1000); // Flush OK
				return req.substring(0, req.indexOf(F("\r")));
			} else {
				// Ignore empty lines and OKs
			}
		}
	}
	
	return F("");
}

bool UC20::waitOK() {
	_Serial->println(F("AT"));
	return wait_ok(1000);
}
	
bool UC20::waitReady() {
	bool timedOut = false;
	start_time_out();
	while (!timedOut) {
		while (_Serial->available())	{
			String req = _Serial->readStringUntil('\n');	
			debug(req);
			if (req.indexOf(F("RDY")) != -1) {
				debug(F("\r\nUC20 Ready..."));
				debug(F("\r\nSet Echo OFF"));
				setEchoMode(false);
				
				return true;
			}	else if (req.indexOf(F("POWERED DOWN")) != -1)	{
				digitalWrite(START_PIN, HIGH);
				delay(1000);
				digitalWrite(START_PIN, LOW);
				delay(1000);
				powerOn();
			}	else {
				// Wait
			}
			
		}
		if (time_out(30000)) {
			timedOut = true;
			debug(F("\r\nBOOT No response--> Please check Hardware"));
		}
	}
	return false;
}

bool UC20::setEchoMode(bool echo) {
	if (echo) {
		_Serial->println(F("ATE1"));
		return wait_ok(300);
	} else {
		_Serial->println(F("ATE0"));
		return wait_ok(300);
	}
}

String UC20::moduleInfo() {
	_Serial->println(F("ATI"));
	delay(300);
	while (_Serial->available())	{
		String req = _Serial->readStringUntil('\n');
		debug(req);
		if (req.indexOf(F("Revision")) != -1) {
			return req.substring(req.indexOf(F(" "))+1, req.indexOf(F("\r")));
		}
		//if (wait_ok(1000)) {
		//	return F("");
		//}
	}
	return F("");
}

bool UC20::waitNetworkRegistered() {
	bool timedOut = false;
	int status = 0;
	unsigned long startTime = millis();
	while (!timedOut) {
		status = getNetworkStatus();
		if (status == 1) {
			debug(F("\r\nNetwork registered"));
			return true;
		}	else {
			// Wait
			delay(1000);
		}
		if (millis() - startTime > (180000)) {
			timedOut = true;
			debug(F("\r\nTimeout! Network not registered, check SIM/Signal"));
		}
	}
	return false;
}

unsigned char UC20::getNetworkStatus() {
	unsigned char ret = 0;	
	_Serial->println(F("AT+CREG?"));
	start_time_out();
	while (1) {
		String req = _Serial->readStringUntil('\n');	 
		if (req.indexOf(F("+CREG")) != -1) {
			wait_ok(1000);
			return (req.substring(req.indexOf(F(",")) + 1).toInt());
		} else {
			//return ret;
		}
		if (time_out(1000)) {
			return ret;
		}
	}
	return ret;
}

String UC20::getOperator() {
	_Serial->println(F("AT+COPS?"));
	//while (!_Serial->available()) {}
	start_time_out();
	while (1) {
		String req = _Serial->readStringUntil('\n');	 
		if (req.indexOf(F("+COPS")) != -1) {
			//+COPS: 0,0,"TRUE-H",2
			/*char comma1 = req.indexOf(F(","));
			char comma2 = req.indexOf(F(","),comma1+1);
			char comma3 = req.indexOf(F(","),comma2+1);
			String  operate_name = req.substring(comma2+2,comma3-1);
			String  acc_tech = req.substring(comma3+1);
			return(operate_name + "," + acc_tech);
			*/
			wait_ok(1000);
			return (req.substring(req.indexOf(F("\"")), req.indexOf(F("\r"))));
		}
		if (time_out(180000)) {
			return(F(""));
		}
		
	}
	return(F(""));
}

unsigned char UC20::signalQuality() {
	unsigned char ret = 101;
	_Serial->println(F("AT+CSQ"));
	delay(300);
	//while (!_Serial->available()) {}
	start_time_out();
	while (1)	{
		String req = _Serial->readStringUntil('\n');	 
		if (req.indexOf(F("+CSQ")) != -1) {
			ret = req.substring(req.indexOf(F(" "))+1,req.indexOf(F(","))).toInt();
			wait_ok(300);
			return ret;
		}
		if (time_out(3000)) {
			return ret;
		}
		
	}
	return ret;
}

int UC20::signalQualitydBm(unsigned char rssi) {
	
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

unsigned char UC20::signalQualityPercentage(unsigned char rssi) {
	if (rssi == 0) return 0;
	if (rssi == 99) return 0;
	
	if ((rssi >= 1) && (rssi <= 31)) return (((float(rssi)*100)/31) + 0.5); // Round nearest integer
	if (rssi > 31) return 100;
	
	return 0;
}

bool UC20::resetDefaults() {
	_Serial->println(F("AT&F"));
	return wait_ok(1000);
}

String UC20::getNetworkTimeString() {
	_Serial->println(F("AT+QLTS"));
	start_time_out();
	while (1) {
		String req = _Serial->readStringUntil('\n');	 
		if (req.indexOf(F("+QLTS")) != -1) {
			wait_ok(1000);
			return (req.substring(req.indexOf(F("\""))+1, req.lastIndexOf(F("\""))));
		}
		if (time_out(5000)) {
			return(F(""));
		}
		
	}
	return(F(""));
}

static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

time_t UC20::getNetworkTimeNumber() {
	// Excludes timezone, get it using getNetworkTimezone
	String strTime = getNetworkTimeString();
	
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
	
	int year = strYear.toInt() + 2000 - 1970; // String does not include century
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
	seconds+= (day-1) * SECS_PER_DAY;
	seconds+= hour * SECS_PER_HOUR;
	seconds+= minute * SECS_PER_MIN;
	seconds+= second;
	
	return (time_t)seconds;
}

int UC20::getNetworkTimezone() {
	String strTimezone = "";
	String strTime = getNetworkTimeString();
	
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

void UC20::start_time_out() {
	previousMillis_timeout = millis();
}

bool UC20::time_out(long timeout_interval) {
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis_timeout >= timeout_interval) {
		previousMillis_timeout = currentMillis;
		return true;
	}
	return false;
}

bool UC20::wait_ok(long time) {
	return wait_ok_(time,true);
}

bool UC20::wait_ok_ndb(long time) {
	return wait_ok_(time,false);
}
	
bool UC20::wait_ok_(long time, bool ack) {
	unsigned long previousMillis = millis(); 
	while (1)	{
		String req = _Serial->readStringUntil('\n');
		if (req.indexOf(F("OK")) != -1) {
			if (ack) {
				debug(F("OK"));
			}
			return (1);
		}
		if (req.indexOf(F("ERROR")) != -1) {
			if(ack)
			debug(F("Error"));
			return (0);
		}
		//debug(req);	
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= time) {
			previousMillis = currentMillis;
			if (ack) {
				debug(F("Error"));
			}
			return (0);
		}			

	}
	
}

unsigned char UC20::eventInput() {
	while (_Serial->available()) {
		String req = _Serial->readStringUntil('\n');
		req.replace("\r", "");
		req.replace("\n", "");
		//Serial.println(req);
		if (req.indexOf(F("RING")) != -1) {
			eventType = EVENT_RING;
			return (eventType);
		} else if (req.indexOf(F("+CMTI: \"SM\"")) != -1) {
			eventType = EVENT_SMS;
			indexNewSMS = req.substring(req.indexOf(F(","))+1).toInt();
			return (eventType);
		} else if (req.indexOf(F("+CMTI: \"ME\"")) != -1) {
			eventType = EVENT_SMS;
			indexNewSMS = req.substring(req.indexOf(F(","))+1).toInt();
			return (eventType);
		} else if (req.indexOf(F("+QSMTPPUT")) != -1) {
			char index1 = req.indexOf(F(":"));
			char index2 = req.indexOf(F(","));
			if (index2 == -1) index2 = sizeof(req);
			emailErr = req.substring(index1+2, index2).toInt();
			eventType = EVENT_EMAIL;
			return (eventType);
		} else if (req.indexOf(F("+QSSLURC")) != -1) {
			eventType = EVENT_SSLURC;
			Serial.println(req);
			return (eventType);
		} else if (req.indexOf(F("+CUSD")) != -1) {
			eventType = EVENT_USD;
			Serial.println(req);
			return (eventType);
		} else if (req.indexOf(F("OK")) == 0) {
			eventType = EVENT_OK;
			return (eventType);
			// Flush
		} else if (req.indexOf(F("ERROR")) != -1) {
			eventType = EVENT_ERROR;
			strError = req;
			return (eventType);
		} else {
			if (req.length() > 0) {
				debug(req);
				Serial.print(F("Unknown event type: "));
				Serial.println(req);
			} else {
				// read timeout
			}
		}
	}
	eventType = EVENT_NULL;
	return (eventType); // EVENT_NULL
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
