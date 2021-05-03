#include "NSB_UC20.h"
#include "File.h"
UC20 gsm;
UC_FILE file;

char filename[80]; // Maximum filename length supported

void debug(String data)
{
  Serial.println(data);
}
void data_out(char data)
{
  Serial.write(data);
}

void setup() {
  boolean cmdOK = false;
  char info[50];

  Serial.begin(9600);
  gsm.begin(&Serial1, 115200);
  delay(3000);
  Serial.println("Start");
  //  gsm.Event_debug = debug;
  Serial.println(F("I,Init Modem"));
  cmdOK = gsm.setAutoReset(RESET_ONETIME, 0);
  Serial.print("I,Software Reset Modem,");
  Serial.println(cmdOK ? "OK" : "ERROR");
  cmdOK = gsm.waitReady();
  Serial.print("I,Wait for Modem Ready,");
  Serial.println(cmdOK ? "OK" : "ERROR");

  cmdOK = gsm.resetDefaults();
  Serial.print("I,Reset to Defaults,");
  Serial.println(cmdOK ? "OK" : "ERROR");

  cmdOK = gsm.setEchoMode(false);
  Serial.print("I,Set Echo Off,");
  Serial.println(cmdOK ? "OK" : "ERROR");

  gsm.moduleInfo(info, 50);
  Serial.print("I,Modem,");
  Serial.println(info);

  file.begin();
  get_space();

  // filename = "*" deletes all files
  sprintf(filename, "%s", "test.txt");
  cmdOK = file.deleteFile(filename, FILE_UFS);
  Serial.print("I,Delete File,");
  Serial.println(cmdOK ? "OK": "ERROR");

  sprintf(filename, "%s", "test.txt");
  uint32_t handle = file.open(filename);
  Serial.print("I,Open File,");
  Serial.println(handle);

  char fileResponse[255];
  sprintf(fileResponse, "%s", "Testing 123.\n\rIs newline working?");
  cmdOK = file.write(handle, fileResponse, strlen(fileResponse));
  Serial.print("I,Write File,");
  Serial.println(cmdOK ? "OK" : "ERROR");

  cmdOK = file.flush(handle);
  Serial.print("I,Flush File,");
  Serial.println(cmdOK ? "OK" : "ERROR");
  
  cmdOK = file.close(handle);
  Serial.print("I,Close File,");
  Serial.println(cmdOK ? "OK" : "ERROR");

  sprintf(filename, "%s", "logs.txt");
  handle = file.open(filename);
  Serial.print("I,Open File,");
  Serial.println(handle);

  cmdOK = file.seek(handle, 100);
  Serial.print("I,Seek File,");
  Serial.println(cmdOK ? "OK" : "ERROR");

  uint32_t offset = file.getPosition(handle);
  Serial.print("I,File Offset,");
  Serial.println(offset);

  
  uint32_t len = file.read(handle, fileResponse, sizeof(fileResponse));
  Serial.print("I,Read File,");
  Serial.println(len);

  Serial.print("I,File Content,");
  Serial.println(fileResponse);
  
  cmdOK = file.close(handle);
  Serial.print("I,Close File,");
  Serial.println(cmdOK ? "OK" : "ERROR");


  uint32_t numFiles = file.list("*"); // Check how many files
  Serial.print("I,Num Files,");
  Serial.println(numFiles);

  uint32_t filesize = file.list("test.txt"); // Check file size
  Serial.print("I,Filesize,");
  Serial.println(filesize);

  char fileNameFound[80];
  cmdOK = file.getFileDetail(0, fileNameFound, sizeof(fileNameFound), &filesize);
  Serial.print("I,File name,");
  Serial.println(fileNameFound);
  Serial.print("I,File Size,");
  Serial.println(filesize);
  
}

void get_space()
{
  uint32_t space_size = file.getTotalSpace();
  Serial.print("Get All Space = ");
  Serial.println(space_size);
  Serial.print("Get Free Space = ");
  space_size = file.getFreeSpace();
  Serial.println(space_size);
}

void loop()
{
  if (gsm.available())
  {
    Serial.write(gsm.read());
  }
  if (Serial.available())
  {
    char c = Serial.read();
    gsm.write(c);
  }
}
