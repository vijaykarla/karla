#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include<stdio.h>
#include<string.h>
#include "max6675.h"

SoftwareSerial mySerial(10,9);

int switchPin = 11;
int resetPin = 12;
int httpErrors = 0;
char secContact[11] = "";
float alarmLow = 0;
float alarmHigh = 100;
bool readTrack = true;
bool alerted = false;
int tempCount = 0;
float prevTemp = 0;

int ktcSO = 6;
int ktcCS = 7;
int ktcCLK = 8;

unsigned long interval = 180000;
unsigned long previousMillis = 0;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

void setup()
{
  start();
}

void start()
{
  Serial.begin(9600);
  mySerial.begin(9600);
   
  switchon();
}

void switchon()
{
  char *response = sendData("AT",1000); 
  if (strstr(response, "OK") == NULL)
  {
    //Serial.println(F("Switching on ..."));    
    pinMode(switchPin, OUTPUT);
    digitalWrite(switchPin, HIGH);
    delay(1000);
    digitalWrite(switchPin, LOW);  
    delay(20000);    
  }
  else
  {
    //Serial.println(F("Already on ..."));
    delay(5000);
  }
}

void getnetwork()
{
  sendData("AT+CSQ", 1000);
  delay(100);    
  sendData("AT+CGATT?", 1000);
  delay(100);     
  sendData("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", 1000);
  delay(5000);    
  sendData("AT+SAPBR=3,1,\"APN\",\"bsnlnet\"", 4000);
  delay(4000);    
  sendData("AT+SAPBR=1,1", 5000);
  delay(5000);    
  sendData("AT+HTTPINIT", 3000);
  delay(3000);  
  ClearSerialData();
}

void loop()
{ 
  if(httpErrors > 5)
  {
    readTrack = true;
    alerted = false;    
    tempCount = 0;
    httpErrors = 0;
    //Serial.println(F("Too many errors!"));
    start();
  }
  else
  {
    if(readTrack == true)
    {
      tempCount++;
      if(tempCount < 4)
      {
        getnetwork();
        fetchTrackInfo();
      }
      else
      {
        readTrack = true;
        alerted = false; 
        tempCount = 0;
        httpErrors = 0;
        start();
      }
    }
    else
    {
      tempCount = 0;
      float temp = ktc.readCelsius();      
      //Serial.println(F("Temperature is ..."));
      Serial.println(temp);
      float diffTemp = prevTemp - temp;
      if((temp <= alarmLow || temp >= alarmHigh) && (diffTemp >= -100 && diffTemp <= 100))
      {
        if(alerted == false)
        {
          delay(1000);
          temp = ktc.readCelsius();
          if(temp <= alarmLow || temp >= alarmHigh)
          {
            dialContact();
          }
          if(alerted)
          {
            feedData(temp);
          }
        }
        else
        {
          unsigned long currentMillis = millis();
          if (alerted || ((unsigned long)(currentMillis - previousMillis) >= interval))
          {
            previousMillis = millis();
            feedData(temp);
            alerted = false;
          }
          else
          {
            delay(1000);
          }
        }
      }
      else
      {
        unsigned long currentMillis = millis();
        if (alerted || ((unsigned long)(currentMillis - previousMillis) >= interval))
        {
          previousMillis = millis();
          feedData(temp);
          alerted = false;
        }
        else
        {
          delay(1000);
        }
      }
      prevTemp = temp;
    }    
  }
  //Serial.println("RAM: " + String(freeRam(), DEC));
}

int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void fetchTrackInfo()
{  
  char* response = sendData("AT+HTTPPARA=\"URL\",\"http://peptrack.com/dashboard/temperature/temptrack?id=10099\"",10000);  
  if (strstr(response, "OK") != NULL)
  { 
    response = sendData("AT+HTTPACTION=0",8000);      
    if ((strstr(response, "+CME ERROR:") == NULL) && (strstr(response, "601") == NULL))
    {
      char myResponse[200] = ""; 
      sendData2("AT+HTTPREAD",8000, myResponse);       
      setTrackInfo(myResponse);        
    }        
  }
}

void setTrackInfo(char* response)
{
  bool addchar = false;
  int index = 0;
  char json[30] = "";
  //Serial.println(F("Track info response is ..."));
  //Serial.println(response);
  if (strstr(response, "#") != NULL)
  {
    while (index < 60)
    {
      char c = response[index];
      if(c == '#' && !addchar)
      {
        addchar = true;
        index++;
        continue;
      }
      else if(c == '#')
        break;
      
      if(addchar)
        append(json, c);
     
      index++;
    }
    append(json, '|');
  
    //Serial.println(F("json is ..."));
    //Serial.println(json);
  
    index = 0;
    bool cAdded = false;
    bool lAdded = false;
    bool hAdded = false;
    char contact[10] = "";
    char low[6] = "";
    char high[6] = "";
    while (json[index] != '\0' && index < 30)
    {
        char c = json[index];
        if(c != '|' && c != '\0')
        {
          if(!cAdded)
            append(contact, c);
          else if(!lAdded)
            append(low, c);
          else
            append(high, c);
        }
        else
        {
          if(!cAdded)
            cAdded = true;
          else if(!lAdded)
            lAdded = true;
          else
          {
            hAdded = true;
            readTrack = false; 
            //Serial.println(F("contact ..."));
            //Serial.println(contact);
            //Serial.println(F("low ..."));
            //Serial.println(low);
            //Serial.println(F("high ..."));
            //Serial.println(high);   
            strncpy(secContact, contact, 10);
            secContact[10] = '\0';                
            alarmLow = atof(low);
            alarmHigh = atof(high);
            break;
          }
        }
        index++;
    }
    if(!hAdded)
      httpErrors++;
    else
      httpErrors = 0;
  }
  
  Serial.println(F("Contact Number is ..."));
  Serial.println(secContact);
  //Serial.println(F("Alarm High is..."));
  //Serial.println(alarmHigh);
  //Serial.println(F("Alarm low is ..."));
  //Serial.println(alarmLow);  
  //Serial.println(F("json is ..."));
  //Serial.println(json);   
}

void feedData(float temp)
{
  bool fail = false; 
  String mycommand = "AT+HTTPPARA=\"URL\",\"http://peptrack.com/api/v1.0/data/pusht?t=10099&k=nl92jcphuko73gu3q3ejw&a=";
  mycommand.concat(temp);   
  mycommand.concat("\"");   
  int str_len = mycommand.length() + 1; 
  char charBuf[str_len];
  mycommand.toCharArray(charBuf, str_len);  
  char* response = sendData(charBuf, 6000);
  
  if (strstr(response, "OK") != NULL)
  { 
    response = sendData("AT+HTTPACTION=0",8000);      
    if ((strstr(response, "+CME ERROR:") == NULL) && (strstr(response, "601") == NULL))
    {
      char myResponse[200] = ""; 
      sendData2("AT+HTTPREAD",8000, myResponse);

      //Serial.println(F("Response state is .........."));
      //Serial.println(myResponse); 

      readStatus(myResponse);
    }        
  }
  else
  {
    fail = true;
  }   
  
  if(fail)
  {
    httpErrors++;   
  }
  else
  {
    httpErrors = 0;
  }  
}

void readStatus(char* response)
{
  bool addchar = false;
  int index = 0;
  char json[200] = "";  
  while (response[index] != '\0' && index < 256)
  {
    char c = response[index];    
    if(c == 'O')
    {
      break;
    }
    if(addchar && c != '\n')
    {     
      append(json, c);
    }
    if(c == '1' || c == '2' || c == '3' || c == '4')
    {
      addchar = true;
    }    
    index = index+1;
  }

  Serial.println(F("State code is ..."));
  Serial.println(json);
  int result = atoi(json);
  if(result == 1)
   readTrack = true;
}

void dialContact()
{
  Serial.println(F("Giving Alert Call...")); 
  giveRing();
  bool isActive = isCallActive();
  int callIndex = 0;
  while(isActive && callIndex < 200)
  {
    callIndex = callIndex + 1;
    //Serial.println(F("Call is active")); 
    delay(2000);
    isActive = isCallActive();       
  }
  callDisconnect();
  mySerial.println();
}

void giveRing()
{
  char cmd[20] = "ATD";
  strcat(cmd, secContact);
  strcat(cmd, ";");
  sendData(cmd, 5000);
}

bool isCallActive()
{
  char myResponse[200] = ""; 
  bool isActive = false;
  sendData2("AT+CPAS",5000, myResponse);
  if (strstr(myResponse, "+CPAS: 3") != NULL || strstr(myResponse, "+CPAS: 4") != NULL) {
    isActive = true;
    alerted = true;
  }
  return isActive;
}

void callDisconnect()
{  
  sendData("ATH", 1000);
}

char* sendData(char *command, int timeout)
{
  //Serial.println(F("Command is .........."));
  //Serial.println(command); 
  char response[200] = "";
  int readIdex = 0;
  while(mySerial.available() > 0 && readIdex < 5000) 
  {
    readIdex = readIdex+1;
    mySerial.read();
  }
  mySerial.println(command);
  long int time = millis();
  while( (time+timeout) > millis())
  {
    readIdex = 0;
    while(mySerial.available() && readIdex < 1000)
    {   
      readIdex = readIdex+1;
      char c = mySerial.read();
      append(response, c);
    }  
  }  
  //Serial.println(F("Response is .........."));
  //Serial.println(response);
  return response;
}

void sendData2(char *command, int timeout, char *response)
{
  //Serial.println(F("Command is .........."));
  //Serial.println(command);  
  int readIdex = 0;
  while(mySerial.available() > 0 && readIdex < 5000) 
  {
    readIdex = readIdex+1;
    mySerial.read();
  }
  mySerial.println(command);
  long int time = millis();
  while( (time+timeout) > millis())
  {
    readIdex = 0;
    while(mySerial.available() && readIdex < 1000)
    {       
      readIdex = readIdex+1;
      char c = mySerial.read();
      append(response, c);
    }  
  }  
  //Serial.println(F("Response is .........."));
  //Serial.println(response);
}

void reset()
{
  //Serial.println(F("Hard Reset"));
  pinMode(resetPin, OUTPUT); 
  digitalWrite(resetPin, LOW);
  delay(500);
  digitalWrite(resetPin, HIGH);
}

void append(char* s, char c)
{
  int len = strlen(s);
  s[len] = c;
  s[len+1] = '\0';
}

void ClearSerialData()
{
  while(mySerial.available())
  {
    mySerial.read();
  }
}
