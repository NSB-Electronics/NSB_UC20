#include "NSB_UC20.h"
//#include "SoftwareSerial.h"
//#include <AltSoftSerial.h>
#include "call.h"
#include "sms.h"

UC20 gsm;
CALL call;
SMS sms;
//SoftwareSerial mySerial(2, 3); // RX, TX
//AltSoftSerial mySerial;
void debug(String data){
  Serial.println(data);
}

void setup() {
  Serial.begin(9600);
  delay(3000);
  
  gsm.begin(&Serial1, 115200);
  //gsm.Event_debug = debug;
  Serial.println(F("I:\tInit Modem"));
//  if (gsm.waitOK()) {
//    Serial.println(F("UC20 Ready..."));
//    gsm.setEchoMode(false);
//  } else {
    // Modem not responding, hard reset
    Serial.println("I:\tSoftware Reset Modem");
    gsm.setAutoReset(1, 0); // For now doing soft reset
    //gsm.PowerOn();
    Serial.print("I:\tWait for Modem Ready: ");
    Serial.println(gsm.waitReady());
//  }

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

  Serial.println("I:\tSet Default SMS Settings");
  sms.defaultSettings();

  Serial.println("I:\tRequest Balances");
  sms.sendUSSD("*135*503#"); // Vodacom balances
}

void loop() {
 
  if (gsm.eventInput()) {
    if (gsm.eventType == EVENT_RING) {
      Serial.println("I:\t RING");
    }
    if (gsm.eventType == EVENT_SMS) {
      int index = sms.indexNewSMS();
      String str_sms = sms.readSMS(index);
      Serial.print("I:\tSMS: ");
      Serial.println(str_sms);
      Serial.print("I:\tFrom: ");
      Serial.println(sms.SMSInfo);
      //Serial.println(sms.convertStrUnicodeToTIS620(str_sms));
      sms.deleteSMS(index);
    }
  }

  if (Serial.available()) {
    char c = Serial.read();
    gsm.write(c);
  }
  
}
