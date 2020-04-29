#include "File.h"

void func_null(String data){}
void func_null(char data){}

UC_FILE::UC_FILE() {
		
}

bool available_ = true;

void UC_FILE::begin() {
	listOutput = func_null;
	dataOutput = func_null;
}

long UC_FILE::getSpace(String pattern) {
	return (space(pattern ,0));
}

long UC_FILE::getFreeSpace(String pattern) {
	return (space(pattern ,1));
} 

long UC_FILE::space(String pattern, unsigned char mode) {//Mode 0 = All Space,1 = Free Space
	char arr[20];
	gsm.print(F("AT+QFLDS="));
	gsm.println("\""+pattern+"\"");
	while(!gsm.available()) {
		
	}
	gsm.start_time_out();
	while(1) {
		String req = gsm.readStringUntil('\n');	
	   
		if (req.indexOf(F("+QFLDS:")) != -1) {
			char index1,index2;
			if (mode) { //FreeSpace
				index1 = req.indexOf(F(" "));
				index2 = req.indexOf(F(","));
				String str_l =req.substring(index1+1,index2);
				str_l.toCharArray(arr,sizeof(arr));	
			} else { //Space
				char index1 = req.indexOf(F(","));
				String str_l =req.substring(index1+1);
				str_l.toCharArray(arr,sizeof(arr));
			}			
			gsm.wait_ok(1000);
			return (atol(arr));
		}
		if (gsm.time_out(20000)) {
			return (-1);
		}
	}
	return (-1);
}

void UC_FILE::list(String pattern) {
	//AT+QFLST="*"
	//AT+QFLST="RAM:*"
	if (pattern == UFS)
	gsm.println(F("AT+QFLST=\"*\""));
	if (pattern == RAM)
	gsm.println(F("AT+QFLST=\"RAM:*\""));	
	while (!gsm.available()) {
		
	}
	available_=true;
	
	while (1) {
		String req = gsm.readStringUntil('\n');	
	   
	    if (req.indexOf(F("OK")) != -1) {
			return;
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return;
		}
		(*listOutput)(req);
	}
	
	
}

long UC_FILE::list(String pattern, String filename) {
	char arr[20];
	if (pattern == UFS) {
		gsm.print(F("AT+QFLST=\""));
		gsm.print(filename);	   
		gsm.println(F("\""));	
	}
	if (pattern == RAM) {
		//AT+QFLST="RAM:1"
		gsm.print(F("AT+QFLST=\"RAM:"));
		gsm.print(filename);
		gsm.println("\"");
	}
	//+QFLST: "RAM:1",0
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	while (1) {
		String req = gsm.readStringUntil('\n');	
	   
	    if (req.indexOf(F("+QFLST:")) != -1) {
			char index1 = req.indexOf(F(","));
			String str_l =req.substring(index1+1);
			str_l.toCharArray(arr,sizeof(arr));
			gsm.wait_ok(1000);
			return (atol(arr));
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (-1);
		}		
	}
	return (-1);

}

bool UC_FILE::available() {
	return (available_);
}

String UC_FILE::readLine() {
	String req = gsm.readStringUntil('\n');
	if (req.indexOf(F("OK")) != -1) {
		available_ = false;
	}
	(*listOutput)(req);
	return (req);	
}

bool UC_FILE::deleteFile(String pattern, String filename) {
	gsm.print(F("AT+QFDEL=\""));
	if (pattern == UFS) {
		gsm.print(filename);
	}
	
	if (pattern == RAM) {
		gsm.print(F("RAM:"));
		gsm.print(filename);
	}
	gsm.println(F("\""));
	gsm.wait_ok(5000);
}

int UC_FILE::open(String pattern, String filename) {
	gsm.print(F("AT+QFOPEN=\""));
	if (pattern == UFS) {
		gsm.print(filename);
	}
	if (pattern == RAM) {
		gsm.print(F("RAM:"));
		gsm.print(filename);
	}
	gsm.println(F("\","));
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	while (1) {
		String req = gsm.readStringUntil('\n');	
	   
	    if (req.indexOf(F("+QFOPEN:")) != -1) {
			char index1 = req.indexOf(F(" "));
			char index2 = req.indexOf(F("\r\n"));
			String str = req.substring(index1+1,index2);
			//Serial.print("OK xxx");
			//Serial.print(str);
			gsm.wait_ok(1000);
			return (str.toInt());
		}
		if (req.indexOf(F("ERROR")) != -1) {
			Serial.print("Error");
			return (-1);
		}
		
	}

}

bool UC_FILE::close(int handle) {
	gsm.print(F("AT+QFCLOSE="));
	gsm.print(handle,DEC);
	gsm.println("");
	gsm.wait_ok(5000);
}

bool UC_FILE::close_(int handle) {
	gsm.print(F("AT+QFCLOSE="));
	gsm.print(handle,DEC);
	gsm.println("");
	gsm.wait_ok_ndb(5000);
}

bool UC_FILE::beginWrite(int handle,int size) {
	//AT+QFWRITE=0,10
	gsm.print(F("AT+QFWRITE="));
	gsm.print(handle,DEC);
	gsm.print(F(","));
	gsm.print(size,DEC);
	gsm.println("");
	
	while (!gsm.available()) {
		
	}
	gsm.start_time_out();
	while (1) {
		String req = gsm.readStringUntil('\n');	
	   
	    if (req.indexOf(F("CONNECT")) != -1) {
			return (true);
		}
		if (req.indexOf(F("ERROR")) != -1) {
			return (false);
		}		
	}

	//return (gsm.wait_ok(5000));
}

void UC_FILE::write(char data) {
	gsm.write(data);
}

void UC_FILE::print(String data) {
	gsm.print(data);
}

void UC_FILE::println(String data) {
	gsm.println(data);
}

bool UC_FILE::waitFinish() {
	return (gsm.wait_ok(10000));
}

bool UC_FILE::seekAtStart(int handle) {
	return (seek(handle, 0, 0));
}

bool UC_FILE::seekAtEnd(int handle) {
	return (seek(handle, 0, 2));
}

bool UC_FILE::seek(int handle, long offset, char position) {
	//AT+QFSEEK=0,0,0
	gsm.print(F("AT+QFSEEK="));
	gsm.print(handle,DEC);
	gsm.print(F(","));
	gsm.print(offset,DEC);
	gsm.print(F(","));
	gsm.print(position,DEC);
	gsm.println("");
	return (gsm.wait_ok(1000));
}

int UC_FILE::read(int handle,int buf_size, char *buf) {
	//AT+QFREAD=0,10
	int size;
	gsm.print(F("AT+QFREAD="));
	gsm.print(handle,DEC);
	gsm.print(F(","));
	gsm.print(buf_size,DEC);
	gsm.println("");
	while (!gsm.available()) {
		
	}
	unsigned char flag=1;
	while (flag) {
		String req = gsm.readStringUntil('\n');	
	   
	    if (req.indexOf(F("CONNECT")) != -1) {
			char index1 = req.indexOf(F(" "));
			char index2 = req.indexOf(F("\r\n"));
			String str =req.substring(index1+1,index2);
			//Serial.println("size = "+str);
			size=str.toInt();
			
			flag = 0;
		}
		if (req.indexOf(F("ERROR")) != -1)
			return(-1);
	}
	flag = 1;
	int cnt = 0;
	while (flag) {
		if (gsm.available()) {
			char c = gsm.read();
			buf[cnt] = c;
			//Serial.write(c);
			cnt++;
			if (cnt>=size) {
				flag=0;
			}
		}
	}
	gsm.wait_ok_ndb(1000);
	return (size);
}

void UC_FILE::readFile(String pattern, String filename) {
	int handle = open(pattern,filename);
	if (handle!=-1) {
		seekAtStart(handle);
		int buf_size=100;
		char buf[buf_size];
		int size_ = read(handle,buf_size,buf);
		while (size_!=-1) {
			for (int i=0;i<size_;i++) {
				//Serial.write(buf[i]);
				(*dataOutput)(buf[i]);
			}
			size_ = read(handle,buf_size,buf);
		}
   }
   close_(handle);
}


bool UC_FILE::upload(String pattern, String filename, int filesize, char* filePtr) {
	unsigned char flag=1;
	gsm.print(F("AT+QFUPL=\""));
	if (pattern == RAM) {
		gsm.print(F("RAM:"));
	}
	gsm.print(filename);
	gsm.print(F("\","));
	gsm.print(filesize,DEC);
	gsm.println("");
	
	while (flag) {
		String req = gsm.readStringUntil('\n');	
	   
	    if (req.indexOf(F("CONNECT")) != -1) {
			flag = 0;
		}
		if (req.indexOf(F("ERROR")) != -1) {
			gsm.debug(F("error uploading file\r\n"));
			return (false);
		}
	}
	
	gsm.print(filePtr);
	
	flag = 1;
	while (1) {
		String req = gsm.readStringUntil('\n');	
	   
	    if (req.indexOf(F("+QFUPL:")) != -1) {
			String retStr = (req.substring(req.indexOf(F(" "))+1));
			gsm.debug(retStr);
			gsm.debug("\r\n");
		}
		
		if (req.indexOf(F("OK")) != -1) {
			return (true);
		}
		
		if (req.indexOf(F("ERROR")) != -1) {
			gsm.debug(F("error uploading file\r\n"));
			return (false);
		}
		
	}
}


bool UC_FILE::flush(int handle) {
	gsm.print(F("AT+QFFLUSH="));
	gsm.print(handle, DEC);
	gsm.println("");
	return gsm.wait_ok_ndb(1000);
}
