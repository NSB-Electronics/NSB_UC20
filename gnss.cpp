#include "gnss.h"


GNSS::GNSS(){}

bool GNSS::start() {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=1", CMD_GPS_OPERATE);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool GNSS::enableNMEA() {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=\"nmeasrc\",1", CMD_GPS_CONF);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}
bool GNSS::disableNMEA() {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=\"nmeasrc\",0", CMD_GPS_CONF);
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

bool GNSS::stop() {
	bool responseOK = false;
	
	responseOK = gsm.sendCheckReply(CMD_GPS_TERMINATE, REPLY_OK);
	
	return responseOK;
}

String GNSS::getNMEA(String nmea) {
	//AT+QGPSGNMEA=“GGA” 
	String data= "Please Wait...";
	gsm.print(F("AT+QGPSGNMEA=\""));
	gsm.print(nmea);
	gsm.println(F("\""));
	while (!gsm.available()) {
		
	}
	
	while(1) {
		String req = gsm.readStringUntil('\n');	
	    //DEBUG_PRINTLN(req);
		if (req.indexOf(F("+QGPSGNMEA:")) != -1) {
			//return(req);
			char index = req.indexOf(F("$"));
			data = req.substring(index);
		}
		if (req.indexOf(F("OK")) != -1) {
			return (data);
		}
	}			
	
}

uint16_t GNSS::getGPS(char *buffer, uint8_t maxBuff, uint8_t mode) {
	bool responseOK = false;
	char cmd[30];
	uint16_t errCode = 0;
	
	sprintf(cmd, "%s=%d", CMD_GPS_LOC, mode);
	responseOK = gsm.sendCheckReply(cmd, REPLY_GPS_LOC);
	
	if (responseOK) {
		strncpy(buffer, gsm.replyBuffer+sizeof(REPLY_GPS_LOC), maxBuff);
		responseOK = gsm.checkReply(REPLY_OK); // read OK
		errCode = 1;
	} else {
		responseOK = gsm.parseReply(REPLY_CME_ERROR, &errCode, ' ', 1);
	}
	return errCode;
}

uint16_t GNSS::getGPS(float *lat, float *lon, float *altitude, float *speed_kmh, float *heading, float *precision, uint8_t *nSat, uint8_t mode) {
	bool responseOK = false;
	char cmd[30];
	char gpsBuffer[120];
	uint16_t errCode = 0;
	
	sprintf(cmd, "%s=%d", CMD_GPS_LOC, mode);
	responseOK = gsm.sendCheckReply(cmd, REPLY_GPS_LOC);
	
	if (responseOK) {
		strncpy(gpsBuffer, gsm.replyBuffer, sizeof(gpsBuffer));
		
		uint8_t retLen = strlen(gpsBuffer);
		
		// make sure we have a response
		if (retLen == 0)
			return false;
		
		// Skip utc
		char *tok = strtok(gpsBuffer, ",");
			if (!tok) return false;
		
		// Grab latitude
		char *latp = strtok(NULL, ",");
			if (!latp) return false;
		*lat = atof(latp);
		
		// Grab longitude
			char *longp = strtok(NULL, ",");
			if (!longp) return false;
		*lon = atof(longp);
		
		// Grab accuracy
		char *accup = strtok(NULL, ",");
		if (!accup) return false;
		if (precision != NULL) {
			*precision = atof(accup);
		}
		
		// Grab altitude
		char *altp = strtok(NULL, ",");
		if (!altp) return false;
		if (altitude != NULL) {
			*altitude = atof(altp);
			}
		
		// Grab fix
			char *fixp = strtok(NULL, ",");
			if (!fixp) return false;
		
		// Grab heading
		char *cogp = strtok(NULL, ",");
		if (!cogp) return false;
		if (heading != NULL) {
			*heading = atof(cogp);
		}
		
		// Grab the speed in km/h
		char *spkmp = strtok(NULL, ",");
		if (!spkmp) return false;
		if (speed_kmh != NULL) {
			*speed_kmh = atof(spkmp);
			}
			
		// Grab spkn
			char *spknp = strtok(NULL, ",");
			if (!spknp) return false;
		
		// Grab date
		char *datep = strtok(NULL, ",");
			if (!datep) return false;
		
		// Grab nSat
		char *nsatp = strtok(NULL, ",");
		if (!nsatp) return false;
		if (nSat != NULL) {
			*nSat = atof(nsatp);
		}
		
		responseOK = gsm.checkReply(REPLY_OK); // read OK
		errCode = 1;
	} else {
		responseOK = gsm.parseReply(REPLY_CME_ERROR, &errCode, ' ', 1);
	}
	
	return errCode;
}

const __FlashStringHelper* GNSS::getGNSSErrorString(uint16_t errCode) {
  switch (errCode) {
		case   1: return F("GPS fixed");
		case 501: return F("Invalid parameter(s)");
		case 502: return F("Operation not supported");
		case 503: return F("GNSS subsystem busy");
		case 504: return F("Session is ongoing");
		case 505: return F("Session not activated");
		case 506: return F("Operation timeout");
		case 507: return F("Function not enabled");
		case 508: return F("Time information error");
		case 509: return F("XTRA not enabled");
		case 510: return F("XTRA file open failed");
		case 511: return F("Bad CRC for XTRA data file");
		case 512: return F("Validity time out of range");
		case 513: return F("Internal resource error");
		case 514: return F("GNSS locked");
		case 515: return F("End by E911");
		case 516: return F("Not fixed now");
		case 549: return F("Unknown error");
		default:  return F("Unknown error");
  }
}