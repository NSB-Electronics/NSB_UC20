#include "uc_mqtts.h"
#include "ssl.h"
SSL ssl_mqtts;

void func_null_mqtts(String topic,char *playload, unsigned char length){}
	
UCxMQTTS::UCxMQTTS() {
	//void (*callback)(String topic ,char *playload,unsigned char length);
	callback = func_null_mqtts;
}

bool UCxMQTTS::connectMQTTServer(unsigned char pdpid, unsigned char contextid, unsigned char clientid, String web, String port, unsigned char mode) {
	connected = ssl_mqtts.open(pdpid, contextid, clientid, web, port, mode); //buffer mode	
	return (connected);
}

bool UCxMQTTS::connectMQTTServer(unsigned char contextid, String web, String port) {
	connected = connectMQTTServer(1, contextid, 0, web, port, 0);
	return (connected);
}

bool UCxMQTTS::connectMQTTServer(String web, String port) {
	connected = connectMQTTServer(1, 1, 0, web, port, 0);
	return (connected);
}

bool UCxMQTTS::disconnectMQTTServer(unsigned char clientid) {
	connected = false;
	return (ssl_mqtts.close(clientid));
}

bool UCxMQTTS::disconnectMQTTServer() {
	return (disconnectMQTTServer(0));
}

// The current MQTT spec is 3.1.1 and available here:
//   http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
unsigned char UCxMQTTS::connectMQTTUser(String id, String user, String pass) {
	
	// Leave room in the buffer for header and variable length field
	uint32_t length = 0;		// Total length of the mqtt data (excludes fixed header)
	uint32_t varLength = 0;		// Stores the temp variable length in order to encode the data length
	uint32_t payloadLen = 0;	// Payload length
	uint8_t encodedByte; 	    // Variable length bytes can be up to 4 bytes long
	uint8_t variableHeaderLen = 10; // Always 10 for a CONNECT packet
	uint8_t connectFlags = MQTT_CONN_CLEANSESSION;	// Always clean the session
	
	payloadLen = id.length() + user.length() + pass.length();
	if (id.length() > 0) {
		payloadLen += 2;
	}
	if (user.length() > 0) {
		payloadLen += 2;
		connectFlags |= MQTT_CONN_USERNAMEFLAG;
	}
	if (pass.length() > 0) {
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
	
	unsigned char len = id.length();
	buffer[length] = (len >> 8) & 0x00FF; // clientid length MSB
	buffer[length+1] = len & 0x00FF;      // clientid length LSB
	length += 2;
	
	for (byte k = 0; k < len; k++) {
		buffer[length] = id[k];
		length++;
		if (length >= MQTT_MAX_PACKET_SIZE) {
			Serial.println(F("MQTT_MAX_PACKET_SIZE exceeded"));
			return (0xFF);
		}
	}

	len = user.length();
	buffer[length] = (len >> 8) & 0x00FF; // username length MSB
	length++;
	buffer[length] = len & 0x00FF;        // username length LSB
	length++;
	for (byte k = 0; k < len; k++) {
		buffer[length] = user[k];
		length++;
		if (length >= MQTT_MAX_PACKET_SIZE) {
			Serial.println(F("MQTT_MAX_PACKET_SIZE exceeded"));
			return (0xFF);
		}
	}
	
	len = pass.length();
	if (len > 0) {
		buffer[length] = (len >> 8) & 0x00FF; // password length MSB
		length++;
		buffer[length] = len & 0x00FF;        // password length LSB
		length++;
		for (byte k = 0; k < len; k++) {
			buffer[length] = pass[k];
			length++;
			if (length >= MQTT_MAX_PACKET_SIZE) {
				Serial.println(F("MQTT_MAX_PACKET_SIZE exceeded"));
				return (0xFF);
			}
		}
	}
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp.concat(dat);
	tmp.concat(F(" -> Connect request"));
	gsm.debug(tmp);
	Serial.println(tmp);
	
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
			//Serial.println("Connect Ack!");
			break;
		}
	}
	previousMillis_ping = millis();
	
	if (connectAck == true) {
		//Serial.print("Connect return: ");
		//Serial.println(connectReturnCode);
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
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp.concat(dat);
	tmp.concat(F(" -> Disconnect request"));
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, 2);
	return true;
}

uint16_t UCxMQTTS::readDataFrom3GBufferMode() {
	uint16_t ret = 99;
	unsigned long pv_timeout = millis();;
	const long interval_timeout = 5000;
	bool state_data_in = false;
	//Serial.println();
	//Serial.println("testbuf");
	//Serial.println();
	
	while(1) {
		unsigned long current_timeout = millis();
		ret = 0;
		if (ssl_mqtts.receiveAvailable()) {
			uint16_t len = ssl_mqtts.readBuffer(10);
			//Serial.print("len = ");
			//Serial.println(len);
			while (len) {
				if (gsm.available()) {
					char c = gsm.read();
					buffer[ret] = c;
					ret++;
					//Serial.print(c,HEX);
					len--;
				}
			}
			if (buffer[0] = 0x20) { // WHAT IS 0x20??
				ret -= 1;
				//Serial.println(ret);
				return (ret);
			}
		}
		if (current_timeout - pv_timeout >= interval_timeout) {
			//Serial.println("readDataFrom3GBufferMode timeout");
			return (-1);
		}
	}
	
}


uint16_t UCxMQTTS::writeSSL(uint8_t* buf, uint16_t length) {
	uint16_t len_let = 0;
	if (ssl_mqtts.startSend(0, length)) {
		for (uint16_t itcp = 0; itcp < length; itcp++) {
			ssl_mqtts.write(buf[itcp]);
			len_let++;
		}
		if(!ssl_mqtts.waitSendFinish()) {
			//Serial.println("false unfinish");
			connected = false;
		}
	} else {
		connected = false;
		//Serial.println("startSend ERROR");
	}
  return (len_let);
}

String UCxMQTTS::connectCodeString(unsigned char input) {
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
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp.concat(dat);
	tmp.concat(F(" -> Publish message"));
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, length);
}

void UCxMQTTS::publish(String topic, String payload, uint8_t qos) {
	
	char chartopic[topic.length()+2];
	char charpay[payload.length()+2];
	
	for (uint16_t k = 0; k < topic.length(); k++) {
		chartopic[k] = topic[k];
	}
	chartopic[topic.length()] = 0;
	
	for (uint16_t k = 0; k < payload.length(); k++) {
		charpay[k] = payload[k];
	}
	charpay[payload.length()] = 0;
	
	publish(chartopic, topic.length(), charpay, payload.length(), qos);
}

void UCxMQTTS::publish(String topic, String payload) {
	
	char chartopic[topic.length()+2];
	char charpay[payload.length()+2];
	
	for (uint16_t k = 0; k < topic.length(); k++) {
		chartopic[k] = topic[k];
	}
	chartopic[topic.length()] = 0;
	
	for (uint16_t k = 0; k < payload.length(); k++) {
		charpay[k] = payload[k];
	}
	charpay[payload.length()] = 0;
	
	publish(chartopic, topic.length(), charpay, payload.length(), MQTT_QOS_0);
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
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp.concat(dat);
	tmp.concat(F(" -> Subscribe request"));
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, all_len+1);
}

void UCxMQTTS::subscribe(String topic, uint8_t qos) {
	char chartopic[topic.length()+2];
	
	for (uint16_t k = 0; k < topic.length(); k++) {
		chartopic[k] = topic[k];
	}
	chartopic[topic.length()] = 0;
	
	subscribe(chartopic, topic.length(), qos);
}

void UCxMQTTS::subscribe(String topic) {
	char chartopic[topic.length()+2];
	
	for (uint16_t k = 0; k < topic.length(); k++) {
		chartopic[k] = topic[k];
	}
	chartopic[topic.length()] = 0;
	
	subscribe(chartopic, topic.length(), MQTT_QOS_0);
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
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp.concat(dat);
	tmp.concat(F(" -> Unsubscribe request"));
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, all_len);
}

void UCxMQTTS::unsubscribe(String topic) {
	char chartopic[topic.length()+2];
	
	for (uint16_t k = 0; k < topic.length(); k++) {
		chartopic[k] = topic[k];
	}
	chartopic[topic.length()] = 0;
	
	unsubscribe(chartopic, topic.length());
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
	
	String tmp = F("0x");
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp.concat(dat);
	tmp.concat(F(" -> PING request"));
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, 2);
}

void UCxMQTTS::mqttLoop() {
	unsigned char ret;
	static byte ping_cnt = 0;
	
	if (millis() - previousMillis_ping > PING_INTERVAL) {
		ping();
		ping_cnt++;
		previousMillis_ping = millis();
	}
	
	if (ping_cnt > MAX_PING_CNT) {
		gsm.debug(F("Max ping cnt exceeded"));		
		Serial.println(F("Max ping cnt exceeded"));
		ping_cnt = 0;
		connected = false;
		return;
	}
	
	uint16_t buf_cnt = 0;
	while(1) {
		uint16_t len_in_buffer = readDataInBufferMode(1);
		if (len_in_buffer == 0)	{
			clearBuffer();
			return;						
		}
		String tmp = F("0x");
		String dat = String(buffer[0], HEX);
		dat.toUpperCase();
		tmp.concat(dat);
		tmp.concat(F(" <- "));
		gsm.debug(tmp);
		Serial.print(tmp);
		
		switch (buffer[0]) {
			case MQTT_CTRL_CONNECTACK << 4: // Connect acknowledgment
				readDataInBufferMode(3);
				connectReturnCode = buffer[2];
				tmp = F("Connect acknowledgment: ");
				tmp.concat(buffer[0]); // Remaining length
				tmp.concat(",");
				tmp.concat(buffer[1]); // Connect Acknowledge Flags
				tmp.concat(",");
				tmp.concat(buffer[2]); // Connect Return code
				gsm.debug(tmp);
				Serial.println(tmp);
				connectAck = true;
				return;
				break;
			case MQTT_CTRL_PUBLISH << 4: // Publish message
				gsm.debug(F("Publish message"));
				Serial.println(F("Publish message"));
				checkRXsub();
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBACK << 4: // Publish acknowledgment
				gsm.debug(F("Publish acknowledgment"));
				Serial.println(F("Publish acknowledgment"));
				previousMillis_ping = millis(); // No need to ping now
				break;
			case MQTT_CTRL_PUBREC << 4: // Publish received (assured delivery part 1)
				gsm.debug(F("Publish received (assured delivery part 1)"));
				Serial.println(F("Publish received (assured delivery part 1)"));
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBREL << 4: // Publish release (assured delivery part 2)
				gsm.debug(F("Publish release (assured delivery part 2)"));
				Serial.println(F("Publish release (assured delivery part 2)"));
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBCOMP << 4: // Publish complete (assured delivery part 3)
				gsm.debug(F("Publish complete (assured delivery part 3)"));
				Serial.println(F("Publish complete (assured delivery part 3)"));
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case 0x32: // rx_sub + retain
				gsm.debug(F("Publish message with retain"));
				Serial.println(F("Publish message with retain"));
				checkRXsub();
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_SUBACK << 4: // Subscribe acknowledgment
				readDataInBufferMode(4);
				tmp = F("Subscribe acknowledgment: ");
				tmp.concat(buffer[0]); // Remaining length
				tmp.concat(",");
				tmp.concat(buffer[1]); // Packet identifier MSB
				tmp.concat(",");
				tmp.concat(buffer[2]); // Packet identifier LSB
				tmp.concat(",");
				tmp.concat(buffer[3]); // Return code
				gsm.debug(tmp);
				Serial.println(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
			  break;
			case MQTT_CTRL_UNSUBACK << 4: // Unsubscribe acknowledgment
				readDataInBufferMode(3);
				tmp = F("Unsubscribe acknowledgment: ");
				tmp.concat(buffer[0]); // Remaining length
				tmp.concat(",");
				tmp.concat(buffer[1]); // Packet identifier MSB
				tmp.concat(",");
				tmp.concat(buffer[2]); // Packet identifier LSB
				gsm.debug(tmp);
				Serial.println(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
			  break;
			case MQTT_CTRL_PINGRESP << 4: // PING response
				
				ping_cnt = 0;
				readDataInBufferMode(1);
				tmp = F("PING response: ");
				tmp.concat(buffer[0]); // Remaining length
				gsm.debug(tmp);
				Serial.println(tmp);
				if(buffer[0] == 0x00) {
					connected = true;
					//Serial.println("ping ok");
					return;
				} else {
					connected = false;
					//Serial.println("ping fail");
					clearBuffer();
					return;	
				}
				
				break;
			default:
				gsm.debug(F("Unknown Control Packet Type, clearing buffers?"));
				Serial.println(F("Unknown Control Packet Type, clearing buffers?"));
				clearBuffer();
				//gsm.flush(); // ?? Is this best or not??
				return;
				break;
		}
		if (connected == false) {
			return;
		}
	}			

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
			//Serial.write(payload[payload_cnt]);
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
	
	//Serial.println("");
	//Serial.print("len = ");
	//Serial.println(len);
	
	while (len) {
		if (gsm.available()) {
			char c = gsm.read();
			//Serial.print(c,HEX);
			buffer[ret] = c;
			ret++;
			len--;
			if (ret > MQTT_MAX_PACKET_SIZE)
				return (ret);
		}
	}
	//}
	//gsm.flush();
	gsm.wait_ok_ndb(1000);
	return (re_turn);
}

bool UCxMQTTS::connectState() {
	return (connected);
}
