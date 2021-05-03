#ifndef SMTP_h
#define SMTP_h

#include "NSB_UC20.h"

#define SMTP_NO_SSL		0
#define SMTP_SSL 			1
#define SMTP_STARTTLS	2

#define SMTP_DELETE		0
#define SMTP_ADD			1

#define SMTP_TO				1
#define SMTP_CC				2
#define SMTP_BCC			3

#define SMTP_ASCII		0
#define SMTP_UTF_8		1
#define SMTP_GB2312		2
#define SMTP_BIG5			3

class SMTP {
public:
	SMTP();
	bool setSSLtype(uint8_t sslType = SMTP_NO_SSL);
	bool setContextID(uint8_t contextid = 1);
	bool setSSLctxID(uint8_t sslctxid = 1);
	bool configServer(char *srvaddr, uint16_t srvport);
	bool setAccount(char *username, char *password);
	bool setSender(char *senderName, char *senderEmail);
	bool addRecipient(char *emailaddr, uint8_t type = SMTP_TO);
	bool deleteRecipients();
	bool setSubject(char *subject, uint8_t charset = SMTP_ASCII);
	bool setBody(char *body, uint8_t charset = SMTP_ASCII, uint16_t inputTime = 90);
	bool addAttachment(char *filename, uint8_t fileIndex = 1);
	bool clearContent();
	bool sendEmail(uint16_t timeout = 300);
};

#endif