#include "call.h"

CALL:: CALL(){}

unsigned char CALL:: Call(String call_number)  /* return 	0 =	Timeout
										return  1 = OK
										return  2 = NO CARRIER
										return  3 = BUSY
									*/
{
	gsm.print(F("ATD"));
	gsm.print(call_number);
	gsm.println(F(";"));
	while(!gsm.available())
	{}
	unsigned long timeout = millis();
	while(1)
	{
		String req = gsm.readStringUntil('\n');	
	    if(req.indexOf(F("OK")) != -1)
		{
			//DEBUG_PRINTLN(F("OK"));
			return(1);
		}
			
		if(req.indexOf(F("NO CARRIER")) != -1)
		{
			DEBUG_PRINTLN(F("\t---- CALL NO CARRIER"));
			return(2);
		}
		if(req.indexOf(F("BUSY")) != -1)
		{
			DEBUG_PRINTLN(F("\t---- CALL BUSY"));
			return(2);
		}
			
		if(millis() - timeout > 10000)
		{
			DEBUG_PRINTLN(F("\t---- CALL Time out Error"));
			return(0);
		}
	}
	return(0);
}

bool CALL:: Answer()
{
	gsm.println(F("ATA"));
	return(gsm.waitOK(3000));
}
bool CALL:: DisconnectExisting()
{
	gsm.println(F("ATH"));
	return(gsm.waitOK(3000));
}
bool CALL:: HangUp()
{
	gsm.println(F("AT+CHUP"));
	return(gsm.waitOK(3000));
}
String CALL:: CurrentCallsMe()
{
	gsm.println(F("AT+CLCC"));
	while(!gsm.available())
	{}
	unsigned long timeout = millis();
	while(1)
	{
		String req = gsm.readStringUntil('\n');	
	   
	   if(req.indexOf(F("+CLCC")) != -1)
		{
			char index1 = req.indexOf(F("\""));
			char index2 = req.indexOf(F("\""),index1+1);
			return(req.substring(index1+1,index2));
			//Serial.println("req");
			return(req);
		}
		if(millis() - timeout > 20000)
		{
			return(F("CurrentCallsMe Timeout"));
		}
	}
	return(F(" "));	
} 
bool CALL::WaitRing()
{
	while(gsm.available())
	{
		String req = gsm.readStringUntil('\n');	
	   
	   if(req.indexOf(F("RING")) != -1)
		{
			return(true);
		}	
	}
	return(false);
}










