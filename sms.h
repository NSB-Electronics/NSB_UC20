#ifndef UC20_SMS
#define UC20_SMS
#include "NSB_UC20.h"


class SMS
{
public:
	SMS();
	void defaultSettings();
	void start(String rx_number);
	void send(String data);
	void sendln(String data);
	bool stop();
	unsigned char indexNewSMS();
	String readSMS(int index);
	bool sendUSSD(String USSDcode);
	String SMSInfo;
	String convertStrUnicodeToTIS620(String data);
	String convertStrUnicodeToUTF8(String data);
	bool deleteSMS(int index);
	bool deleteAllSMS();
	unsigned char checkSMSReceived(unsigned char* smsIdx);
	unsigned char maxSMS = 20;
};


#endif