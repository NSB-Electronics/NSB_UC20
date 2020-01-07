#include "NSB_UC20.h"
//#include "SoftwareSerial.h"
//#include <AltSoftSerial.h>
#include "internet.h"
#include "uc_mqtt.h"

UC20 gsm;
INTERNET net;
UCxMQTT mqtt;

#define FORCE_REBOOT  1

#define UC20_PWR_PIN  12

#define APN "internet"
#define USER ""
#define PASS ""

#define MQTT_SERVER      ""
#define MQTT_PORT        ""
#define MQTT_ID          ""   // Used to identify the connection
#define MQTT_USER        ""
#define MQTT_PASSWORD    ""

unsigned long previousmqtt = 0;
const long intervalmqtt = 60000;

//AltSoftSerial mySerial;

String feedName;
String feedPayload;

//  ---------------------------------------------------------------------------
/// debugGSM
//  ---------------------------------------------------------------------------
void debugGSM(String data) {
  // Forwards GSM debug messages
  Serial.println(data);
} // debugGSM

//  ---------------------------------------------------------------------------
/// Setup
//  ---------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(3000);
  
  gsm.begin(&Serial1, 115200);
  
  //gsm.Event_debug = debugGSM; // Enable debug messages
  
  Serial.println(F("I:\tInit Modem"));
  #if FORCE_REBOOT
      // Modem not responding, hard reset
      Serial.println("I:\tSoftware Reset Modem");
      gsm.setAutoReset(1, 0); // For now doing soft reset, don't have access to UC20 reset pin
      //gsm.setPowerKeyPin(UC20_PWR_PIN);
      //gsm.PowerOn();
      Serial.print("I:\tWait for Modem Ready: ");
      Serial.println(gsm.waitReady());
  #else
    if (gsm.waitOK()) {
      Serial.println(F("I:\tUC20 Ready..."));
      gsm.setEchoMode(false);
    } else {
    // Modem not responding, hard reset
    Serial.println("I:\tSoftware Reset Modem");
    gsm.setAutoReset(1, 0); // For now doing soft reset, don't have access to UC20 reset pin
    //gsm.setPowerKeyPin(UC20_PWR_PIN);
    //gsm.PowerOn();
    Serial.print("I:\tWait for Modem Ready: ");
    Serial.println(gsm.waitReady());
    }
  #endif

  Serial.println("I:\tReset to Defaults");
  gsm.resetDefaults();
  Serial.println("I:\tSet Echo Off");
  gsm.setEchoMode(false);
  
  Serial.print("I:\t");
  Serial.println(gsm.moduleInfo());
  
  if (gsm.waitSIMReady()) {
    Serial.println(F("I:\tSIM Ready"));
  } else {
    Serial.print(F("E:\tSIM Issue: "));
    Serial.println(gsm.getSIMStatus());
  }

  if (gsm.waitNetworkRegistered()) {
    Serial.print(F("I:\tNetwork Registered: "));
    Serial.println(gsm.getOperator());
    Serial.print(F("I:\tSignal Quality: "));
    Serial.println(gsm.signalQuality());
  } else {
    Serial.print(F("E:\tNetwork Registration Issue: "));
    Serial.println(gsm.getNetworkStatus());
  }
  
  Serial.print(F("I:\tDisconnect Internet: "));
  Serial.println(net.DisConnect());
  
  Serial.print(F("I:\tSet APN and Password: "));
  Serial.println(net.Configure(APN, USER, PASS));
  
  Serial.print(F("I:\tConnect to Internet: "));
  Serial.println(net.Connect());
  
  Serial.print(F("I:\tIP: "));
  Serial.println(net.GetIP());


  feedName.concat(MQTT_USER);
  feedName.concat("/feeds/testmqtt");

  mqtt.callback = callback;
  
  connectToServer();
} //setup

//  ---------------------------------------------------------------------------
/// callback
//  ---------------------------------------------------------------------------
void callback(String topic, char *payload, unsigned char payloadLength) {
  Serial.println();
  Serial.print(F("I:\tTopic --> "));
  Serial.println(topic);
  payload[payloadLength] = 0;
  String str_data(payload);
  Serial.print(F("I:\tPayload --> "));
  Serial.println(str_data);
} // callback

//  ---------------------------------------------------------------------------
/// connectToServer
//  ---------------------------------------------------------------------------
void connectToServer() {
  do {
    Serial.print(F("I:\tConnecting to MQTT Server: "));
    if (mqtt.DisconnectMQTTServer()) {
      mqtt.ConnectMQTTServer(MQTT_SERVER, MQTT_PORT);
    }
    delay(500);
    Serial.println(mqtt.ConnectState());
  } while(!mqtt.ConnectState());
  Serial.println(F("I:\tMQTT Server Connected"));
  unsigned char ret = mqtt.Connect(MQTT_ID, MQTT_USER, MQTT_PASSWORD);
  Serial.print(F("I:\tMQTT Server Login: "));
  Serial.println(mqtt.ConnectReturnCode(ret));
  
  mqtt.Subscribe(feedName);
  mqtt.Subscribe("time/seconds"); // Adafruit IO specific
 
} // connectToServer

//  ---------------------------------------------------------------------------
/// loop
//  ---------------------------------------------------------------------------
void loop() {
  // Publish to feed every interval
  if (millis() - previousmqtt >= intervalmqtt) {
    previousmqtt = millis();
    feedPayload = String(float(random(46000, 56000))*0.001);
    mqtt.Publish(feedName, feedPayload);
  }
  
  mqtt.MqttLoop();
  
  if (!mqtt.ConnectState()) {
     connectToServer();
  }
} // loop

