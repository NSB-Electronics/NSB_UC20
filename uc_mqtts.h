#ifndef uc_mqtts_h
#define uc_mqtts_h
#include "NSB_UC20.h"

#define MQTT_MAX_PACKET_SIZE 128
class UCxMQTTS
{
	private:
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint8_t writeSSL(uint8_t* buf, uint8_t length); 
	int readDataFrom3GBufferMode();
	unsigned int readDataInBufferMode(unsigned int buf_len);
	void checkRXsub();
	
	
	public:
	UCxMQTTS();
	bool connectMQTTServer(String web, String port);
	bool disconnectMQTTServer();
	bool connectState();
	unsigned char connectMQTTUser(String id, String user, String pass);
	String connectCodeString(unsigned char input);
	void publish(char *topic ,int lentopic, char *payload, int lenpay);
	void publish(String topic, String payload);
	void subscribe(char *topic, int topiclen);
	void subscribe(String topic);
	void clearBuffer();
	void ping();
	void mqttLoop();
	void (*callback)(String topic, char *playload, unsigned char length);
	
	protected:	
	bool connected = false;
	unsigned int len_buffer_in_module = 0;

	const long interval_ping = 10000; 
	unsigned long previousMillis_ping = 0; 
	unsigned long currentMillis_ping; 
	
};	
#endif