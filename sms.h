#ifndef UC20_SMS
#define UC20_SMS
#include "NSB_UC20.h"

#define SMS_MODE_PDU	0
#define SMS_MODE_TXT	1

#define SMS_MEM1			1
#define SMS_MEM2			2
#define SMS_MEM3			3

const char SMS_CHARSET_GSM[] PROGMEM = "GSM";
const char SMS_CHARSET_IRA[] PROGMEM = "IRA";
const char SMS_CHARSET_UCS2[] PROGMEM = "UCS2";

const char SMS_URC_USBAT[] PROGMEM = "usbat";
const char SMS_URC_USBMOD[] PROGMEM = "usbmodem";
const char SMS_URC_UART1[] PROGMEM = "uart1";

const char SMS_STORE_SM[] PROGMEM = "SM";
const char SMS_STORE_ME[] PROGMEM = "ME";
const char SMS_STORE_MT[] PROGMEM = "MT";

class SMS
{
public:
	SMS();
	bool defaultSettings();
	bool setFormat(uint8_t mode);
	bool setCharSet(const char * chset);
	bool configURC(const char *urcPortValue);
	bool setMsgStorage(const char *mem1, const char *mem2, const char *mem3);
	bool showTextModeParam(bool show = false);
	uint16_t getStorageUsed(uint8_t mem);
	uint16_t getStorageTotal(uint8_t mem);
	bool sendUSSD(char *ussdMsg, char *ussdBuff, uint16_t maxLen);
	unsigned char indexNewSMS();
	bool sendSMS(char *smsAddr, char *smsMsg);
	uint16_t readSMS(uint8_t index, char *smsbuff, uint16_t maxLen);
	bool getSMSSender(uint8_t index, char *sender, int senderLen);
	bool getSMSDate(uint8_t index, char *date, int dateLen);
	bool getSMSTime(uint8_t index, char *date, int dateLen);
	bool deleteSMS(uint8_t index);
	bool deleteAllSMS();
	int8_t getNumSMS(void);
	int8_t getNextSMSidx(void);
};


#endif