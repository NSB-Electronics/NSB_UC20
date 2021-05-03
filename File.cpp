#include "File.h"

UC_FILE::UC_FILE() {
		
}

bool available_ = true;

void UC_FILE::begin() {
	
}

uint32_t UC_FILE::getFreeSpace(uint8_t pattern) {	
	bool responseOK = false;
	uint32_t freeSpace = 0;
	char cmd[20];
	
	if (pattern == FILE_UFS) {
		sprintf(cmd, "%s", CMD_FILE_SPACE_UFS);
	} else if (pattern == FILE_RAM) {
		sprintf(cmd, "%s", CMD_FILE_SPACE_RAM);
	} else {
		sprintf(cmd, "%s", CMD_FILE_SPACE_UFS);
	}
	
	responseOK = gsm.sendParseReply(cmd, REPLY_FILE_SPACE, &freeSpace, ',', 0);
	
	if (responseOK == false) {
		freeSpace = 0;
	}
	
	return freeSpace;
}

uint32_t UC_FILE::getTotalSpace(uint8_t pattern) {	
	bool responseOK = false;
	uint32_t totalSpace = 0;
	char cmd[20];
	
	if (pattern == FILE_UFS) {
		sprintf(cmd, "%s", CMD_FILE_SPACE_UFS);
	} else if (pattern == FILE_RAM) {
		sprintf(cmd, "%s", CMD_FILE_SPACE_RAM);
	} else {
		sprintf(cmd, "%s", CMD_FILE_SPACE_UFS);
	}
	
	responseOK = gsm.sendParseReply(cmd, REPLY_FILE_SPACE, &totalSpace, ',', 1);
	
	if (responseOK == false) {
		totalSpace = 0;
	}
	
	return totalSpace;
}

bool UC_FILE::deleteFile(char *filename, uint8_t pattern) {
	bool responseOK = false;
	char cmd[100]; // Max filename is 80bytes
	
	if (pattern == FILE_UFS) {
		sprintf(cmd, "%s=\"%s\"", CMD_FILE_DELETE, filename);
	} else if (pattern == FILE_RAM) {
		sprintf(cmd, "%s=RAM:\"%s\"", CMD_FILE_DELETE, filename);
	} else {
		sprintf(cmd, "%s=\"%s\"", CMD_FILE_DELETE, filename);
	}
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	return responseOK;
}

int32_t UC_FILE::open(char *filename, uint8_t pattern, uint8_t mode) {
	bool responseOK = false;
	uint32_t fileHandle = 0;
	char cmd[100]; // Max filename is 80bytes
	
	if (pattern == FILE_UFS) {
		sprintf(cmd, "%s=\"%s\",%d", CMD_FILE_OPEN, filename, mode);
	} else if (pattern == FILE_RAM) {
		sprintf(cmd, "%s=RAM:\"%s\",%d", CMD_FILE_OPEN, filename, mode);
	} else {
		sprintf(cmd, "%s=\"%s\",%d", CMD_FILE_OPEN, filename, mode);
	}
	
	responseOK = gsm.sendParseReply(cmd, REPLY_FILE_OPEN, &fileHandle, ' ', 0);
	
	if (responseOK == false) {
		fileHandle = -1;
	}
	
	return fileHandle;
	
}

bool UC_FILE::close(uint32_t handle) {
	bool responseOK = false;
	char cmd[20];
	
	sprintf(cmd, "%s=%Lu", CMD_FILE_CLOSE, handle);
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

uint32_t UC_FILE::read(uint32_t handle, char *response, uint16_t responseLen) {
	
	bool responseOK = false;
	uint32_t readLen = 0;
	char cmd[20];
	
	sprintf(cmd, "%s=%Lu,%Lu", CMD_FILE_READ, handle, responseLen-1); // Leave space for 0
	
	gsm.getReply(cmd);
	
	responseOK = gsm.parseReply(AVT1_CONNECT, &readLen, ' ');
  
	if (responseOK) {
		readLen = gsm.readRaw(readLen);
		char *p = gsm.replyBuffer;
		uint16_t lentocopy = min(readLen, (uint16_t)strlen(p));
		lentocopy = min(lentocopy, responseLen-1);
		strncpy(response, p, lentocopy);
		response[lentocopy] = 0;
		readLen = lentocopy;
		
		responseOK = gsm.checkReply(REPLY_OK); // eat OK
		
		if (!responseOK) {
			readLen = 0;
		}
	
	} else {
		readLen = 0;
	}
	
  return readLen;
}


bool UC_FILE::write(uint32_t handle, char *data, uint16_t dataLen) {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=%Lu,%d,%Lu", CMD_FILE_WRITE, handle, dataLen, TIMEOUT_DEF_MS);
	
	responseOK = gsm.sendCheckReply(cmd, AVT1_CONNECT);
  if (responseOK) {
		gsm.println(data);
		
		responseOK |= gsm.checkReply(REPLY_FILE_WRITE);
		
		responseOK |= gsm.checkReply(REPLY_OK); // read OK
	} else {
		// Command failed
	}
	

  return responseOK;
}

bool UC_FILE::seek(uint32_t handle, uint32_t offset, uint8_t position) {
	bool responseOK = false;
	char cmd[30];
	
	sprintf(cmd, "%s=%Lu,%Lu,%d", CMD_FILE_SEEK, handle, offset, position);
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	
	return responseOK;
}

uint32_t UC_FILE::getPosition(uint32_t handle) {
	bool responseOK = false;
	char cmd[20];
	uint32_t offset = 0;
	
	sprintf(cmd, "%s=%Lu", CMD_FILE_POSITION, handle);
	
	responseOK = gsm.sendParseReply(cmd, REPLY_FILE_POSITION, &offset, ' ', 0);
	
	if (!responseOK) {
		offset = 0;
	}
	return offset;
}

bool UC_FILE::flush(uint32_t handle) {
	bool responseOK = false;
	char cmd[20];
	uint32_t fileHandle = 0;
	
	sprintf(cmd, "%s=%Lu", CMD_FILE_FLUSH, handle);
	
	responseOK = gsm.sendParseReply(cmd, REPLY_FILE_FLUSH, &fileHandle, ' ', 0);
	return responseOK;
}

bool UC_FILE::truncate(uint32_t handle) {
	bool responseOK = false;
	char cmd[20];
	uint32_t fileHandle = 0;
	
	sprintf(cmd, "%s=%Lu", CMD_FILE_TUCAT, handle);
	
	responseOK = gsm.sendCheckReply(cmd, REPLY_OK);
	return responseOK;
}

uint32_t UC_FILE::list(char *filename, uint8_t pattern) {
	bool responseOK = false;
	bool responseFilesize = false;
	char cmd[100];
	uint32_t numFiles = 0;
	uint32_t filesize = 0;
	uint32_t filenameLen = strlen(filename);
	
	if (pattern == FILE_UFS) {
		sprintf(cmd, "%s=\"%s\"", CMD_FILE_LIST, filename);
	} else if (pattern == FILE_RAM) {
		sprintf(cmd, "%s=\"RAM:%s\"", CMD_FILE_LIST, filename);
	} else {
		sprintf(cmd, "%s=\"%s\"", CMD_FILE_LIST, filename);
	}
	responseOK = gsm.sendCheckReply(cmd, REPLY_FILE_LIST);
	if (responseOK) {
		numFiles += 1;
		responseOK = false;
		while (!responseOK) {
			responseFilesize = gsm.parseReply(REPLY_FILE_LIST, &filesize, ',', 1);
			responseOK = gsm.checkReply(REPLY_FILE_LIST);
			if (responseOK) {
				numFiles += 1;
				responseOK = false;
			} else {
				char *chReply;
				chReply = strstr(gsm.replyBuffer, REPLY_OK);
				if (chReply != NULL) {
					// Found OK
					responseOK = true;
				} else {
					// Error
					numFiles = 0;
					responseOK = true;
				}
			}
		}
	} else {
		numFiles = 0;
	}
	if ((numFiles > 0) && (filenameLen > 1) && (responseFilesize)) {
		numFiles = filesize;
	}
	return numFiles;
}

bool UC_FILE::getFileDetail(uint32_t fileIdx, char *filename, uint32_t filenameLen, uint32_t *filesize, uint8_t pattern) {
	bool responseOK = false;
	bool responseError = false;
	bool foundDetail = false;
	char cmd[20];
	uint32_t numFiles = 0;
	
	if (pattern == FILE_UFS) {
		sprintf(cmd, "%s=\"*\"", CMD_FILE_LIST);
	} else if (pattern == FILE_RAM) {
		sprintf(cmd, "%s=\"RAM:*\"", CMD_FILE_LIST);
	} else {
		sprintf(cmd, "%s=\"*\"", CMD_FILE_LIST);
	}
	responseOK = gsm.sendCheckReply(cmd, REPLY_FILE_LIST);
	if (responseOK) {
		numFiles += 1;
		while (responseOK && !responseError) {
			if (numFiles-1 == fileIdx) {
				// Extract filename and filesize
				responseOK = gsm.parseReplyQuoted(REPLY_FILE_LIST, filename, filenameLen, ',', 0);
				gsm.parseReply(REPLY_FILE_LIST, filesize, ',', 1);
				foundDetail = true;
			} else {
				// Continue
			}
			responseOK = gsm.checkReply(REPLY_FILE_LIST);
			if (responseOK) {
				numFiles += 1;
			} else {
				char *chReply;
				chReply = strstr(gsm.replyBuffer, REPLY_OK);
				if (chReply != NULL) {
					// Found OK
					responseOK = false; // Exit
				} else {
					// Error
					numFiles = 0;
					responseError = true;
				}
			}
		}
	} else {
		responseError = true;
	}
	
	return foundDetail;
}


bool UC_FILE::upload(char *filename, char* filePtr, uint16_t filesize, uint8_t pattern) {
	bool responseOK = false;
	char cmd[30];
	
	if (pattern == FILE_UFS) {
		sprintf(cmd, "%s=\"%s\",%d", CMD_FILE_UPLOAD, filename, filesize);
	} else if (pattern == FILE_RAM) {
		sprintf(cmd, "%s=\"RAM:%s\",%d", CMD_FILE_UPLOAD, filename, filesize);
	} else {
		sprintf(cmd, "%s=\"%s\",%d", CMD_FILE_UPLOAD, filename, filesize);
	}
	responseOK = gsm.sendCheckReply(cmd, AVT1_CONNECT);
	
  if (responseOK) {
		gsm.println(filePtr);
		
		responseOK |= gsm.checkReply(REPLY_FILE_UPLOAD);
		
		responseOK |= gsm.checkReply(REPLY_OK); // read OK
	} else {
		// Command failed
	}

  return responseOK;
}