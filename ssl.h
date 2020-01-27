#ifndef SSL_h
#define SSL_h

#include "NSB_UC20.h"

#define SSL3_0	0
#define TLS1_0	1
#define TLS1_1	2
#define TLS1_2	3

#define CIPHER_AES_256_CBC_SHA		"0x0035"
#define CIPHER_AES_128_CBC_SHA		"0x002F"
#define CIPHER_RC4_128_SHA				"0x0005"
#define CIPHER_RC4_128_MD5				"0x0004"
#define CIPHER_3DES_EDE_CBC_SHA		"0x000A"
#define CIPHER_AES_256_CBC_SHA256	"0x003D"
#define CIPHER_ALL								"0xFFFF"

#define NO_AUTH							0
#define SERVER_AUTH					1
#define SERVER_CLIENT_AUTH	2

#define CARE_TIME_CHECK			0
#define IGNORE_TIME_CHECK		1


class SSL {
public:
	SSL();
	bool setContextID(unsigned char contextid);
	bool setSSLversion(unsigned char contextid, unsigned char sslver);
	bool setCiphersuite(unsigned char contextid, String cipher);
	bool setSeclevel(unsigned char contextid, unsigned char level);
	bool setCertificate(unsigned char contextid, String cacertpath);
	bool setCertificate(unsigned char contextid);
	bool setIgnorelocaltime(unsigned char contextid, bool ignoretime);
	int open(unsigned char pdpid, unsigned char contextid, unsigned char clientid, String serverid, String port, unsigned char acc_mode);
	bool close(unsigned char contextid);
	bool startSend(unsigned char clientid);
	bool startSend(unsigned char clientid, int len);
	bool startSend();
	bool stopSend();
	bool waitSendFinish();
	bool receiveAvailable();
	bool waitRead(long time);
	bool state();
	int read(unsigned char contextid);
	int  readBuffer();
	int  readBuffer(unsigned char contextid, int max_len);
	int  readBuffer(int max_len);
	
	int clear_buf(unsigned char contextid);
	void write(char data);
	void print(String data);
	void println(String data);
	void print(int data);
	void println(int data);
	
	unsigned char receiveClientID;
};

#endif