#ifndef HTTP_h
#define HTTP_h

#include "NSB_UC20.h"


class HTTP {
public:
	HTTP();
	bool begin(uint8_t contextid);
	
	int  post(String data, uint16_t inputTime, uint16_t rspTime);
	int  post(String data);
	int  post();
	int16_t read(char *recvBuffer, uint16_t maxLen, uint16_t waitTime = 60);
	bool SaveResponseToMemory(String pattern, String Filename);
	bool setURL(char *url, uint16_t urlLen, uint16_t timeout = TIMEOUT_URL);
	bool get(uint16_t *err, uint16_t *httpRspCode, uint16_t *contentLen, uint16_t rspTime = TIMEOUT_URL);
	
	const __FlashStringHelper* getHTTPErrorString(uint16_t errorCode);
	const __FlashStringHelper* getHTTPResponseString(uint16_t responseCode);
};

#endif