#include "NSB_UC20.h"
//#include "SoftwareSerial.h"
//#include <AltSoftSerial.h>
#include "internet.h"
#include "uc_mqtts.h"
#include "ssl.h"

#define FORCE_REBOOT  1

#define UC20_PWR_PIN  12

#define APN "internet"
#define USER ""
#define PASS ""

#define SSL_CONTEXT_ID  1 

#define MQTT_SERVER     "io.adafruit.com"
#define MQTT_PORT       "8883"
#define MQTT_ID         "MQTTS_TEST"   // Used to identify the connection
#define MQTT_USER       "YOUR_USERNAME"
#define MQTT_PASSWORD   "YOUR_PASSWORD/KEY"


UC20 gsm;
INTERNET net;
UCxMQTTS mqtt;
SSL ssl;

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

  randomSeed(analogRead(0));
  
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
    delay(5000); // Give some time before getting signal quality
    unsigned char rssi = gsm.signalQuality();
    int dBm = gsm.signalQualitydBm(rssi);
    unsigned char  quality = gsm.signalQualityPercentage(rssi);
    Serial.print(F("I:\tSignal Quality: "));
    Serial.print(rssi);
    Serial.print(F("(rssi), "));
    Serial.print(dBm);
    Serial.print(F("(dBm), "));
    Serial.print(quality);
    Serial.println(F("%"));
  } else {
    Serial.print(F("E:\tNetwork Registration Issue: "));
    Serial.println(gsm.getNetworkStatus());
    Serial.println(F("E:\tHALTING"));
    while (1) {
      
    }
  }
  
  Serial.print(F("I:\tDeactivate PDP: "));
  Serial.println(net.disconnect());
  
  Serial.print(F("I:\tSet APN and Password: "));
  Serial.println(net.configure(APN, USER, PASS));
  
  Serial.print(F("I:\tConnect to Internet: "));
  Serial.println(net.connect());
  
  Serial.print(F("I:\tConfigure DNS: "));
  Serial.println(net.configDNS("8.8.8.8", "9.9.9.9"));
  
  Serial.print(F("I:\tIP: "));
  Serial.println(net.getIP());

  // Enable SSL
  Serial.print(F("I:\tSet SSL Version: "));
  Serial.println(ssl.setSSLversion(SSL_CONTEXT_ID, TLS1_2)); // Set SSL version to TLS1.2
  Serial.print(F("I:\tSet SSL Cipher: "));
  Serial.println(ssl.setCiphersuite(SSL_CONTEXT_ID, CIPHER_ALL));  // Allow all cipher suites
  Serial.print(F("I:\tSet SSL seclevel: "));
  Serial.println(ssl.setSeclevel(SSL_CONTEXT_ID, SERVER_AUTH)); // Server authentication
  Serial.print(F("I:\tSet SLL Certificate: "));
  //Serial.println(ssl.setCertificate(SSL_CONTEXT_ID)); // Defaults to UFS:cacert.pem
  Serial.println(ssl.setCertificate(SSL_CONTEXT_ID, "UFS:cacert.pem"));
  Serial.print(F("I:\tSet SLL ignorelocaltime: "));
  Serial.println(ssl.setIgnorelocaltime(SSL_CONTEXT_ID, IGNORE_TIME_CHECK)); // IGNORE_TIME_CHECK or CARE_TIME_CHECK
  
  feedName.concat(MQTT_USER);
  feedName.concat("/feeds/testmqttssl");

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
    if (mqtt.disconnectMQTTServer()) {
      //mqtt.connectMQTTServer(MQTT_SERVER, MQTT_PORT);
      mqtt.connectMQTTServer(SSL_CONTEXT_ID, MQTT_SERVER, MQTT_PORT);
    }
    delay(500);
    Serial.println(mqtt.connectState());
  } while(!mqtt.connectState());
  Serial.println(F("I:\tMQTT Server Connected"));
  
  unsigned char ret = mqtt.connectMQTTUser(MQTT_ID, MQTT_USER, MQTT_PASSWORD);
  Serial.print(F("I:\tMQTT Server Login: "));
  Serial.println(mqtt.connectCodeString(ret));
  
  mqtt.subscribe(feedName);
  //mqtt.subscribe("time/seconds"); // Adafruit IO specific

  //Serial.println(mqtt.disconnectMQTTUser());
  //mqtt.unsubscribe("time/seconds");
} // connectToServer

//  ---------------------------------------------------------------------------
/// loop
//  ---------------------------------------------------------------------------
void loop() {
  // Publish to feed every interval
  if (millis() - previousmqtt >= intervalmqtt) {
    previousmqtt = millis();
    feedPayload = String(float(random(46000, 56000))*0.001);
    mqtt.publish(feedName, feedPayload);
  }
  unsigned long tick = millis();
  mqtt.mqttLoop();
  if (millis() - tick > 150) {
    Serial.print("Tick: ");
    Serial.println(millis() - tick);
  }
  if (!mqtt.connectState()) {
     connectToServer();
  }
} // loop

