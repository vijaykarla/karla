#include <ArduinoJson.h>
#include <EasyTransfer.h>
#include <SoftwareSerial.h>
#include<stdio.h>
#include<string.h>

EasyTransfer ETin;
SoftwareSerial mySerial(10,9);

int switchPin = 11;
int httpErrors = 0;
int emergencyLoop = 0;
int actionCode = 0;
int secNoLoop = 0;
char secContact[12] = "9916291525";

struct RECEIVE_DATA_STRUCTURE{ 
  int emergencyState;
  double G;
  double X;
  double Y;
  double Z;
  double Roll;
  double Pitch;  
};

struct SERVER_STATUS{ 
  int security;
  int safety;
  int relaySwitch;
  int buzzerDestinationArrival;  
  int buzzerDestinationDeparture;
  int buzzerLocator;
  int buzzOnSecurity;
  int buzzerSecurity;
  int buzzerEmergency;
  int securityCall;
  int voiceSurveillance;
  int qualityAssurance;
  int reset;
};

RECEIVE_DATA_STRUCTURE rxdata;
SERVER_STATUS serverStatus;

void setup()
{
  doSetup();
}

void loop()
{  
  secNoLoop = secNoLoop + 1;
  if(secNoLoop == 30)
    secNoLoop = 0;
  String latstr = "";
  String longstr = "";
  String elevation = "";
  String groundSpeed = "";
  char* response = {sendData("AT+CGNSINF",1000)};
  if((strlen(response) < 30) || (strstr(response, "+CGNSINF: 0,,,,,,,,,,,,,,,,,,,,") != NULL))
  {
    //logEntry("Device_is_off-Restarting");
    //Serial.println("Device is off, Restarting...");
    start();
  }
  else if (strstr(response, "+CGNSINF: 1,1") != NULL && strlen(response) > 60) {
    String content = response;
    int index = 46;
    bool latstatus = false;   
    bool longstatus = false;  
    bool elevationstatus = false;   
    while (content[index] != '\0' && index < 256)
    {
      char a = content[index];
      if(!latstatus && a != ',')
      {
        latstr += a;
      }
      else
      {
        if(latstatus)
        {
          if(!longstatus && a != ',')
          {
            longstr += a;
          }
          else
          {
            if(longstatus)
            {
              if(!elevationstatus && a != ',')
              {
                elevation += a;
              }
              else
              {
                if(elevationstatus)
                {
                  if(a != ',')
                  {
                    groundSpeed += a;
                  }
                  else
                  {
                    break;
                  }
                }
                else
                {
                  elevationstatus = true;
                }
              }
            }
            else
            {
              longstatus = true;
            }
          }
        }
        else
        {
          latstatus = true;
        }
      }
      index++;
    }    
    feedData(latstr, longstr, elevation, groundSpeed, false);    
    delay(500);
  }
  else if (strstr(response, "+CGNSINF: 1,0") != NULL)
  {
    feedData("", "", "", "", false);
    delay(500);
  }
  if(secNoLoop == 1)
    fetchTrackInfo();  
  getMaxData(latstr, longstr, elevation, groundSpeed);
  Serial.println("RAM: " + String(freeRam(), DEC));
}

int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void getMaxData(String latstr, String longstr, String elevation, String groundSpeed)
{  
  Serial.print(F(":::::::::::::::::::::::::::::: "));
  Serial.print(emergencyLoop);
  Serial.println(F(" ::::::::::::::::::::::::::::"));
  for(int i=0; i<5; i++){
    ETin.receiveData();    
    Serial.print(rxdata.emergencyState);
    Serial.print(F("   "));
    Serial.print(rxdata.G);
    Serial.print(F("   "));
    Serial.print(rxdata.X);
    Serial.print(F("   "));
    Serial.print(rxdata.Y);
    Serial.print(F("   "));
    Serial.print(rxdata.Z);
    Serial.print(F("   "));
    Serial.print(rxdata.Roll);
    Serial.print(F("   "));
    Serial.println(rxdata.Pitch);
    /*if(rxdata.emergencyState == 1)
    {
      break;
    }*/
    //digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)   
    delay(10);
  }
  if((serverStatus.safety || serverStatus.security) && (rxdata.emergencyState == 1 || rxdata.emergencyState == 2 || emergencyLoop > 0))
  {
    if(emergencyLoop < 3)
    {      
      if(serverStatus.security && rxdata.emergencyState == 2 && emergencyLoop == 0)
      {
        if(serverStatus.buzzOnSecurity)
          tone(3, 3000, 30000);

        Serial.println(F("Giving Security Call..1")); 
        for(int d=0; d<3; d++){
          giveRing();
          delay(20000);
          callDisconnect();
        }
      }
      if(serverStatus.safety && serverStatus.buzzerEmergency && rxdata.emergencyState == 1 && emergencyLoop == 0)
      {
        tone(3, 3000, 30000);
      }
      for(int e=0; e<2; e++){
        feedData(latstr, longstr, elevation, groundSpeed, true);
        delay(1000);
      }
      emergencyLoop++;
    }
    else
    {
      resetEmergency();
    }
  }
  else
  {
    resetEmergency(); 
  }
}

void fetchTrackInfo()
{  
  char* response = sendData("AT+HTTPPARA=\"URL\",\"http://peptrack.com/api/v1.0/data/securityno/id/1\"",6000);
  //Serial.println(response);
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
  char json[200] = "";
  Serial.println(F("threshold response is ..."));
  Serial.println(response);
  while (response[index] != '\0' && index < 256)
  {
    char c = response[index];
    if(c == '{')
    {
      addchar = true;
    }
    if(addchar)
    {
      append(json, c);
    }
    if(c == '}')
    {
      break;
    }
    index = index+1;
  }

  Serial.println(F("json is ..."));
  Serial.println(json);
    
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json); 

  if(root.success())
  {
    strcpy(secContact, root["data"][1]);    
    Serial.println(F("Contact Number is ..."));
    Serial.println(secContact);
  }
  else
  {
    Serial.println(F("Error getting Contact Number...."));
    Serial.println(secContact);
  }
}

void giveRing()
{
  char cmd[20] = "ATD";
  strcat(cmd, secContact);
  strcat(cmd, ";");
  sendData(cmd, 1000);
}

void callDisconnect()
{  
  sendData("ATH", 1000);
}

bool isCallActive()
{
  char myResponse[200] = ""; 
  bool isActive = false;
  sendData2("AT+CPAS",5000, myResponse);
  if (strstr(myResponse, "+CPAS: 3") != NULL || strstr(myResponse, "+CPAS: 4") != NULL) {
    isActive = true;
  }
  return isActive;
}

void resetEmergency()
{
  digitalWrite(3, HIGH);
  emergencyLoop = 0;
  rxdata.emergencyState = 0;
  rxdata.G = 0;
  rxdata.X = 0;
  rxdata.Y = 0;
  rxdata.Z = 0;
  rxdata.Roll = 0;
  rxdata.Pitch = 0;
}

void feedData(String lat, String lng, String elevation, String groundSpeed, bool emergency)
{
  bool fail = false; 
  String mycommand = "AT+HTTPPARA=\"URL\",\"http://peptrack.com/api/v1.0/data/push?t=1&a=";
  mycommand.concat(lat);
  mycommand.concat("&b=");
  mycommand.concat(lng);
  mycommand.concat("&c=");
  mycommand.concat(elevation);
  mycommand.concat("&d=");
  mycommand.concat(groundSpeed);
  if(emergency)
  {
     mycommand.concat("&s=");
     mycommand.concat(rxdata.emergencyState);
     mycommand.concat("&g=");
     mycommand.concat(rxdata.G);
     mycommand.concat("&x=");
     mycommand.concat(rxdata.X);
     mycommand.concat("&y=");
     mycommand.concat(rxdata.Y);
     mycommand.concat("&z=");
     mycommand.concat(rxdata.Z);
     mycommand.concat("&r=");
     mycommand.concat(rxdata.Roll);
     mycommand.concat("&p=");
     mycommand.concat(rxdata.Pitch);
  }
  mycommand.concat("&k=dmlqYXk6dmlqYXk1\"");    
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

      Serial.println(F("Response state is .........."));
      Serial.println(myResponse); 

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
    char s1[20] = "Http Error Count: ";
    char s2[1];
    //Serial.println(strcat(s1,itoa(httpErrors, s2, 10)));
  }
  else
  {
    httpErrors = 0;
  }
  if(httpErrors > 1)
  {
    //May have to check network
    httpErrors = 0;
    //Serial.println("Ping is off, Restarting network...");
    getnetwork();
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
  setStatus(result);
}

void setStatus(int dec)
{
  serverStatus.security = serverStatus.safety = serverStatus.relaySwitch = serverStatus.buzzerDestinationArrival = serverStatus.buzzerDestinationDeparture = serverStatus.buzzerLocator = serverStatus.buzzOnSecurity = serverStatus.buzzerSecurity = serverStatus.buzzerEmergency = serverStatus.securityCall = serverStatus.voiceSurveillance = serverStatus.qualityAssurance = serverStatus.reset = 0;

  if(dec > 0)
  {
    int a[20]; 
    int s,c=0; 
    while(dec>0) 
    { 
       s = dec%2; 
       c++; 
       dec=dec/2;
      
      if(c == 1)
      {
        serverStatus.security = s;
        if(serverStatus.security == 1)
          Serial.println(F("Server State: Security - ON"));
      }
      else if(c == 2)
      {
        serverStatus.safety = s;
        if(serverStatus.safety == 1)
          Serial.println(F("Server State: Safety - ON"));
      }
      else if(c == 3)
      {
        serverStatus.relaySwitch = s;
        if(serverStatus.relaySwitch == 1)
          Serial.println(F("Server State: RelaySwitch - ON"));
      }
      else if(c == 4)
      {
        serverStatus.buzzerDestinationArrival = s;
        if(serverStatus.buzzerDestinationArrival == 1)
          doBuzz(1);
      }
      else if(c == 5)
      {
        serverStatus.buzzerDestinationDeparture = s;
        if(serverStatus.buzzerDestinationDeparture == 1)
          doBuzz(2);
      }
      else if(c == 6)
      {
        serverStatus.buzzerLocator = s;
        if(serverStatus.buzzerLocator == 1)
          doBuzz(3);
      }
      else if(c == 7)
      {
        serverStatus.buzzOnSecurity = s;     
        if(serverStatus.buzzOnSecurity == 1)
          Serial.println(F("Server State: BuzzOnSecurity - ON")); 
      }
      else if(c == 8)
      {
        serverStatus.buzzerSecurity = s;
        if(serverStatus.buzzerSecurity == 1)
          tone(3, 3000, 30000);
      }
      else if(c == 9)
      {
        serverStatus.buzzerEmergency = s;
        if(serverStatus.buzzerEmergency == 1)
          tone(3, 3000, 30000);
      }
      else if(c == 10)
      {
        serverStatus.securityCall = s;
        if(serverStatus.securityCall == 1)
        {
          Serial.println(F("Giving Security Call..2")); 
          for(int d=0; d<3; d++){
              giveRing();
              delay(20000);
              callDisconnect();
            }
        }
      }
      else if(c == 11)
      {
        serverStatus.voiceSurveillance = s;
        if(serverStatus.voiceSurveillance == 1)
        {
          Serial.println(F("Giving Voice Surveillance Call")); 
          giveRing();
          bool isActive = isCallActive();
          while(isActive)
          {
            Serial.println(F("Call is active")); 
            delay(2000);
            isActive = isCallActive();
          }
          callDisconnect();
          //Start as we get error after this
          hardReset();
        }
      }
      else if(c == 12)
      {
        serverStatus.qualityAssurance = s;
      }
      else if(c == 13)
      {
        serverStatus.reset = s;
        if(serverStatus.reset == 1)
          hardReset();
      }
    }
  }
}

void doBuzz(int type)
{
  if(type == 1)
  {
    tone(3, 2000, 500);
    tone(3, 3000, 500);
    tone(3, 4000, 500);
    tone(3, 5000, 500);
  }
  else if(type == 2)
  {
    tone(3, 5000, 500);
    tone(3, 4000, 500);
    tone(3, 3000, 500);
    tone(3, 2000, 500);
  }
  else if(type == 3)
  {
    for(int l = 0; l < 5; l++)
    {
      doBuzz(1);
      doBuzz(2);
    }
  }
}

char* sendData(char *command, int timeout)
{
  Serial.println(F("Command is .........."));
  Serial.println(command); 
  char response[200] = "";
  while(mySerial.available() > 0) mySerial.read(); // Clean the input buffer
  mySerial.println(command);
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(mySerial.available())
    {       
      char c = mySerial.read();
      append(response, c);
    }  
  }  
  Serial.println(F("Response is .........."));
  Serial.println(response);
  return response;
}

void sendData2(char *command, int timeout, char *response)
{
  Serial.println(F("Command is .........."));
  Serial.println(command);  
  while(mySerial.available() > 0) mySerial.read(); // Clean the input buffer
  mySerial.println(command);
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(mySerial.available())
    {       
      char c = mySerial.read();
      append(response, c);
    }  
  }  
  Serial.println(F("Response is .........."));
  Serial.println(response);  
}

void hardReset()
{
  Serial.println(F("Hard Reset"));
  pinMode(12, OUTPUT);
  int resetPin = 12;
  digitalWrite(resetPin, LOW);
  delay(500);
  digitalWrite(resetPin, HIGH);
  doSetup();
}

void doSetup()
{
  delay(20000);
  pinMode(13, OUTPUT);  
  pinMode(3,OUTPUT);
  digitalWrite(3, HIGH);
  start();
}

void start()
{
  switchon();
  Serial.begin(9600); 
  ETin.begin(details(rxdata), &Serial);  
  mySerial.begin(9600);
  delay(10000);
  getgps();
  delay(5000);
  getnetwork();
}

void getgps(void)
{
  char *response = sendData("AT+CGNSPWR=1",1000);
  int counter = 0;
  while (strstr(response, "OK") == NULL && counter < 8)
  {
    switchon();
    response = sendData("AT+CGNSPWR=1",1000);
    counter++;
  }
  sendData("AT+CGNSSEQ=RMC",1000);
}

void getnetwork()
{
  //Check signal
  sendData("AT+SAPBR=3,1,\"APN\",\"live.vodafone.com\"",1000);
  sendData("AT+SAPBR=1,1",1000);
  sendData("AT+HTTPINIT",1000);
  sendData("AT+CLIP=1",1000);
}

void switchon()
{
  char *response = sendData("AT",1000); 
  if (strstr(response, "OK") == NULL)
  {
    Serial.println(F("Switching on ..."));
    delay(4000);
    pinMode(switchPin, OUTPUT);
    digitalWrite(switchPin, HIGH);
    delay(1000);
    digitalWrite(switchPin, LOW);  
    delay(3000);
  }
  else
  {
    Serial.println(F("Already on ..."));
  }
}

void append(char* s, char c)
{
  int len = strlen(s);
  s[len] = c;
  s[len+1] = '\0';
}
