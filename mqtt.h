#ifndef MQTT_h
#define MQTT_h

#include "NSB_UC20.h"

#define CTRL_Z	26

#define MQTT_MAX_BUF			255
#define MQTT_VERSION_3_1		3
#define MQTT_VERSION_3_1_1	4

#define MQTT_NORMAL_TCP		0
#define MQTT_SSL_TCP			1

#define MQTT_STORE_SESSION	0
#define MQTT_CLEAN_SESSION	1

#define MQTT_TIMEOUT_IGNORE	0
#define MQTT_TIMEOUT_REPORT	1

#define MQTT_WILL_FLAG_IGNORE		0
#define MQTT_WILL_FLAG_REQUIRE	1

#define MQTT_QOS_0		0
#define MQTT_QOS_1		1
#define MQTT_QOS_2		2

#define MQTT_WILL_NO_RETAIN		0
#define MQTT_WILL_RETAIN			1

#define MQTT_WILL_QOS_0		0
#define MQTT_WILL_QOS_1		1
#define MQTT_WILL_QOS_2		2

#define MQTT_RECV_CONTAIN			0
#define MQTT_RECV_NO_CONTAIN	1

#define MQTT_OPEN_TIMEOUT_MS	120000
#define MQTT_CLOSE_TIMEOUT_MS	60000

#define MQTT_INITIAL				1
#define MQTT_CONNECTING			2
#define MQTT_CONNECTED			3
#define MQTT_DISCONNECTING	4

class MQTT {
public:
	MQTT();
	void setID(uint8_t tcpConnectID = 0);
	bool configVersion(uint8_t version = MQTT_VERSION_3_1_1);
	bool configPDP(uint8_t cid = 1);
	bool configSSL(bool sslEnable = MQTT_NORMAL_TCP, uint8_t sslctxidx = 1);
	bool configKeepAlive(uint16_t keepAliveTime = 120);
	bool configSession(bool cleanSession = MQTT_CLEAN_SESSION);
	bool configTimeout(uint8_t pktTimeout = 5, uint8_t retryTimes = 3, bool timeoutNotice = MQTT_TIMEOUT_IGNORE);
	bool configWill(char *willTopic, char *willMsg, bool willFlag = MQTT_WILL_FLAG_IGNORE, uint8_t willQoS = MQTT_WILL_QOS_0, uint8_t willRetain = MQTT_WILL_NO_RETAIN);
	bool configRecvMode(bool msgRecvMode = MQTT_RECV_CONTAIN);
	
	int8_t open(char *hostName, uint16_t port);
	bool close();
	uint8_t getState();
	int8_t connectClient(char *clientID, char *username, char *password);
	bool disconnectClient();
	bool connected();
	bool subscribe(char *topic, uint16_t msgID = 1, uint8_t qos = MQTT_QOS_0);
	bool unsubscribe(char *topic, uint16_t msgID = 1);
	bool publish(char *topic, char *msg, uint16_t msgID = 0, uint8_t qos = MQTT_QOS_0, uint8_t retain = MQTT_WILL_NO_RETAIN);
	
	bool readRecvPacket(char *topic, uint16_t topicLen, char *msg, uint16_t msgLen);
	
	const __FlashStringHelper* getOpenResultString(int8_t result);
	const __FlashStringHelper* getConnectResultString(int8_t result);
	const __FlashStringHelper* getURCString(char *urcResponse);
private:
	uint8_t _tcpConnectID = 0;
	uint8_t _pktTimeout = 5;
	uint8_t _retryTimes = 3;
};

#endif