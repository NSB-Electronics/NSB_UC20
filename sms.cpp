#include "sms.h"

SMS::SMS(){}


void SMS::defaultSettings() {
	// Set SMS message format as text mode
	gsm.println(F("AT+CMGF=1"));
	gsm.debug(F("AT+CMGF=1"));
	gsm.wait_ok(1000);
	
	// Set char set as GSM
	gsm.println(F("AT+CSCS=\"GSM\""));
	gsm.debug(F("AT+CSCS=\"GSM\""));
	gsm.wait_ok(1000);
	
	// Configure URC output to UART1
	gsm.println(F("AT+QURCCFG=\"URCPORT\",\"UART1\""));
	gsm.debug(F("AT+QURCCFG=\"URCPORT\",\"UART1\""));
	gsm.wait_ok(1000);
	
	// Read, write, receive SMS in SIM storage
	gsm.println(F("AT+CPMS=\"SM\",\"SM\",\"SM\""));
	gsm.debug(F("AT+CPMS=\"SM\",\"SM\",\"SM\""));
	gsm.wait_ok(1000);
	
	// Do not show header values
	gsm.println(F("AT+CSDH=0"));
	gsm.debug(F("AT+CSDH=0"));
	gsm.wait_ok(1000);
	
}

void SMS::start(String rx_number) {
	gsm.print(F("AT+CMGS=\""));
	gsm.print(rx_number);
	gsm.println(F("\""));
	
	// MAKE NON BLOCKING, ADD TIMEOUT
	while(!gsm.available()) {}
	String req = gsm.readStringUntil('>');
	
}

void SMS::send(String data) {
	gsm.print(data);

}

void SMS::sendln(String data) {
	send(data+"\r\n");
}

void SMS::stop() {
	gsm.write(0x1A);
	if (gsm.wait_ok(10000)) {
		gsm.debug("Send OK");
	}
}

unsigned char SMS::indexNewSMS() {
	return (gsm.indexNewSMS);
}

String SMS::readSMS(int index) {	
	gsm.print(F("AT+CMGR="));
	gsm.print(index, DEC);
	gsm.println("");
  String data;
	
	while (1) {
		if (gsm.available()) {
			String req = gsm.readStringUntil('\n');
			//gsm.debug(req);
			if (req.indexOf(F("+CMGR"))!= -1) {
				SMSInfo = req.substring(req.indexOf(",") + 1);
				//SMSInfo = req;
				//data+=req;
				//data+="\r\n";
			}	else if (req.indexOf(F("OK"))!= -1) {
				break;
			}	else {
				if ((req!="\r")&&(req!="\n")) {
					data+=req;
			  }
			}
		}
	}
	return (data);
}

bool SMS::sendUSSD(String USSDcode) {
	gsm.print(F("AT+CUSD=1,\""));
	gsm.print(USSDcode);
	gsm.println("\"");
	return (gsm.wait_ok(5000));
}

bool SMS::deleteSMS(int index) {
	gsm.print(F("AT+CMGD="));
	gsm.print(index,DEC);
	gsm.println("");
	return (gsm.wait_ok(10000));
}

String SMS::convertStrUnicodeToTIS620(String data) {
  int i=0;
  char c;
  String output="";
  unsigned char flag_th=0;
  while (data[i]) {
		if ((data[i]=='0')&&(data[i+1]=='E')) {
			flag_th=1;
		}	else {
	    output+=data[i];
			output+=data[i+1];
		}
		i+=2;  
		if (i>data.length())
			break;
	}
	if (!flag_th)
		return(output);
	else
		output="";
	
	i=0;
	while (data[i]) {
		if ((data[i]=='0')&&(data[i+1]=='0')) {
			c  = (data[i+2]-0x30)<<4;
			if ((data[i+3])>='A')
				c |= (data[i+3]-0x30)-0x07;
			else
				c |= (data[i+3]-0x30);
			output+=c;
		}
		if ((data[i]=='0')&&(data[i+1]=='E')) {
			c  = (data[i+2]-0x30)<<4;
			c += 0xA0;
			if ((data[i+3])>='A') {
				c |= (data[i+3]-0x30)-0x07;
			} else {
				c |= (data[i+3]-0x30);  
			}
			output+=c;
	   }
		i+=4;
		if (i>data.length())
			break;
	}
  return (output);
}

String SMS::convertStrUnicodeToUTF8(String data) {
	int i=0,j=0;
	String dat = convertStrUnicodeToTIS620(data);
	//Serial.println(dat);
	char out[dat.length()*3];
	while (dat[i]) {
		unsigned char c = dat[i];
		//Serial.print(c,HEX);
		if(c >= 0xA0)	{
			out[j] = 0xE0;
			j++;
			if(c >=0xE0) {
				out[j] = 0xB9;
				j++;
				out[j] = c-0x60;
				j++;
			}	else {
				out[j] = 0xB8;
				j++;
				out[j] = c-0x20;
				j++;
			}
		}	else {
			out[j] = c;
			j++;
		}
		
		i++;
	}
	out[j]=0;
//for(int x=0;x<j;x++)
//		Serial.print(out[x],HEX);
//Serial.println();
	return (String(out));	
}
