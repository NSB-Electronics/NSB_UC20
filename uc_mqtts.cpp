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

bool UCxMQTTS::disconnectMQTTServer(unsigned char contextid) {
	connected = false;
	return (ssl_mqtts.close(contextid));
}

bool UCxMQTTS::disconnectMQTTServer() {
	return (disconnectMQTTServer(1));
}

// The current MQTT spec is 3.1.1 and available here:
//   http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
unsigned char UCxMQTTS::connectMQTTUser(String id, String user, String pass) {
	
	// Leave room in the buffer for header and variable length field
	uint16_t length = 0;
	unsigned int j;
	unsigned char ctrl_flag = MQTT_CONN_CLEANSESSION;	// Always clean the session
	buffer[0] = (MQTT_CTRL_CONNECT << 4) | 0x0; // Control packet type
	buffer[1] = 0x00;    // Remaining Length
	buffer[2] = 0x00;    // Protocol name Length MSB (Fix=0)
	buffer[3] = 0x04;    // Protocol name Length LSB (Fix=4)
	buffer[4] = 'M';     // Protocol name   (Fix=M)
	buffer[5] = 'Q';     // Protocol name   (Fix=Q)
	buffer[6] = 'T';     // Protocol name   (Fix=T)
	buffer[7] = 'T';     // Protocol name   (Fix=T)
	buffer[8] = MQTT_PROTOCOL_LEVEL;    // Protocol Level  (Fix=4)
	//buffer[9] = 0xC2;    // Control Flag (Bit7 : user name flag),(Bit6 : password flag) , (Bit5 : will retain) , (Bit4-3 : will Qos) , (Bit2 : will flag) , (Bit1 : clear session) , (Bit0 : fix0)
	buffer[10] = MQTT_CONN_KEEPALIVE >> 8;   // keep alive MSB
	buffer[11] = MQTT_CONN_KEEPALIVE & 0xFF; // keep alive LSB time ping 16 bit (seconds)

	
	unsigned char  i;
	unsigned char len = id.length();
	buffer[12] = (len>>8)&0x00FF;   // clientid length MSB
	buffer[13] = len&0x00FF;        // clientid length LSB
	
	length = 14;
	for (i=0; i<len; i++) {
		buffer[length] = id[i];
		length++;
	}

	len = user.length();
	if (len > 0) ctrl_flag |= 1<<7;
	buffer[length] = (len>>8)&0x00FF;   // username length MSB
	length++;
	buffer[length] = len&0x00FF;        // username length LSB
	length++;
	for (i=0; i<len; i++) {
		buffer[length] = user[i];
		length++;
	}

	len = pass.length();
	if (len > 0) {
		ctrl_flag |= 1<<6;
		buffer[length] = (len>>8)&0x00FF;   // password length MSB
		length++;
		buffer[length] = len&0x00FF;        // password length LSB
		length++;
		for (i=0; i<len; i++) {
			buffer[length] = pass[i];
			length++;
		}
	}
	
	buffer[1] = length-2;
	buffer[9] = ctrl_flag;
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp += dat;
	tmp += " -> Connect request";
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
	
	// REWRITE TO WAIT FOR THE ACK CODE - SEE ABOVE
//	int ret = readDataFrom3GBufferMode();
	
	// if (ret == -1) {
		// connected = false;
		// return (0xFF);
	// }	else if (buffer[ret] == 0)
		// connected = true;
	// else {
		// connected = false;
	// }
}

bool UCxMQTTS::disconnectMQTTUser() {
	buffer[0] = MQTT_CTRL_DISCONNECT << 4;
	buffer[1] = 0;
	
	connected = false;
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp += dat;
	tmp += " -> Disconnect request";
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, 2);
  return true;
}

int UCxMQTTS::readDataFrom3GBufferMode() {
	int ret = 99;
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
			int len = ssl_mqtts.readBuffer(10);
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


uint8_t UCxMQTTS::writeSSL(uint8_t* buf, uint8_t length) {
	uint8_t len_let = 0;
	if (ssl_mqtts.startSend(0,length)) {
		for (int itcp=0; itcp<length; itcp++) {
			ssl_mqtts.write(buf[itcp]);
			//Serial.write(buf[itcp]);
			len_let++;
	  }
		//Serial.println();
	  if(!ssl_mqtts.waitSendFinish()) {
		  //Serial.println("false unfinish");
		  connected = false;
	  }
	}	else {
		connected = false;
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
		case 0xFF: 	return F("0xFF TimeOut Occurred");
		default: 		return F("Unknown Code!");
	}
}

void UCxMQTTS::publish(char *topic, int lentopic, char *payload, int lenpay, uint8_t qos) {
	
	// ToDO: Need to add QoS and retain?
	clearBuffer();

	buffer[0] = MQTT_CTRL_PUBLISH << 4 | qos << 1;    // Control packet type (Fix=3x) ,x bit3 = DUP , xbit2-1 = QoS level , xbit0 = Retain
	buffer[1] = 0x00;     // remaining length

	int i = 0;
	int len = lentopic;
	buffer[2] = (len>>8)&0x00FF;    //  topic length MSB
	buffer[3] = len&0x00FF;    //  topic length LSB

	int all_len = 4;
	for (i=0; i<len; i++) {
		buffer[all_len] = topic[i];
		all_len++;
	}

	len = lenpay;
	for (i=0; i<len; i++) {
		buffer[all_len] = payload[i];
		all_len++;
	}
	
	if (qos > 0) {
		buffer[all_len] = (packet_id_counter >> 8) & 0xFF; // Packet Identifier MSB
		buffer[all_len+1] = packet_id_counter & 0xFF;			// Packet Identifier LSB
		all_len += 2;
		packet_id_counter++;
	}

	buffer[1] = all_len-2;
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp += dat;
	tmp += " -> Publish message";
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, all_len);
}

void UCxMQTTS::publish(String topic, String payload, uint8_t qos) {
	
	char chartopic[topic.length()+2];
	char charpay[payload.length()+2];
	unsigned char i = 0;
	for (i=0; i<topic.length(); i++) {
		chartopic[i] = topic[i];
	}
	chartopic[i] = 0;
	
	for (i=0; i<payload.length(); i++) {
		charpay[i] = payload[i];
	}
	charpay[i] = 0;
	
	publish(chartopic, topic.length(), charpay, payload.length(), qos);
}

void UCxMQTTS::publish(String topic, String payload) {
	
	char chartopic[topic.length()+2];
	char charpay[payload.length()+2];
	unsigned char i = 0;
	for (i=0; i<topic.length(); i++) {
		chartopic[i] = topic[i];
	}
	chartopic[i] = 0;
	
	for (i=0; i<payload.length(); i++) {
		charpay[i] = payload[i];
	}
	charpay[i] = 0;
	
	publish(chartopic, topic.length(), charpay, payload.length(), MQTT_QOS_0);
}

void UCxMQTTS::subscribe(char *topic, int topiclen, uint8_t qos) {
	clearBuffer();
	buffer[0] = MQTT_CTRL_SUBSCRIBE << 4 | 0x02; // Reserved must be 0x02
	buffer[1] = 0x00;		// Remaining Length
	buffer[2] = (packet_id_counter >> 8) & 0xFF; // Packet Identifier MSB
	buffer[3] = packet_id_counter & 0xFF;			// Packet Identifier LSB
	
	packet_id_counter++;
	
	int i = 0;
	int len = topiclen;
	buffer[4] = (len>>8)&0x00FF;	//  topic length MSB
	buffer[5] = len&0x00FF;				//  topic length LSB

	int all_len = 6;
	for (i=0; i<len; i++) {
		buffer[all_len] = topic[i];
		all_len++;
	}
	buffer[all_len] = qos;
	buffer[1] = all_len-1;
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp += dat;
	tmp += " -> Subscribe request";
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, all_len+1);
}

void UCxMQTTS::subscribe(String topic, uint8_t qos) {
	char chartopic[topic.length()+2];
	unsigned char i = 0;
	for (i=0; i<topic.length(); i++) {
		chartopic[i] = topic[i];
	}
	chartopic[i] = 0;
	
	subscribe(chartopic, topic.length(), qos);
}

void UCxMQTTS::subscribe(String topic) {
	char chartopic[topic.length()+2];
	unsigned char i = 0;
	for (i=0; i<topic.length(); i++) {
		chartopic[i] = topic[i];
	}
	chartopic[i] = 0;
	
	subscribe(chartopic, topic.length(), MQTT_QOS_0);
}

void UCxMQTTS::unsubscribe(char *topic, int topiclen) {
	clearBuffer();
	buffer[0] = MQTT_CTRL_UNSUBSCRIBE << 4 | 0x02; // Reserved must be 0x02
	buffer[1] = 0x00;		// Remaining Length
	buffer[2] = (packet_id_counter >> 8) & 0xFF; // Packet Identifier MSB
	buffer[3] = packet_id_counter & 0xFF;			// Packet Identifier LSB
	
	packet_id_counter++;
	
	int i = 0;
	int len = topiclen;
	buffer[4] = (len>>8)&0x00FF;	//  topic length MSB
	buffer[5] = len&0x00FF;				//  topic length LSB

	int all_len = 6;
	for (i=0; i<len; i++) {
		buffer[all_len] = topic[i];
		all_len++;
	}
	
	buffer[1] = all_len-2;
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp += dat;
	tmp += " -> Unsubscribe request";
	gsm.debug(tmp);
	Serial.println(tmp);
	writeSSL(buffer, all_len);
}

void UCxMQTTS::unsubscribe(String topic) {
	char chartopic[topic.length()+2];
	unsigned char i = 0;
	for (i=0; i<topic.length(); i++) {
		chartopic[i] = topic[i];
	}
	chartopic[i] = 0;
	
	unsubscribe(chartopic, topic.length());
}

void UCxMQTTS::clearBuffer() {
	for (int i=0; i<MQTT_MAX_PACKET_SIZE; i++) {
		buffer[i] = 0;
	}
	//gsm.flush();
}

void UCxMQTTS::ping() {
	buffer[0] = MQTT_CTRL_PINGREQ << 4;
	buffer[1] = 0x00;
	
	String tmp = "0x";
	String dat = String(buffer[0], HEX);
	dat.toUpperCase();
	tmp += dat;
	tmp += " -> PING request";
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
		Serial.println("Max ping cnt exceeded");
		ping_cnt = 0;
		connected = false;
		return;
	}
	
	unsigned int buf_cnt = 0;
	while(1) {
		unsigned int len_in_buffer = readDataInBufferMode(1);
		if (len_in_buffer == 0)	{
			clearBuffer();
			return;						
		}
		String tmp = "0x";
		String dat = String(buffer[0], HEX);
		dat.toUpperCase();
		tmp += dat;
		tmp += " <- ";
		gsm.debug(tmp);
		Serial.print(tmp);
		
		switch (buffer[0]) {
			case MQTT_CTRL_CONNECTACK << 4: // Connect acknowledgment
				readDataInBufferMode(3);
				connectReturnCode = buffer[2];
				tmp = "Connect acknowledgment: ";
				tmp += buffer[0]; // Remaining length
				tmp += ",";
				tmp += buffer[1]; // Connect Acknowledge Flags
				tmp += ",";
				tmp += buffer[2]; // Connect Return code
				gsm.debug(tmp);
				Serial.println(tmp);
				connectAck = true;
				return;
				break;
			case MQTT_CTRL_PUBLISH << 4: // Publish message
				gsm.debug(F("Publish message"));
				Serial.println("Publish message");
				checkRXsub();
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBACK << 4: // Publish acknowledgment
				gsm.debug(F("Publish acknowledgment"));
				Serial.println("Publish acknowledgment");
				previousMillis_ping = millis(); // No need to ping now
				break;
			case MQTT_CTRL_PUBREC << 4: // Publish received (assured delivery part 1)
				gsm.debug(F("Publish received (assured delivery part 1)"));
				Serial.println("Publish received (assured delivery part 1)");
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBREL << 4: // Publish release (assured delivery part 2)
				gsm.debug(F("Publish release (assured delivery part 2)"));
				Serial.println("Publish release (assured delivery part 2)");
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_PUBCOMP << 4: // Publish complete (assured delivery part 3)
				gsm.debug(F("Publish complete (assured delivery part 3)"));
				Serial.println("Publish complete (assured delivery part 3)");
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case 0x32: // rx_sub + retain
				gsm.debug(F("Publish message with retain"));
				Serial.println("Publish message with retain");
				checkRXsub();
				previousMillis_ping = millis(); // No need to ping now
				return;
				break;
			case MQTT_CTRL_SUBACK << 4: // Subscribe acknowledgment
				readDataInBufferMode(4);
				tmp = "Subscribe acknowledgment: ";
				tmp += buffer[0]; // Remaining length
				tmp += ",";
				tmp += buffer[1]; // Packet identifier MSB
				tmp += ",";
				tmp += buffer[2]; // Packet identifier LSB
				tmp += ",";
				tmp += buffer[3]; // Return code
				gsm.debug(tmp);
				Serial.println(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
			  break;
			case MQTT_CTRL_UNSUBACK << 4: // Unsubscribe acknowledgment
				readDataInBufferMode(3);
				tmp = "Unsubscribe acknowledgment: ";
				tmp += buffer[0]; // Remaining length
				tmp += ",";
				tmp += buffer[1]; // Packet identifier MSB
				tmp += ",";
				tmp += buffer[2]; // Packet identifier LSB
				gsm.debug(tmp);
				Serial.println(tmp);
				previousMillis_ping = millis(); // No need to ping now
				return;
			  break;
			case MQTT_CTRL_PINGRESP << 4: // PING response
				
				ping_cnt = 0;
				readDataInBufferMode(1);
				tmp = "PING response: ";
				tmp += buffer[0]; // Remaining length
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
				Serial.println("Unknown Control Packet Type, clearing buffers?");
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
	unsigned int all_byte;
	unsigned char topic_len;
	unsigned char topic_cnt = 0;
	unsigned char payload_cnt = 0;
	unsigned int i;
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
	
	for (i=0; i<len_in_buffer; i++) {
		if (i < topic_len) {
			topic[topic_cnt] = buffer[i];
			topic_cnt++;
			if (topic_cnt >= 100) {
				break;
			}
		} else {
			payload[payload_cnt] = buffer[i];
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

unsigned int UCxMQTTS::readDataInBufferMode(unsigned int buf_len) {
	unsigned int len = ssl_mqtts.readBuffer(buf_len);
	unsigned int re_turn = len; 
	unsigned int ret = 0;
	
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













