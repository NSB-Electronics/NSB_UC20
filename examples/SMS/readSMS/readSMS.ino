#include "NSB_UC20.h"
//#include "SoftwareSerial.h"
//#include <AltSoftSerial.h>
#include "call.h"
#include "sms.h"

UC20 gsm;
CALL call;
SMS sms;

#define STAT_LED_RED  3
#define STAT_LED_GRN  4

unsigned long flashLed = millis();

char smsBuffer[250];
char smsSender[20];

//SoftwareSerial mySerial(2, 3); // RX, TX
//AltSoftSerial mySerial;
void debug(String data){
  Serial.println(data);
}

void setup() {
  Serial.begin(9600);
  delay(3000);

  pinMode(STAT_LED_RED, OUTPUT);
  pinMode(STAT_LED_GRN, OUTPUT);
  digitalWrite(STAT_LED_RED, LOW);
  digitalWrite(STAT_LED_GRN, LOW);

  boolean cmdOK = false;
  char info[50];
  unsigned char rssi = 0;
  int dBm = 0;
  unsigned char quality = 0;
  
  gsm.begin(&Serial1, 115200);
  //gsm.Event_debug = debug;
  Serial.println(F("I,Init Modem"));
  cmdOK = gsm.setAutoReset(RESET_ONETIME, 0);
  Serial.print("I,Software Reset Modem,");
  Serial.println(cmdOK ? "OK": "ERROR");
  cmdOK = gsm.waitReady();
  Serial.print("I,Wait for Modem Ready,");
  Serial.println(cmdOK ? "OK": "ERROR");

  cmdOK = gsm.resetDefaults();
  Serial.print("I,Reset to Defaults,");
  Serial.println(cmdOK ? "OK": "ERROR");
  
  cmdOK = gsm.setEchoMode(false);
  Serial.print("I,Set Echo Off,");
  Serial.println(cmdOK ? "OK": "ERROR");
  
  gsm.moduleInfo(info, 50);
  Serial.print("I,Modem,");
  Serial.println(info);
  
  if (gsm.waitSIMReady()) {
    Serial.println(F("I,SIM Ready"));
  } else {
    gsm.getSIMStatus(info, sizeof(info));
    Serial.print(F("E,SIM Issue,"));
    Serial.println(gsm.getCMEerrorString(info));
  }

  cmdOK = gsm.getICCID(info, sizeof(info));
  Serial.print("I,ICCID,");
  Serial.println(info);
  
  if (gsm.waitNetworkRegistered()) {
    gsm.getOperator(info, sizeof(info));
    Serial.print(F("I,Network,"));
    Serial.println(info);
    delay(5000); // Give some time before getting signal quality
    rssi = gsm.signalQuality();
    dBm = gsm.signalQualitydBm(rssi);
    quality = gsm.signalQualityPercentage(rssi);
    Serial.print(F("I,Signal:\t"));
    Serial.print(rssi);
    Serial.print(F("(rssi), "));
    Serial.print(dBm);
    Serial.print(F("(dBm), "));
    Serial.print(quality);
    Serial.println(F("%"));
  } else {
    Serial.print(F("E,Network Registration Issue,"));
    Serial.println(gsm.getNetworkStatus());
  }
  
  Serial.println("I,Set Default SMS Settings");
  sms.setFormat(SMS_MODE_TXT); // Set SMS message format as text mode 
  sms.setCharSet(SMS_CHARSET_GSM);  // Set char set as GSM
  sms.configURC(SMS_URC_UART1); // Configure URC output to UART1
  sms.setMsgStorage(SMS_STORE_SM, SMS_STORE_SM, SMS_STORE_SM); // Read, write, receive SMS in SIM storage only
  //sms.showTextModeParam(false); // Do not show header values
  
  // Or call this
  //sms.defaultSettings();

  uint16_t smsUsed = sms.getStorageUsed(SMS_MEM1);
  uint16_t smsTotal = sms.getStorageTotal(SMS_MEM1);
  
  Serial.print("I,SMS Used Mem1,");
  Serial.println(smsUsed);
  Serial.print("I,SMS Total Mem1,");
  Serial.println(smsTotal);

  sms.deleteAllSMS();

  //Serial.println("I,Request Balances");
  char ussdMsg[] = "*135*503#"; // Vodacom balances
  sms.sendUSSD(ussdMsg, smsBuffer, sizeof(smsBuffer));
  Serial.print("I,USSD Reply,");
  Serial.println(smsBuffer);
}

void loop() {
  if (millis() - flashLed > 1000) {
    digitalWrite(STAT_LED_GRN, !digitalRead(STAT_LED_GRN));
    flashLed = millis();
  }
  
  if (gsm.eventInput()) {
    if (gsm.eventType == EVENT_RING) {
      Serial.println("I,RING");
    }
    if (gsm.eventType == EVENT_SMS) {
      int index = sms.indexNewSMS();
      bool smsOK = sms.getSMSSender(index, smsSender, sizeof(smsSender));
      if (smsOK) {
        Serial.print("I,SMS Sender,");
        Serial.println(smsSender);
      }
      uint16_t smsLen = sms.readSMS(index, smsBuffer, sizeof(smsBuffer));
      if (smsLen) {
        Serial.print("I,SMS Recv,");
        Serial.println(smsBuffer);
      }
      
      sms.deleteSMS(index);
    }
    gsm.eventType = EVENT_NONE;
  }

  if (Serial.available()) {
    char c = Serial.read();
    gsm.write(c);
  }
  
}
