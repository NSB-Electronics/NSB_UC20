#ifndef FILE_h
#define FILE_h

#include "NSB_UC20.h"

class UC_FILE {
public:
	UC_FILE();
	void begin();
	long getSpace(String pattern);
	long getFreeSpace(String pattern);
	void list(String pattern);
	long list(String pattern, String filename);
	bool available();
	String readLine();
	bool deleteFile(String pattern, String filename);
	int open(String pattern, String filename);
	bool close(int handle);
	bool close_(int handle);
	bool beginWrite(int handle, int size);
	void write(char data);
	void print(String data);
	void println(String data);
	bool waitFinish();
	bool seek(int handle, long offset, char position);
	bool seekAtStart(int handle);
	bool seekAtEnd(int handle);
	int  read(int handle,int buf_size, char *buf);
	void (*listOutput)(String data);
	void readFile(String pattern, String filename);
	void (*dataOutput)(char data);
	bool upload(String pattern, String filename, int filesize, char* filePtr);
	bool flush(int handle);
private:
	long space(String pattern, unsigned char mode);	
};

#endif 