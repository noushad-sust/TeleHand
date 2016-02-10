#include<Servo.h>

String info="Command server: To be configured and commanded from software.\n"+
            String("Version: 1: Digital,Analog,Servo: 9-1-16\n")+
            String("Command: header=pin:value,pin:value...\n")+
            String("value=anyvalue for read and config\n")+
            String("Ex: sw=7:120,6:180,5:30 \n")+
            String("Response: success/failed\nOption\n\\n");
            
#define header_dc "dc"    //digital config
#define header_dr "dr"    //digital read
#define header_dw "dw"    //digital write

#define header_ar "ar"    //analog read

/*
#define header_pc "pc"    //pwm config
#define header_pw "pw"    //pwm write
*/

#define header_sc "sc"    //servo config
#define header_sr "sr"
#define header_sw "sw"


char delimiter='=';
char comma=',';
char delimiter2=':';

#define MAX_COMMAND 20
#define SERVO_MAX 12
Servo servo[SERVO_MAX];
byte servo_pins[SERVO_MAX];


/*
Following 2 are dynamic temporary memory.
*/
byte pins[MAX_COMMAND];
byte vals[MAX_COMMAND];

String resp_data="";

/*
Usage:
sc=0:7,1:6,2:5,3:4
sw=0:120,3:90

key is always a pin number.
val is any number.

Response:
Success or Failed
Option Data
\n



*/

void printHeader() {
  Serial.println("Supported Headers:");
  Serial.println(header_dc);
  Serial.println(header_dr);
  Serial.println(header_dw);
  Serial.println(header_ar);
  Serial.println(header_sc);
  Serial.println(header_sr);
  Serial.println(header_sw);
}

void setup() {
  Serial.begin(460800);
  Serial.println(info);
  printHeader();
  Serial.println("Ready");
  Serial.println("\n");  //blank line indicate end of the response.
  
}

void loop() {
  String line=readLine();
  Serial.println("echo: "+String(line) );
  
  String header=getSplit(line, delimiter, 0);
  String data=getSplit(line, delimiter, 1);
  
  resp_data="";
  boolean success=onNewCommand(header, data);
  
  if(success) {
   Serial.println("Success"); 
  }else {
    Serial.println("Failed");
  }
  if(!resp_data.equals("")) {
    Serial.println(resp_data);
  }
  Serial.println("\n");  //blank line indicate end of the response.
}

byte parsePinsVals(String data) {
  byte n=nSplit(data, comma);
  n=n+1;  //length
  for(byte i=0;i<n;i++) {
   String cmd=getSplit(data, comma, i);
   String id=getSplit(cmd, delimiter2, 0);
   String val=getSplit(cmd, delimiter2, 1); 
   pins[i]=id.toInt();
   vals[i]=val.toInt();
  }
  return n;
}

boolean onNewCommand(String header, String data) {
  //Serial.println("nc="+String(header) );
  byte n=parsePinsVals(data);
  
  boolean success=false;
  
  
  if(header.equals(header_dc) ) {
    success=doDigital(header, n);
  }else if(header.equals(header_dr)) {
     success=doDigital(header, n);
  }else if(header.equals(header_dw) ) {
    success=doDigital(header, n);
  }
  
  else if(header.equals(header_ar) ) {
    success=doAnalog(header, n);
  }
  
  else if(header.equals(header_sc) ) {
    success=doServo(header, n);
  }else if(header.equals(header_sr)) {
     success=doServo(header, n);
  }else if(header.equals(header_sw)) {
    success=doServo(header, n);
  }
  
  return success;
}

void setResponse(String header, String d) {
 resp_data=header+delimiter+d; 
}

 

/*
Servo Control
*/
boolean doServo(String header, byte n) {
  
    if(header.equals(header_sc) ) {
      for(byte i=0;i<n;i++) {
       byte pin=pins[i];
       byte val=vals[i];
       
       boolean sc=false;
       for(byte s=0;s<SERVO_MAX;s++) {
          if(!servo[s].attached() )  {
            servo[s].attach(pin);
            servo_pins[s]=pin;
            sc=true;
            break;
          }
       }
       if(!sc) {
        //not attached.
         setResponse(header, "Can't attach on pin="+String(pin) );
         return false; 
       }
       
      }
      return true;
    }else if(header.equals(header_sr)) {
       for(byte i=0;i<n;i++) {
         //Serial.print("i="+String(i)+",");
         byte pin=pins[i];
         byte val=vals[i];
         
         boolean sc=false;
         for(byte s=0;s<SERVO_MAX;s++) {
          if(servo_pins[s]==pin)  {
            vals[i]=servo[s].read();
            sc=true;
            break;
          }
       }
       
        if(!sc) {
        //not attached.
         setResponse(header, "Can't read on pin="+String(pin) );
         return false; 
       }
       
         
         
      }
      String resp="";
      boolean isFirst=true;
      for(byte i=0;i<n;i++) {
           byte pin=pins[i];
           byte val=vals[i];
          if(isFirst) {
            isFirst=false;
          }else {
            resp+=comma;
          }
          resp+=String(pin)+String(":")+String(val);
      }
      
      setResponse(header,resp);
      return true;
      
    }else if(header.equals(header_sw)) {
      for(byte i=0;i<n;i++) {
       byte pin=pins[i];
       byte val=vals[i];
       
       boolean sc=false;
       for(byte s=0;s<SERVO_MAX;s++) {
          if(servo_pins[s]==pin)  {
            servo[s].write(val);
            sc=true;
            break;
          }
       }
        if(!sc) {
        //not attached.
         setResponse(header, "Can't write on pin="+String(pin) );
         return false; 
       }
       
       
      }
      return true;
    }else {
     setResponse(header,"Invalid Header");
     return false; 
  }
  
  return false;
}
/*
Digital IO
*/
boolean doDigital(String header, byte n) {
  
  if(header.equals(header_dc) ) {
    for(byte i=0;i<n;i++) {
     byte pin=pins[i];
     byte val=vals[i];
     pinMode(pin,val);
    }
    return true;
  }else if(header.equals(header_dr)) {
     for(byte i=0;i<n;i++) {
       byte pin=pins[i];
       byte val=vals[i];
       
       vals[i]=digitalRead(pin);
    }
    String resp="";
    for(byte i=0;i<n;i++) {
        byte pin=pins[i];
        byte val=vals[i];
        resp=String(pin)+String(":")+String(val)+comma;
    }
    
    setResponse(header,resp);
    return true;
    
  }else if(header.equals(header_dw) ) {
    for(byte i=0;i<n;i++) {
       byte pin=pins[i];
       byte val=vals[i];
       digitalWrite(pin,val);
    }
    return true;
  }else {
     setResponse(header,"Invalid Header");
     return false; 
  }
  
  return false;
}
boolean doAnalog(String header, byte n) {
   if(header.equals(header_ar)) {
       for(byte i=0;i<n;i++) {
         byte pin=pins[i];
         byte val=vals[i];
         
         vals[i]=analogRead(pin);
      }
      String resp="";
      for(byte i=0;i<n;i++) {
          byte pin=pins[i];
          byte val=vals[i];
          resp=String(pin)+String(":")+String(val)+comma;
      }
      
      setResponse(header,resp);
      return true;
      
    }
}


//----------------------Utility-------------
String readLine() {
 String dt="";
 while(true) {
    if(Serial.available()) {
      char ch=Serial.read();
      if(ch=='\n')
        break;
      else 
        dt+=ch;
    }  
 }
 return dt; 
}


//----------------------------------String parsing function--------------------

byte nSplit(String data, char separator) {
  byte total=0;
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex; i++){
    if(data.charAt(i)==separator){
      ++total;
    }
  }
  return total;
}
/*
*Alternative for String Split
 //data->String, speparator->separator, index->required splited part.
 */
String getSplit(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1    };
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

