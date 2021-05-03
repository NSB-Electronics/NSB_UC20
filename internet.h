#ifndef INTERNET_h
#define INTERNET_h

#include "NSB_UC20.h"

#define AUTH_NONE	0
#define AUTH_PAP	1
#define AUTH_CHAP	2

class INTERNET
{
public:
	INTERNET();
	bool configure(uint8_t contextid, uint8_t context_type, char *apn, char *user, char *password, uint8_t auth = AUTH_PAP);
	bool configure(uint8_t contextid, char *apn, char *user, char *password);
	bool configure(char *apn, char *user, char *password);
	bool configDNS(uint8_t contextid, char *pridnsaddr, char *secdnsaddr);
	bool configDNS(char *pridnsaddr, char *secdnsaddr);
	bool configDNS(uint8_t contextid);
	bool configDNS();
	bool connect(uint8_t contextid);
	bool connect();
	bool disconnect(uint8_t contextid);
	bool disconnect();
	bool getIP(char *ip, uint8_t ipLen);

};


















#endif 