#ifndef TCP_h
#define TCP_h

#include "NSB_UC20.h"


class TCP
{
public:
	TCP();
	bool Open(unsigned char contextid,unsigned char connectid,String service_type,String ip_url,String remote_port,String local_port,unsigned char access_mode);
	bool Open(String ip_url,String port);
	bool StartSend(unsigned char contextid);
	bool StartSend(unsigned char contextid,int len);
	bool StartSend();
	bool StopSend();
	bool WaitSendFinish();
	bool ReceiveAvailable();
	int  ReadBuffer();
	int  ReadBuffer(unsigned char contextid,int max_len);
	int  ReadBuffer(int max_len);
	void Ping(unsigned char contextid,String ip_url);
	void write(char data);
	void print(String data);
	void println(String data);
	void print(int data);
	void println(int data);
	bool Close(unsigned char contextid);
	bool Close();
	bool CheckConnection(unsigned char query_type , unsigned char contextid );
	bool CheckConnection();
	String NTP(unsigned char contextid,String ip_url,String port);
	
	unsigned char ReceiveConnectID;
	
};

#endif