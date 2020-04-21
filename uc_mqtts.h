#ifndef uc_mqtts_h
#define uc_mqtts_h
#include "NSB_UC20.h"

// MQTT 3.1.1
#define MQTT_PROTOCOL_LEVEL 	0x04

#define CONNECT_TIMEOUT_MS 6000
#define PUBLISH_TIMEOUT_MS 500
#define PING_TIMEOUT_MS    500
#define SUBACK_TIMEOUT_MS  500

// Keepalive (seconds), Default to 5 minutes
#define MQTT_CONN_KEEPALIVE 	300
#define PING_INTERVAL 				60000
#define MAX_PING_CNT					5

#define MQTT_CTRL_CONNECT     0x01
#define MQTT_CTRL_CONNECTACK  0x02
#define MQTT_CTRL_PUBLISH     0x03
#define MQTT_CTRL_PUBACK      0x04
#define MQTT_CTRL_PUBREC      0x05
#define MQTT_CTRL_PUBREL      0x06
#define MQTT_CTRL_PUBCOMP     0x07
#define MQTT_CTRL_SUBSCRIBE   0x08
#define MQTT_CTRL_SUBACK      0x09
#define MQTT_CTRL_UNSUBSCRIBE 0x0A
#define MQTT_CTRL_UNSUBACK    0x0B
#define MQTT_CTRL_PINGREQ     0x0C
#define MQTT_CTRL_PINGRESP    0x0D
#define MQTT_CTRL_DISCONNECT  0x0E

#define MQTT_QOS_1 0x01
#define MQTT_QOS_0 0x00

// Largest full packet we're able to send.
// Need to be able to store at least ~90 chars for a connect packet with full
// 23 char client ID.
#define MQTT_MAX_PACKET_SIZE 1500

#define MQTT_CONN_USERNAMEFLAG    0x80
#define MQTT_CONN_PASSWORDFLAG    0x40
#define MQTT_CONN_WILLRETAIN      0x20
#define MQTT_CONN_WILLQOS_1       0x08
#define MQTT_CONN_WILLQOS_2       0x18
#define MQTT_CONN_WILLFLAG        0x04
#define MQTT_CONN_CLEANSESSION    0x02

class UCxMQTTS
{
	private:
	const char *servername;
	uint16_t portnum;
	const char *clientid;
	const char *username;
	const char *password;
	const char *will_topic;
	const char *will_payload;
	uint8_t will_qos;
	uint8_t will_retain;
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint16_t writeSSL(uint8_t* buf, uint16_t length); 
	uint16_t readDataFrom3GBufferMode();
	uint16_t readDataInBufferMode(uint16_t buf_len);
	void checkRXsub();
	
	
	public:
	UCxMQTTS();
	bool connectMQTTServer(unsigned char pdpid, unsigned char contextid, unsigned char clientid, String web, String port, unsigned char mode);
	bool connectMQTTServer(unsigned char contextid, String web, String port);
	bool connectMQTTServer(String web, String port);
	bool disconnectMQTTServer(unsigned char contextid);
	bool disconnectMQTTServer();
	bool connectState();
	unsigned char connectMQTTUser(String id, String user, String pass);
	bool disconnectMQTTUser();
	String connectCodeString(unsigned char input);
	void publish(char *topic, uint16_t lentopic, char *payload, uint16_t lenpay, uint8_t qos);
	void publish(String topic, String payload, uint8_t qos);
	void publish(String topic, String payload);
	void subscribe(char *topic, uint16_t topiclen, uint8_t qos);
	void subscribe(String topic, uint8_t qos);
	void subscribe(String topic);
	void unsubscribe(char *topic, uint16_t topiclen);
	void unsubscribe(String topic);
	void clearBuffer();
	void ping();
	void mqttLoop();
	void (*callback)(String topic, char *playload, unsigned char length);
	
	protected:	
	bool connected = false;
	bool connectAck = false;
	uint16_t len_buffer_in_module = 0;
	uint16_t packet_id_counter;
	unsigned long previousMillis_ping = 0; 
	unsigned long currentMillis_ping; 
	uint8_t connectReturnCode = 0xFF;
	
};	
#endif