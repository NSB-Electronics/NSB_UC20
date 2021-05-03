// Based on TEE_UG20 by ThaiEasyElec
// https://github.com/ThaiEasyElec/TEE_UC20_Shield

#ifndef UC20_h
#define UC20_h

#include "includes/UC20_AT_Definitions.h"

#define DEBUG 0
#define DebugStream		Serial

#if DEBUG == 1
#define DEBUG_PRINT(...)		DebugStream.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...)	DebugStream.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)		
#define DEBUG_PRINTLN(...)	
#endif

#define ATLSOFTSERIAL 0
#define SOFTSERIAL 0

#include <Arduino.h>
#include <Stream.h>
#include <Time.h>

#if SOFTSERIAL 
	#include <SoftwareSerial.h>
#endif
#if ATLSOFTSERIAL 
	#include "AltSoftSerial.h"
#endif

#define prog_char  							char PROGMEM
#define prog_char_strstr(a, b)	strstr_P((a), (b))
#define prog_char_strlen(a)			strlen_P((a))

#define MAX_REPLY_LEN		255
#define TIMEOUT_MIN_MS	100
#define TIMEOUT_MAX_MS	150000
#define TIMEOUT_SIM_MS	15000
#define TIMEOUT_DEF_MS	1000
#define TIMEOUT_URL			60

#define EVENT_NONE			0
#define EVENT_OK				1
#define EVENT_ERROR 		2
#define EVENT_RING			3
#define EVENT_SMS				4
#define EVENT_EMAIL			5
#define EVENT_SSLURC		6
#define EVENT_USSD		  7
#define EVENT_SIM_ERR		8
#define EVENT_URC_DEACT	9
#define EVENT_MQTT_RECV	10
#define EVENT_MQTT_STAT	11


#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_WEEK * 52UL))
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k
#define LEAP_YEAR(Y)  (((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400)))

class UC20 
{
public:
	UC20();
	
	void setWatchdog(void (*watchdogCallback)(void));
	
	#if SOFTSERIAL 
	void begin(SoftwareSerial *serial,long baud);	
	#endif
	void begin(HardwareSerial *serial,long baud);
	#if ATLSOFTSERIAL 
	void begin(AltSoftSerial *serial,long baud);
	#endif
	void setPowerKeyPin(int pin);
	bool powerOn(int pwrKeyPin);
	bool powerOn();
	bool powerOff(boolean usePwrKey,int pwrKeyPin);
	bool powerOff(boolean usePwrKey);
	bool powerOff();
	bool hardwareReset(int rstPin);
	bool setAutoReset(uint8_t mode = RESET_ONETIME, uint32_t delay = 0);
	uint32_t getAutoReset();
	bool waitSIMReady();
	bool getSIMStatus(char *response, uint8_t responseLen = 20);
	bool getIMEI(char *response, uint8_t responseLen = 20);
	bool getIMSI(char *response, uint8_t responseLen = 20);
	bool getICCID(char *response, uint8_t responseLen = 20);
	bool waitOK(uint32_t delay = TIMEOUT_DEF_MS);
	bool waitReady();
	bool setEchoMode(bool echo);
	bool moduleInfo(char *response, uint8_t responseLen);
	bool waitNetworkRegistered();
	uint8_t getNetworkStatus();
	bool getOperator(char *response, uint8_t responseLen = 50);
	uint8_t signalQuality();
	int signalQualitydBm(uint8_t rssi);
	uint8_t signalQualityPercentage(uint8_t rssi);
	bool resetDefaults();
	
	bool getNetworkTime(char *response, uint8_t responseLen);
	time_t networkTimeUTC(char * strTime);
	int networkTimezone(char * strTime);
	
	bool getBattVoltage(uint16_t *v, uint16_t *p=0);
	
	unsigned char checkURC();
	unsigned char eventType;
	
	uint16_t indexNewSMS = 0;
	uint16_t emailErr = 0;
	
	int  peek();
	virtual size_t write(uint8_t byte);
	int read();
  int available();
  void flush();
	void flushInput();
	void print(unsigned char data,int type);
	void print(int data,int type);
	void print(unsigned int data,int type);
	void print(long data,int type);
	size_t print(String data);
	size_t println (String data);
	size_t print(String data,int type);
	size_t println (String data,int type);
	String readStringUntil(char data);
	
	void send(const char *send);
	bool sendCheckReply(const char *send, const char *reply, uint32_t timeout = TIMEOUT_DEF_MS);
	uint8_t sendGetResponse(const char *send, char *response, uint8_t responseLen, uint32_t timeout = TIMEOUT_DEF_MS);
	bool checkReply(const char *reply, uint32_t timeout = TIMEOUT_DEF_MS);
	uint8_t getReply(const char *send, uint32_t timeout = TIMEOUT_DEF_MS);
	uint8_t waitReply(const char *reply, uint32_t timeout = TIMEOUT_DEF_MS);
	
	bool sendParseReply(const char *send, const char *reply, uint16_t *value, char divider = ',', uint8_t index = 0);
	bool sendParseReply(const char *send, const char *reply, uint32_t *value, char divider = ',', uint8_t index = 0);
	bool sendParseReply(const char *send, const char *reply, char *value, uint16_t maxLen, char divider = ',', uint8_t index = 0);
	
	bool parseReply(const char *reply, int16_t *value, char divider = ',', uint8_t index = 0);
	bool parseReply(const char *reply, uint16_t *value, char divider = ',', uint8_t index = 0);
	bool parseReply(const char *reply, uint32_t *value, char divider = ',', uint8_t index = 0);
	bool parseReply(const char *reply, char *value, uint16_t maxLen, char divider = ',', uint8_t index = 0);
	
	bool parseReplyQuoted(const char *toreply, char *v, uint16_t maxLen, char divider, uint8_t index);
	
	const __FlashStringHelper* getCMEerrorString(char *errResponse);
	const __FlashStringHelper* getCMSerrorString(char *errResponse);
	
	char replyBuffer[MAX_REPLY_LEN];
	
	uint16_t readRaw(uint16_t b = MAX_REPLY_LEN);
	uint8_t readLine(uint32_t timeout = TIMEOUT_DEF_MS, bool multiline = false, uint8_t numLines = 1);
	
	void resetWatchdog();
protected:
	Stream *_Serial;
  void (*_watchdogCallback)(void);
};
extern UC20 gsm;

#endif


