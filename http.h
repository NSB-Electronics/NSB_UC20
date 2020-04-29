#ifndef HTTP_h
#define HTTP_h

#include "NSB_UC20.h"


class HTTP {
public:
	HTTP();
	bool begin(unsigned char contextID);
	int  get(unsigned int rspTime);
	int  get();
	int  post(String data, unsigned int inputTime, unsigned int rspTime);
	int  post(String data);
	int  post();
	int  read(char *recvBuffer, unsigned int maxLen, unsigned int waitTime);
	int  read(char *recvBuffer, unsigned int maxLen);
	int  read(char *recvBuffer);
	void ReadData();
	bool SaveResponseToMemory(String pattern, String Filename);
	bool url(String url);

};

#endif