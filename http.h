#ifndef HTTP_h
#define HTTP_h

#include "NSB_UC20.h"


class HTTP {
public:
	HTTP();
	bool begin(unsigned char contextID);
	int  get();
	int  post(String data);
	int  post();
	String read(unsigned char waitTime);
	String read();
	void ReadData();
	bool SaveResponseToMemory(String pattern, String Filename);
	bool url(String url);

};

#endif