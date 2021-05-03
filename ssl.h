#ifndef SSL_h
#define SSL_h

#include "NSB_UC20.h"

#define SSL3_0	0
#define TLS1_0	1
#define TLS1_1	2
#define TLS1_2	3

#define CIPHER_AES_256_CBC_SHA		(char*)"0x0035"
#define CIPHER_AES_128_CBC_SHA		(char*)"0x002F"
#define CIPHER_RC4_128_SHA				(char*)"0x0005"
#define CIPHER_RC4_128_MD5				(char*)"0x0004"
#define CIPHER_3DES_EDE_CBC_SHA		(char*)"0x000A"
#define CIPHER_AES_256_CBC_SHA256	(char*)"0x003D"
#define CIPHER_ALL								(char*)"0xFFFF"

#define NO_AUTH								0
#define SERVER_AUTH						1
#define SERVER_CLIENT_AUTH		2

#define CARE_TIME_CHECK				0
#define IGNORE_TIME_CHECK			1

#define SSL_BUFFER_MODE				0
#define SSL_DIRECT_MODE				1
#define SSL_TRANSP_MODE				2

#define SSL_UFS_CERT					(char*)"UFS:cacert.pem"

class SSL {
public:
	SSL();
	bool setSSLversion(uint8_t contextid, uint8_t sslver);
	bool setCiphersuite(uint8_t contextid, char *cipher);
	bool setSeclevel(uint8_t contextid, uint8_t level);
	bool setCertificate(uint8_t contextid, char *cacertpath = SSL_UFS_CERT);
	bool setIgnorelocaltime(uint8_t contextid, bool ignoretime);
	uint16_t open(uint8_t pdpid, uint8_t contextid, uint8_t clientid, char *serverAddr, uint16_t port, uint8_t accessMode);
	bool close(uint8_t clientid, uint16_t closeTimeout = 10);
	bool send(uint8_t clientid, uint8_t *data, uint16_t sendLen = 0);
	bool receiveAvailable();
	int16_t state(uint8_t clientid = 0);
	uint16_t readBuffer(uint8_t clientid, uint16_t readLen);
	uint16_t readBuffer(uint16_t readLen);
	uint16_t readBuffer();
	
	uint16_t receiveClientID;
};

#endif