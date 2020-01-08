#ifndef SSL_h
#define SSL_h

#include "NSB_UC20.h"

class SSL {
public:
	SSL();
	bool setContextID(unsigned char contextid);
	bool setSSLversion(unsigned char contextid, unsigned char sslver);
	bool setCiphersuite(unsigned char contextid, String tls_rsa);
	bool setSeclevel(unsigned char contextid, unsigned char level);
	bool setCertificate(unsigned char contextid);
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
	
	unsigned char ReceiveConnectID;
};

#endif