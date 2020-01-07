#include "NSB_UC20.h"
//#include "SoftwareSerial.h"
//#include <AltSoftSerial.h>
#include "call.h"
#include "sms.h"

UC20 gsm;
CALL call;
SMS sms;
String phone_number = "0828677338";
//SoftwareSerial mySerial(8, 9); // RX, TX
//AltSoftSerial mySerial;

void debug(String data) {
  Serial.println(data);
}

void setup() {
  Serial.begin(9600);
  delay(3000);
  
  gsm.begin(&Serial1, 115200);
  gsm.Event_debug = debug;
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
  
  
//  Serial.println("Send SMS");
  sms.defaultSettings();
//  sms.start(phone_number);  
//  sms.sendln("Hello");
//  sms.sendln("From NSB GSM Test");
//  sms.stop();
//  Serial.println("END"); 
 
}

void loop() {
  if (gsm.available()) {
    Serial.write(gsm.read());
  } 
  if (Serial.available()) {
    char c = Serial.read();
    gsm.write(c);
  }
}
