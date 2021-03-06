#include "uc_mqtts.h"
#include "ssl.h"
SSL ssl_mqtts;

void func_null_mqtts(String topic,char *playload, unsigned char length){}
	
UCxMQTTS::UCxMQTTS() {
	//void (*callback)(String topic ,char *playload,unsigned char length);
	callback = func_null_mqtts;
}

bool UCxMQTTS::connectMQTTServer(uint8_t pdpid, uint8_t contextid, uint8_t clientid, char *serverAddr, uint16_t port, uint8_t accessMode) {
	uint16_t errorCode = ssl_mqtts.open(pdpid, contextid, clientid, serverAddr, port, accessMode); //buffer mode	
	if (errorCode == 0) {
		connected = true;
	} else {
		connected = false;
	}
	return (connected);
}

bool UCxMQTTS::connectMQTTServer(uint8_t contextid, char *serverAddr, uint16_t port) {
	connected = connectMQTTServer(1, contextid, 0, serverAddr, port, 0);
	return (connected);
}

bool UCxMQTTS::connectMQTTServer(char *serverAddr, uint16_t port) {
	connected = connectMQTTServer(1, 1, 0, serverAddr, port, 0);
	return (connected);
}

bool UCxMQTTS::disconnectMQTTServer(uint8_t clientid) {
	connected = false;
	return (ssl_mqtts.close(clientid));
}

bool UCxMQTTS::disconnectMQTTServer() {
	return (disconnectMQTTServer(0));
}

// The current MQTT spec is 3.1.1 and available here:
//   http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
unsigned char UCxMQTTS::connectMQTTUser(char *id, char *user, char *pass) {
	
	// Leave room in the buffer for header and variable length field
	uint32_t length = 0;		// Total length of the mqtt data (excludes fixed header)
	uint32_t varLength = 0;		// Stores the temp variable length in order to encode the data length
	uint32_t payloadLen = 0;	// Payload length
	uint8_t encodedByte; 	    // Variable length bytes can be up to 4 bytes long
	uint8_t variableHeaderLen = 10; // Always 10 for a CONNECT packet
	uint8_t connectFlags = MQTT_CONN_CLEANSESSION;	// Always clean the session
	
	payloadLen = strlen(id) + strlen(user) + strlen(pass);
	if (strlen(id) > 0) {
		payloadLen += 2;
	}
	if (strlen(user) > 0) {
		payloadLen += 2;
		connectFlags |= MQTT_CONN_USERNAMEFLAG;
	}
	if (strlen(pass) > 0) {
		payloadLen += 2;
		connectFlags |= MQTT_CONN_PASSWORDFLAG;
	}
	
	varLength = variableHeaderLen + payloadLen; // Variable length to encode
	
	buffer[0] = (MQTT_CTRL_CONNECT << 4); // Control packet type
	length++;
	
	// Remaining length larger than 127, need to split into 2 bytes or more depending on length
	// See MQTT spec section 2.2.3
	do {
		encodedByte = varLength % 128;
		varLength = varLength / 128;
		if (varLength > 0) {
			encodedByte = encodedByte | 128;
		}
		buffer[length] = encodedByte;
		length++;
	} while (varLength > 0);
	
	buffer[length+0] = 0x00;    // Protocol name Length MSB (Fix=0)
	buffer[length+1] = 0x04;    // Protocol name Length LSB (Fix=4)
	buffer[length+2] = 'M';     // Protocol name   (Fix=M)
	buffer[length+3] = 'Q';     // Protocol name   (Fix=Q)
	buffer[length+4] = 'T';     // Protocol name   (Fix=T)
	buffer[length+5] = 'T';     // Protocol name   (Fix=T)
	buffer[length+6] = MQTT_PROTOCOL_LEVEL;    // Protocol Level  (Fix=4)
	buffer[length+7] = connectFlags;    // Connect Flags (Bit7: user name flag), (Bit6: password flag), (Bit5: will retain), (Bit4-3: will Qos), (Bit2: will flag), (Bit1: clear session), (Bit0: fixed 0)
	buffer[length+8] = MQTT_CONN_KEEPALIVE >> 8;   // keep alive MSB
	buffer[length+9] = MQTT_CONN_KEEPALIVE & 0xFF; // keep alive LSB time ping 16 bit (seconds)
	
	length += 10;
	
	unsigned char len = strlen(id);
	buffer[length] = (len >> 8) & 0x00FF; // clientid length MSB
	buffer[length+1] = len & 0x00FF;      // clientid length LSB
	length += 2;
	
	for (byte k = 0; k < len; k++) {
		buffer[length] = *id++;
		length++;
		if (length >= MQTT_MAX_PACKET_SIZE) {
			DEBUG_PRINTLN(F("MQTT_MAX_PACKET_SIZE exceeded"));
			return (0xFF);
		}
	}

	len = strlen(user);
	buffer[length] = (len >> 8) & 0x00FF; // username length MSB
	length++;
	buffer[length] = len & 0x00FF;        // username length LSB
	length++;
	for (byte k = 0; k < len; k++) {
		buffer[length] = *user++;
		length++;
		if (length >= MQTT_MAX_PACKET_SIZE) {
			DEBUG_PRINTLN(F("MQTT_MAX_PACKET_SIZE exceeded"));
			return (0xFF);
		}
	}
	
	len = strlen(pass);
	if (len > 0) {
		buffer[length] = (len >> 8) & 0x00FF; // password length MSB
		length++;
		buffer[length] = len & 0x00FF;        // password length LSB
		length++;
		for (byte k = 0; k < len; k++) {
			buffer[length] = *pass++;
			length++;
			if (length >= MQTT_MAX_PACKET_SIZE) {
				DEBUG_PRINTLN(F("MQTT_MAX_PACKET_SIZE exceeded"));
				return (0xFF);
			}
		}
	}
	
	char tmp[30];
	snprintf(tmp, sizeof(tmp), "\t---> 0x%02X - Connect request", buffer[0]);
	DEBUG_PRINTLN(tmp);
	
	writeSSL(buffer, length);
	
	connectAck = false;
	packet_id_counter = 0;
	previousMillis_ping = millis();
	
	// Check for Connect Acknowledge message
	unsigned long previousMillis = millis();
	while (millis() - previousMillis < 10000) {
		previousMillis_ping = millis(); // Do not ping while trying to connect
		mqttLoop();
		if (connectAck == true) {
			//DEBUG_PRINTLN("Connect Ack!");
			break;
		}
	}
	previousMillis_ping = millis();
	
	if (connectAck == true) {
		//DEBUG_PRINT("Connect return: ");
		//SDEBUG_PRINTLN(connectReturnCode);
		if (connectReturnCode != 0x00) {
			connected = false;
		}
		return (connectReturnCode); // Contains the return code from server
	} else {
		connected = false;
		return (0xFF);
	}
}

bool UCxMQTTS::disconnectMQTTUser() {
	buffer[0] = MQTT_CTRL_DISCONNECT << 4;
	buffer[1] = 0;
	
	connected = false;
	
	char tmp[30];
	snprintf(tmp, sizeof(tmp), "\t---> 0x%02X - Disconnect request", buffer[0]);
	DEBUG_PRINTLN(tmp);
	
	writeSSL(buffer, 2);
	return true;
}

uint16_t UCxMQTTS::writeSSL(uint8_t *buf, uint16_t length) {
	// uint16_t len_let = 0;
	// if (ssl_mqtts.startSend(0, length)) {
		// for (uint16_t itcp = 0; itcp < length; itcp++) {
			// ssl_mqtts.write(buf[itcp]);
			// len_let++;
		// }
		// if(!ssl_mqtts.waitSendFinish()) {
			// //DEBUG_PRINTLN("false unfinish");
			// connected = false;
		// }
	// } else {
		// connected = false;
		// //DEBUG_PRINTLN("startSend ERROR");
	// }
  // return (len_let);
	
	bool sendOK = false;
	sendOK = ssl_mqtts.send(0, buf, length);
	if (sendOK != 0) {
		connected = false;
	}
	return sendOK;
}

const __FlashStringHelper* UCxMQTTS::connectCodeString(unsigned char input) {
	switch (input) {
		case 0:			return F("0x00 Connection Accepted");
		case 1:			return F("0x01 Connection Refused, unacceptable protocol version");
		case 2:			return F("0x02 Connection Refused, identifier rejected");
		case 3:			return F("0x03 Connection Refused, Server unavailable");
		case 4:			return F("0x04 Connection Refused, bad user name or password");
		case 5:			return F("0x05 Connection Refused, not authorized");
		case 6: 		return F("0x06 Exceeded reconnect rate limit. Please try again later."); // Adafruit IO?
		case 7: 		return F("0x07 You have been banned from connecting. Please contact the MQTT server administrator."); // Adafruit IO?
		case 0xFF: 		return F("0xFF TimeOut Occurred");
		default: 		return F("Unknown Code!");
	}
}

void UCxMQTTS::publish(char *topic, uint16_t lentopic, char *payload, uint16_t lenpay, uint8_t qos) {
	
	uint32_t varLength = 0;	// Stores the temp variable length in order to encode the data length
	uint8_t encodedByte; 	// Variable length bytes can be up to 4 bytes long
	uint16_t length = 0;
	
	// ToDO: Need to add QoS and retain?
	clearBuffer();
	
	varLength = 2 + lentopic + lenpay; // Variable length to encode
	if (qos > 0) {
		varLength += 2;
	}
	
	buffer[0] = MQTT_CTRL_PUBLISH << 4 | qos << 1;    // Control packet type (Fix=3x) ,x bit3 = DUP , xbit2-1 = QoS level , xbit0 = Retain
	length++;
	
	// Remaining length larger than 127, need to split into 2 bytes or more depending on length
	// See MQTT spec section 2.2.3
	do {
		encodedByte = varLength % 128;
		varLength = varLength / 128;
		if (varLength > 0) {
			encodedByte = encodedByte | 128;
		}
		buffer[length] = encodedByte;
		length++;
	} while (varLength > 0);

	uint16_t len = lentopic;
	buffer[length] = (len >> 8) & 0x00FF; //  topic length MSB
	buffer[length+1] = len & 0x00FF;      //  topic length LSB
	length += 2;
	
	for (uint16_t k = 0; k < len; k++) {
		buffer[length] = topic[k];
		length++;
	}
	
	if (qos > 0) {
		buffer[length] = (packet_id_counter >> 8) & 0x00FF; // Packet Identifier MSB
		buffer[length+1] = packet_id_counter & 0x00FF;		// Packet Identifier LSB
		length += 2;
		packet_id_counter++;
	}
	
	len = lenpay;
	for (uint16_t k = 0; k < len; k++) {
		buffer[length] = payload[k];
		length++;
	}
	
	char tmp[30];
	snprintf(tmp, sizeof(tmp), "\t---> 0x%02X - Publish message", buffer[0]);
	DEBUG_PRINTLN(tmp);
	
	writeSSL(buffer, length);
}

void UCxMQTTS::subscribe(char *topic, uint16_t topiclen, uint8_t qos) {
	clearBuffer();
	buffer[0] = MQTT_CTRL_SUBSCRIBE << 4 | 0x02; // Reserved must be 0x02
	buffer[1] = 0x00;		// Remaining Length
	buffer[2] = (packet_id_counter >> 8) & 0xFF; // Packet Identifier MSB
	buffer[3] = packet_id_counter & 0xFF;			// Packet Identifier LSB
	
	packet_id_counter++;
	
	uint16_t len = topiclen;
	buffer[4] = (len>>8)&0x00FF;	//  topic length MSB
	buffer[5] = len&0x00FF;				//  topic length LSB

	uint16_t all_len = 6;
	for (uint16_t k = 0; k < len; k++) {
		buffer[all_len] = topic[k];
		all_len++;
	}
	buffer[all_len] = qos;
	buffer[1] = all_len-1;
	
	char tmp[30];
	snprintf(tmp, sizeof(tmp), "\t---> 0x%02X - Subscribe request", buffer[0]);
	DEBUG_PRINTLN(tmp);
	
	writeSSL(buffer, all_len+1);
}

void UCxMQTTS::unsubscribe(char *topic, uint16_t topiclen) {
	clearBuffer();
	buffer[0] = MQTT_CTRL_UNSUBSCRIBE << 4 | 0x02; // Reserved must be 0x02
	buffer[1] = 0x00;		// Remaining Length
	buffer[2] = (packet_id_counter >> 8) & 0xFF; // Packet Identifier MSB
	buffer[3] = packet_id_counter & 0xFF;			// Packet Identifier LSB
	
	packet_id_counter++;
	
	uint16_t len = topiclen;
	buffer[4] = (len>>8)&0x00FF;	//  topic length MSB
	buffer[5] = len&0x00FF;				//  topic length LSB

	uint16_t all_len = 6;
	for (uint16_t k = 0; k < len; k++) {
		buffer[all_len] = topic[k];
		all_len++;
	}
	
	buffer[1] = all_len-2;
	
	char tmp[30];
	snprintf(tmp, sizeof(tmp), "\t---> 0x%02X - Unsubscribe request", buffer[0]);
	DEBUG_PRINTLN(tmp);
	
	writeSSL(buffer, all_len);
}

void UCxMQTTS::clearBuffer() {
	for (uint16_t k = 0; k < MQTT_MAX_PACKET_SIZE; k++) {
		buffer[k] = 0;
	}
	//gsm.flush();
}

void UCxMQTTS::ping() {
	buffer[0] = MQTT_CTRL_PINGREQ << 4;
	buffer[1] = 0x00;
	
	char tmp[30];
	snprintf(tmp, sizeof(tmp), "\t---> 0x%02X - PING request", buffer[0]);
	DEBUG_PRINTLN(tmp);
	
	writeSSL(buffer, 2);
}

void UCxMQTTS::mqttLoop() {
	unsigned char ret;
	static byte ping_cnt = 0;
	char tmp[350];
	
	if (millis() - previousMillis_ping > PING_INTERVAL) {
		ping();
		ping_cnt++;
		previousMillis_ping = millis();
	}
	
	if (ping_cnt > MAX_PING_CNT) {
		DEBUG_PRINTLN(F("Max ping cnt exceeded"));		
		ping_cnt = 0;
		connected = false;
		return;
	}
	
	uint16_t buf_cnt = 0;
	//while(1) {
		uint16_t len_in_buffer = readDataInBufferMode(1);
		if (len_in_buffer == 0)	{
			clearBuffer();
			return;						
		}
		
		switch (buffer[0]) {
			case MQTT_CTRL_CONNECTACK << 4: // Connect acknowledgment
				readDataInBufferMode(3);
				connectReturnCode = buffer[2];
				
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Connect acknowledgment: %d,%d,%d", buffer[0], buffer[0], buffer[1], buffer[2]);
				DEBUG_PRINTLN(tmp);
				connectAck = true;
				return;
				break;
			case MQTT_CTRL_PUBLISH << 4: // Publish message
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Publish message", buffer[0]);
				DEBUG_PRINTLN(tmp);
				checkRXsub();
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBACK << 4: // Publish acknowledgment
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Publish acknowledgment", buffer[0]);
				DEBUG_PRINTLN(tmp);
				previousMillis_ping = millis(); // No need to ping now
				break;
			case MQTT_CTRL_PUBREC << 4: // Publish received (assured delivery part 1)
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Publish received (assured delivery part 1)", buffer[0]);
				DEBUG_PRINTLN(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBREL << 4: // Publish release (assured delivery part 2)
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Publish release (assured delivery part 2)", buffer[0]);
				DEBUG_PRINTLN(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBCOMP << 4: // Publish complete (assured delivery part 3)
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Publish complete (assured delivery part 3)", buffer[0]);
				DEBUG_PRINTLN(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case 0x32: // rx_sub + retain
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Publish message with retain", buffer[0]);
				DEBUG_PRINTLN(tmp);
				checkRXsub();
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_SUBACK << 4: // Subscribe acknowledgment
				readDataInBufferMode(4);
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Subscribe acknowledgment: %d,%d,%d,%d", buffer[0], buffer[0], buffer[1], buffer[2], buffer[3]);
				DEBUG_PRINTLN(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
			  break;
			case MQTT_CTRL_UNSUBACK << 4: // Unsubscribe acknowledgment
				readDataInBufferMode(3);
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Unsubscribe acknowledgment: %d,%d,%d", buffer[0], buffer[0], buffer[1], buffer[2]);
				DEBUG_PRINTLN(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
			  break;
			case MQTT_CTRL_PINGRESP << 4: // PING response
				ping_cnt = 0;
				readDataInBufferMode(1);
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - PING response:  %d", buffer[0], buffer[0]);
				DEBUG_PRINTLN(tmp);
				if(buffer[0] == 0x00) {
					connected = true;
					//DEBUG_PRINTLN("ping ok");
					return;
				} else {
					connected = false;
					//DEBUG_PRINTLN("ping fail");
					clearBuffer();
					return;	
				}
				break;
			default:
				snprintf(tmp, sizeof(tmp), "\t<--- 0x%02X - Unknown Control Packet Type, clearing buffers?", buffer[0]);
				DEBUG_PRINTLN(tmp);
				clearBuffer();
				//gsm.flush(); // ?? Is this best or not??
				return;
				break;
		}
		if (connected == false) {
			return;
		}
	//}
}

void UCxMQTTS::checkRXsub() {
	uint16_t all_byte;
	unsigned char topic_len;
	unsigned char topic_cnt = 0;
	unsigned char payload_cnt = 0;
	
	uint8_t  header = buffer[0]; // sub_head
	unsigned int len_in_buffer = readDataInBufferMode(1);
	all_byte = buffer[0];
	len_in_buffer = readDataInBufferMode(2);
	topic_len = buffer[1];
	len_in_buffer = readDataInBufferMode(all_byte-2);
	char topic[topic_len+1];
	char payload[len_in_buffer+1];
	//char topic[100];
	//char payload[100];
	
	for (uint16_t k = 0; k < len_in_buffer; k++) {
		if (k < topic_len) {
			topic[topic_cnt] = buffer[k];
			topic_cnt++;
			if (topic_cnt >= 100) {
				break;
			}
		} else {
			payload[payload_cnt] = buffer[k];
			payload_cnt++;
			//if(payload_cnt>100)
			//	break;
		}
	}
	topic[topic_cnt] = 0;
	String str_topic(topic);
	if (header == 0x32)	{
		(*callback)(str_topic, payload+2, payload_cnt-2);
	}	else {
		(*callback)(str_topic, payload, payload_cnt);
	}
}

uint16_t UCxMQTTS::readDataInBufferMode(uint16_t buf_len) {
	uint16_t len = ssl_mqtts.readBuffer(buf_len);
	uint16_t re_turn = len; 
	uint16_t ret = 0;
	
	// THIS IS RISKY, IF MODEM DID NOT SEND CORRECT DATA WE GET STUCK IN WHILE
	// WATCHDOG WILL RESET UNIT
	while (len) {
		if (gsm.available()) {
			char c = gsm.read();
			buffer[ret] = c;
			ret++;
			len--;
			if (ret > MQTT_MAX_PACKET_SIZE)
				return (ret);
		}
	}
	//}
	//gsm.flush();
	gsm.waitOK(1000);
	return (re_turn);
}

bool UCxMQTTS::connectState() {
	return (connected);
}
