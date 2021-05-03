#ifndef GNSS_h
#define GNSS_h

#include "NSB_UC20.h"

class GNSS
{
public:
	GNSS();
	bool start();
	bool stop();
	bool enableNMEA();
	bool disableNMEA();
	String getNMEA(String nmea);
	uint16_t getGPS(char *buffer, uint8_t maxBuff, uint8_t mode = 2);
	uint16_t getGPS(float *lat, float *lon, float *altitude = 0, float *speed_kmh = 0, float *heading = 0, float *precision = 0, uint8_t *nSat = 0, uint8_t mode = 2);
	
	const __FlashStringHelper* getGNSSErrorString(uint16_t errCode);
};

#endif