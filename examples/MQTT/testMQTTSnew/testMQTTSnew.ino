#include "NSB_UC20.h"
//#include "SoftwareSerial.h"
//#include <AltSoftSerial.h>
#include "internet.h"
#include "mqtt.h"
#include "ssl.h"
#include <Adafruit_SleepyDog.h>
#include <TimeLib.h>

#define FORCE_REBOOT  1

#define RESET_WATCHDOG  1000

#define GSM_RST       12

#define APN "internet"
#define USER ""
#define PASS ""

#define SSL_CONTEXT_ID  1 

#define MQTT_SERVER     "io.adafruit.com"
#define MQTT_PORT       8883
#define MQTT_ID         "MQTTS_TEST"   // Used to identify the connection
#define MQTT_USER       "YOUR_USERNAME"
#define MQTT_PASSWORD   "YOUR_PASSWORD/KEY"


UC20 gsm;
INTERNET net;
MQTT mqtt;
SSL ssl;

unsigned long previousmqtt = 0;
unsigned long resetWatchdogTime = 0;
const long intervalmqtt = 60000;

//AltSoftSerial mySerial;

char feedName[50];
char feedPayload[100];
bool cmdOK = false;

//  ---------------------------------------------------------------------------
/// debugGSM
//  ---------------------------------------------------------------------------
void debugGSM(String data) {
  // Forwards GSM debug messages
  Serial.println(data);
} // debugGSM

void watchdogReset() {
  Watchdog.reset();
}

//  ---------------------------------------------------------------------------
/// Setup
//  ---------------------------------------------------------------------------
void setup() {
  Watchdog.enable(8000);
  
  char info[50];
  char simStatus[20];
  Serial.begin(9600);
  delay(3000);

  randomSeed(analogRead(0));

  watchdogReset();
  
  gsm.setWatchdog(watchdogReset);
  gsm.begin(&Serial1, 115200);
  
  //gsm.Event_debug = debugGSM; // Enable debug messages
  
  Serial.println(F("I:\tInit Modem"));
  #if FORCE_REBOOT
    cmdOK = gsm.hardwareReset(GSM_RST); // Hard reset
    Serial.print("I:\tHardware Reset Modem: ");
    Serial.println(cmdOK ? "OK": "ERROR");
  #else
    if (gsm.waitOK()) {
      Serial.println(F("I:\tUC20 Ready..."));
      gsm.setEchoMode(false);
    } else {
      // Modem not responding, hard reset
      cmdOK = gsm.setAutoReset(RESET_ONETIME, 0);
      Serial.print("I:\tSoftware Reset Modem: ");
      Serial.println(cmdOK ? "OK": "ERROR");
      //gsm.setPowerKeyPin(UC20_PWR_PIN);
      //gsm.PowerOn();
      cmdOK = gsm.waitReady();
      Serial.print("I:\tModem Ready: ");
      Serial.println(cmdOK ? "OK": "ERROR");
    }
  #endif
  
  cmdOK = gsm.resetDefaults();
  Serial.print("I:\tReset to Defaults: ");
  Serial.println(cmdOK ? "OK": "ERROR");
  
  cmdOK = gsm.setEchoMode(false); // Echo must be off else other functions will not work
  Serial.print("I:\tSet Echo Off: ");
  Serial.println(cmdOK ? "OK": "ERROR");
  
  gsm.moduleInfo(info, sizeof(info));
  Serial.print("I:\tModem:\t");
  Serial.println(info);

  cmdOK = gsm.waitSIMReady();
  Serial.print(F("I:\tSIM Ready: "));
  Serial.println(cmdOK ? "OK": "ERROR");
    
  if (cmdOK) {
    Serial.println(F("I:\tSIM Ready"));
  } else {
    gsm.getSIMStatus(simStatus, sizeof(simStatus));
    Serial.print(F("E:\tSIM Issue: "));
    Serial.println(gsm.getCMEerrorString(simStatus));
  }
  
  cmdOK = gsm.waitNetworkRegistered();
  if (cmdOK) {
    gsm.getOperator(info, sizeof(info));
    Serial.print(F("I:\tNetwork: "));
    Serial.println(info);
    watchdogReset();
    delay(5000); // Give some time before getting signal quality
    watchdogReset();
    unsigned char rssi = gsm.signalQuality();
    int dBm = gsm.signalQualitydBm(rssi);
    unsigned char  quality = gsm.signalQualityPercentage(rssi);
    Serial.print(F("I:\tSignal:\t"));
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
  
  cmdOK = net.disconnect(SSL_CONTEXT_ID);
  Serial.print(F("I:\tDeactivate PDP: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  
  cmdOK = net.configure(SSL_CONTEXT_ID, APN, USER, PASS);
  Serial.print(F("I:\tSet APN and Password: "));
  Serial.println(cmdOK ? "OK": "ERROR");

  cmdOK = net.connect(SSL_CONTEXT_ID);
  Serial.print(F("I:\tConnect to Internet: "));
  Serial.println(cmdOK ? "OK": "ERROR");

  cmdOK = net.configDNS("8.8.8.8", "9.9.9.9");
  Serial.print(F("I:\tConfigure DNS: "));
  Serial.println(cmdOK ? "OK": "ERROR");

  char ipAddr[20];
  net.getIP(ipAddr, sizeof(ipAddr));
  Serial.print(F("I:\tIP: "));
  Serial.println(ipAddr);

  // Enable SSL
  cmdOK = ssl.setSSLversion(SSL_CONTEXT_ID, TLS1_2); // Set SSL version to TLS1.0
  Serial.print(F("I:\tSet SSL Version: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = ssl.setCiphersuite(SSL_CONTEXT_ID, CIPHER_ALL); // Allow all cipher suites
  Serial.print(F("I:\tSet SSL Cipher: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = ssl.setSeclevel(SSL_CONTEXT_ID, NO_AUTH); // Server authentication
  Serial.print(F("I:\tSet SSL seclevel: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = ssl.setCertificate(SSL_CONTEXT_ID, "cacert.pem");
  Serial.print(F("I:\tSet SLL Certificate: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = ssl.setIgnorelocaltime(SSL_CONTEXT_ID, IGNORE_TIME_CHECK); // IGNORE_TIME_CHECK or CARE_TIME_CHECK
  Serial.print(F("I:\tSet SLL ignorelocaltime: "));
  Serial.println(cmdOK ? "OK": "ERROR");

  cmdOK = mqtt.configVersion(MQTT_VERSION_3_1_1);
  Serial.print(F("I:\tConfig MQTT Version: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = mqtt.configPDP();
  Serial.print(F("I:\tConfig MQTT PDP: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = mqtt.configSSL(MQTT_SSL_TCP, SSL_CONTEXT_ID);
  Serial.print(F("I:\tConfig MQTT SSL: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = mqtt.configKeepAlive(120);
  Serial.print(F("I:\tConfig MQTT KeepAlive: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = mqtt.configSession(MQTT_CLEAN_SESSION);
  Serial.print(F("I:\tConfig MQTT Session: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = mqtt.configTimeout(10, 3, MQTT_TIMEOUT_IGNORE); // 10sec, 3 retries
  Serial.print(F("I:\tConfig MQTT Timeout: "));
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = mqtt.configRecvMode(MQTT_RECV_CONTAIN);
  Serial.print(F("I:\tConfig MQTT Recv Mode: "));
  Serial.println(cmdOK ? "OK": "ERROR");

  connectToMQTTServer();
  
  
  //cmdOK = mqtt.unsubscribe(feedName, 4);
  //Serial.print(F("I:\tUnsubscribe from topic: "));
  //Serial.println(cmdOK ? "OK": "ERROR");

//  delay(5000);
//  
//  cmdOK = mqtt.disconnectClient();
//  Serial.print(F("I:\tDisconnect Client: "));
//  Serial.println(cmdOK ? "OK": "ERROR");
//  
//  cmdOK = mqtt.close();
//  Serial.print(F("I:\tClose MQTT Connection: "));
//  Serial.println(cmdOK ? "OK": "ERROR");

} //setup

//  ---------------------------------------------------------------------------
/// connectToMQTTServer
//  ---------------------------------------------------------------------------
void connectToMQTTServer() {
  
  int8_t mqttResult = -1;
  mqttResult = mqtt.open(MQTT_SERVER, MQTT_PORT);
  Serial.print(F("I:\tOpen MQTT Connection: "));
  Serial.println(mqtt.getOpenResultString(mqttResult));
  
  mqttResult = mqtt.connectClient(MQTT_ID, MQTT_USER, MQTT_PASSWORD);
  Serial.print(F("I:\tConnect Client: "));
  Serial.println(mqtt.getConnectResultString(mqttResult));

  snprintf(feedName, sizeof(feedName), "%s/feeds/testmqttsub", MQTT_USER);
  
  //cmdOK = mqtt.subscribe(feedName, 4);
  cmdOK = mqtt.subscribe("time/seconds"); // Adafruit IO specific
  Serial.print(F("I:\tSubscribe to topic: "));
  Serial.println(cmdOK ? "OK": "ERROR");
} // connectToMQTTServer

//  ---------------------------------------------------------------------------
/// checkMQTTPacket
//  ---------------------------------------------------------------------------
void checkMQTTPacket() {
  char topic[100];
  char message[500];
  
  mqtt.readRecvPacket(topic, sizeof(topic), message, sizeof(message));
  
  Serial.print("I:\tTopic: ");
  Serial.println(topic);
  Serial.print("I:\tMessage: ");
  Serial.println(message);
} // checkMQTTPacket

//  ---------------------------------------------------------------------------
/// loop
//  ---------------------------------------------------------------------------
void loop() {
  if (millis() - resetWatchdogTime > RESET_WATCHDOG) {
    resetWatchdogTime = millis();
    watchdogReset();
  }
  
  // Publish to feed every interval
  if (millis() - previousmqtt >= intervalmqtt) {
    previousmqtt = millis();
    if (mqtt.connected()) {
      snprintf(feedName, sizeof(feedName), "%s/feeds/testmqttssl", MQTT_USER);
      snprintf(feedPayload, sizeof(feedPayload), "%5.2f", float(random(46000, 56000))*0.001);
      cmdOK = mqtt.publish(feedName, feedPayload);
      Serial.print(F("I:\tPublish to topic: "));
      Serial.println(cmdOK ? "OK": "ERROR");
    } else {
      Serial.println("E:\tMQTT not connected");
      connectToMQTTServer();
    }

    unsigned long upTime;
    unsigned long mySeconds;
    unsigned long myMinutes;
    unsigned long myHours;
    unsigned long myDays;
    upTime = now();
    mySeconds = upTime%60;
    myMinutes = (upTime/60)%60;
    myHours = (upTime/3600)%24;
    myDays = upTime/86400;
    Serial.print("Uptime:");
    Serial.print(myDays);  Serial.print("d:");
    Serial.print(myHours);  Serial.print("h:");
    Serial.print(myMinutes);  Serial.print("m:");
    Serial.print(mySeconds);  Serial.println("s");
  }
//  unsigned long tick = millis();
//  mqtt.mqttLoop();
//  if (millis() - tick > 150) {
//    Serial.print("Tick: ");
//    Serial.println(millis() - tick);
//  }
//  if (!mqtt.connectState()) {
//     connectToServer();
//  }
  switch (gsm.checkURC()) {
    case EVENT_MQTT_RECV:
      checkMQTTPacket();
      break;
    default:
      break;
  }
  
} // loop
