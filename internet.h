#ifndef INTERNET_h
#define INTERNET_h

#include "NSB_UC20.h"


class INTERNET
{
public:
	INTERNET();
	bool configure(unsigned char contextid, unsigned char context_type, String apn, String user, String password, unsigned char auth);
	bool configure(unsigned char contextid, String apn, String user, String password);
	bool configure(String apn, String user, String password);
	bool configDNS(unsigned char contextid, String pridnsaddr, String secdnsaddr);
	bool configDNS(String pridnsaddr, String secdnsaddr);
	bool configDNS(unsigned char contextid);
	bool configDNS();
	bool connect(unsigned char contextid);
	bool connect();
	bool disconnect(unsigned char contextid);
	bool disconnect();
	String getIP();

};


















#endif 