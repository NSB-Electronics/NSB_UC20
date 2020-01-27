// Based on TEE_UG20 by ThaiEasyElec
// https://github.com/ThaiEasyElec/TEE_UC20_Shield

#ifndef UC20_h
#define UC20_h

#define DEBUG 1

#define ATLSOFTSERIAL 0
#define SOFTSERIAL 0

#include <Arduino.h>
#include <Stream.h>
#if SOFTSERIAL 
	#include <SoftwareSerial.h>
#endif
#if ATLSOFTSERIAL 
	#include "AltSoftSerial.h"
#endif

#define EVENT_NULL	0
#define EVENT_RING	1
#define EVENT_SMS		2
#define UFS "UFS"
#define RAM "RAM"
#define COM "COM"

class UC20 
{
public:
	UC20();
	#if SOFTSERIAL 
	void begin(SoftwareSerial *serial,long baud);	
	#endif
	void begin(HardwareSerial *serial,long baud);
	#if ATLSOFTSERIAL 
	void begin(AltSoftSerial *serial,long baud);
	#endif
	void (*Event_debug)(String data);
	void debug(String data);
	void setPowerKeyPin(int pin);
	bool powerOn();
	bool powerOff();
	bool setAutoReset(uint8_t mode, uint16_t delay);
	uint16_t getAutoReset();
	bool waitSIMReady();
	String getSIMStatus();
	bool waitOK();
	bool waitReady();
	bool setEchoMode(bool echo);
	String moduleInfo();
	bool waitNetworkRegistered();
	unsigned char getNetworkStatus();
	String getOperator();
	unsigned char signalQuality();
	int signalQualitydBm(unsigned char rssi);
	unsigned char signalQualityPercentage(unsigned char rssi);
	bool resetDefaults();
	bool wait_ok_(long time,bool db);
	bool wait_ok(long time);
	bool wait_ok_ndb(long time);
	unsigned char eventInput();
	unsigned char eventType;
	
	
	unsigned char indexNewSMS;
	void start_time_out();
	bool time_out(long timeout_interval);
	
	int  peek();
	virtual size_t write(uint8_t byte);
	int read();
    int available();
    virtual void flush();
	void print(unsigned char data,int type);
	void print(int data,int type);
	void print(unsigned int data,int type);
	void print(long data,int type);
	size_t print(String data);
	size_t println (String data);
	size_t print(String data,int type);
	size_t println (String data,int type);
	String readStringUntil(char data);
protected:
	 Stream *_Serial;
};
extern UC20 gsm;

#endif


