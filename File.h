#ifndef FILE_h
#define FILE_h

#define FILE_UFS 	0
#define FILE_RAM 	1

#define FILE_BEGIN		 0
#define FILE_CURR      1
#define FILE_END			 2

#include "NSB_UC20.h"

class UC_FILE {
public:
	UC_FILE();
	void begin();
	uint32_t getFreeSpace(uint8_t pattern = FILE_UFS);
	uint32_t getTotalSpace(uint8_t pattern = FILE_UFS);
	bool deleteFile(char *filename, uint8_t pattern = FILE_UFS);
	int32_t open(char *filename, uint8_t pattern = FILE_UFS, uint8_t mode = 0);
	bool close(uint32_t handle);
	uint32_t read(uint32_t handle, char *response, uint16_t responseLen = MAX_REPLY_LEN);
	bool write(uint32_t handle, char *data, uint16_t dataLen);
	bool seek(uint32_t handle, uint32_t offset = 0, uint8_t position = FILE_BEGIN);
	uint32_t getPosition(uint32_t handle);
	bool flush(uint32_t handle);
	bool truncate(uint32_t handle);
	
	uint32_t list(char *filename, uint8_t pattern = FILE_UFS);
	bool getFileDetail(uint32_t fileIdx, char *filename, uint32_t filenameLen, uint32_t *filesize, uint8_t pattern = FILE_UFS);
	
	bool upload(char *filename, char* filePtr, uint16_t filesize, uint8_t pattern = FILE_UFS);
	//bool upload(String pattern, String filename, int filesize, char* filePtr);
	
private:
	
};

#endif 