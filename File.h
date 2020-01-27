#ifndef FILE_h
#define FILE_h

#include "NSB_UC20.h"

class UC_FILE {
public:
	UC_FILE();
	void begin();
	long GetSpace(String pattern);
	long GetFreeSpace(String pattern);
	void List(String pattern);
	long List(String pattern, String filename);
	bool available();
	String ReadLine();
	bool Delete(String pattern, String filename);
	int Open(String pattern, String filename);
	bool Close(int handle);
	bool Close_(int handle);
	bool BeginWrite(int handle, int size);
	void Write(char data);
	void Print(String data);
	void Println(String data);
	bool WaitFinish();
	bool Seek(int handle, long start_at);
	bool SeekAtStart(int handle);
	int  Read(int handle,int buf_size, char *buf);
	void (*ListOutput)(String data);
	void ReadFile(String pattern, String filename);
	void (*DataOutput)(char data);
	bool upload(String pattern, String filename, int filesize, char* filePtr);
private:
	long space(String pattern, unsigned char mode);	
};

#endif 