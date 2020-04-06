#ifndef SMTP_h
#define SMTP_h

#include "NSB_UC20.h"

#define SMTP_NO_SSL		0
#define SMTP_SSL 		1
#define SMTP_STARTTLS	2

#define SMTP_DELETE		0
#define SMTP_ADD		1

#define SMTP_TO			1
#define SMTP_CC			2
#define SMTP_BCC		3

#define SMTP_ASCII		0
#define SMTP_UTF-8		1
#define SMTP_GB2312		2
#define SMTP_BIG5		3

class SMTP {
public:
	SMTP();
	bool setSSLtype(unsigned char sslType);
	bool setSSLtype();
	bool setContextID(unsigned char contextid);
	bool setContextID();
	bool setSSLctxID(unsigned char sslctxid);
	bool setSSLctxID();
	bool configServer(String srvaddr, unsigned int srvport);
	bool setAccount(String username, String password);
	bool setSender(String senderName, String senderEmail);
	bool addRecipient(unsigned char type, String emailaddr);
	bool addRecipient(String emailaddr);
	bool deleteRecipients();
	bool setSubject(unsigned char charset, String subject);
	bool setSubject(String subject);
	
	bool startBody(unsigned char charset, unsigned int bodyLength, unsigned int inputTime);
	bool startBody(unsigned int bodyLength);
	bool startBody();
	void addBody(String data);
	void addBodyln(String data);
	int stopBody(unsigned int inputTime);
	int stopBody();
	
	bool addAttachment(unsigned char fileIndex, String filename);
	bool addAttachment(String filename);
	
	bool sendEmail(unsigned int timeout);
	bool sendEmail();
	bool clearContent();
	
};

#endif