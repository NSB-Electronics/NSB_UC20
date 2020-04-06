#include "NSB_UC20.h"
//#include "SoftwareSerial.h"
//#include <AltSoftSerial.h>
#include "internet.h"
#include "ssl.h"
#include "smtp.h"

INTERNET net;
UC20 gsm;
SSL ssl;
SMTP smtp;
//SoftwareSerial mySerial(8, 9); // RX, TX
//AltSoftSerial mySerial;

#define APN "internet"
#define USER ""
#define PASS ""

#define SSL_CONTEXT_ID  1 

#define SERVER    "nsb-electronics.co.za"
#define PORT      465 // SSL
//#define PORT      587 // TSL
#define USERNAME  "iot@nsb-electronics.co.za"
#define PASSWORD  "i0t@nsb&mail"

#define SENDER_NAME "NSB Electronics"
#define SENDER_EMAIL "iot@nsb-electronics.co.za"
#define SUBJECT "Test Send Email via UC20"

#define RECIPIENT_1 "barnes.ns@gmail.com"


void debug(String data) {
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
  Serial.println(ssl.setSeclevel(SSL_CONTEXT_ID, NO_AUTH)); // No authentication

  
  // Send email
  Serial.print(F("I:\tSet SMTP contextid: "));
  Serial.println(smtp.setContextID());
  Serial.print(F("I:\tSet SMTP SSL type: "));
  Serial.println(smtp.setSSLtype(SMTP_SSL));
  Serial.print(F("I:\tSet SSL context for SMTP: "));
  Serial.println(smtp.setSSLctxID(SSL_CONTEXT_ID));
  Serial.print(F("I:\tConfigure Email server: "));
  Serial.println(smtp.configServer(SERVER, PORT));
  Serial.print(F("I:\tSet username and password: "));
  Serial.println(smtp.setAccount(USERNAME, PASSWORD));
  Serial.print(F("I:\tSet sender: "));
  Serial.println(smtp.setSender(SENDER_NAME, SENDER_EMAIL));
  Serial.print(F("I:\tAdd recipient: "));
  Serial.println(smtp.addRecipient(RECIPIENT_1));

  Serial.print(F("I:\tSet subject: "));
  Serial.println(smtp.setSubject(SUBJECT));
  
  Serial.print(F("I:\tAdd body: "));
  Serial.println(smtp.startBody());
  smtp.addBodyln("Hello");
  smtp.addBodyln("We are testing if we can send an email via UC20");
  Serial.print(F("I:\tEnd body: "));
  Serial.println(smtp.stopBody());

  Serial.print(F("I:\tSend email: "));
  Serial.println(smtp.sendEmail());
}

void loop() {
  if (gsm.available()) {
    Serial.print(gsm.read());
  } 
  if (Serial.available()) {
    Serial.print(".");
    gsm.write(Serial.read());
  }
}
