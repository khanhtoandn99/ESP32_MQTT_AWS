// link cach encode & decode JSON : https://arduinojson.org/v6/example/ 
// link thu vien json version 6.xx.x : https://github.com/bblanchon/ArduinoJson

#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <string.h>


AWS_IOT hornbill;

char WIFI_SSID[]="KhanhToan";
char WIFI_PASSWORD[]="09111999";
char HOST_ADDRESS[]="a20xh0y90cepw1-ats.iot.us-east-2.amazonaws.com";
char CLIENT_ID[]= "client id";
char TOPIC_NAME[]= "$aws/things/TAPIT-ESP/shadow/update";
//char topic_name[]= "$aws/things/TAPIT-ESP/shadow/update/delta" ;
 
#define DHTPIN 4 
#define DHTTYPE DHT11
#define BUTTON 2
#define LED 13
DHT dht(DHTPIN, DHTTYPE);

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[200];
char rcvdPayload[200];
unsigned long Time ;
float h ;
float t ;
bool x ;
String LightState  ;
const char* LightStateRcv ;
String check ;


// viec encode va decode co the dung assistant trong arduinoJson.org de no lam giup, link https://arduinojson.org/v6/assistant/
void mySubCallBackHandler (char* topicName, int payloadLen, char* payLoad)
{   
        check = "";
        for(int i = 0; i < payloadLen ; i++) 
          {
              rcvdPayload[i]=(char)payLoad[i] ;
              check += (char)payLoad[i] ;
          }
         if(check.indexOf("reported") > 0){ Serial.println(" Your Publishing ") ; } // neu trong payLoad ma co reported thi la do esp gưi, esp chi nhan tu desired thoi.
        else
          {   
              //////////////////////// DECODE:{ //////////////////////////////
              const size_t capacity = 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + 40 ;  /// dung ham nay deu subcribe den update
              DynamicJsonDocument jsonRcvBuffer(capacity);    
              DeserializationError error = deserializeJson( jsonRcvBuffer, rcvdPayload );
              if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
              }
              Serial.println("Received Message:"); Serial.println(payLoad);
              
              LightStateRcv = jsonRcvBuffer["state"]["desired"]["light"] ; // Dung ham nay neu ta subcribe den update 
              Serial.println("Received Message after Decode:");
              Serial.println(LightStateRcv) ; 
              /////////////////////// END DECODE } /////////////////////////////
              
              if(strcmp(LightStateRcv,"ON")==0){ x = 1 ; digitalWrite(LED,HIGH) ;}
              else if(strcmp(LightStateRcv,"OFF")==0){ x = 0 ; digitalWrite(LED,LOW) ; }
              msgReceived = 1 ;
          }
 
    for (int i = 0; i < payloadLen; i++) {
          rcvdPayload[i]=0;
      }
}



StaticJsonDocument<1024> jsonBuffer ;
void Encode( String LightState, float h, float t ) 
{
      // link cach encode 1 chuoi json ma cac truong (object) long nhau : Https://arduinojson.org/v6/api/jsonobject/createnestedobject/
      JsonObject root = jsonBuffer.to<JsonObject>();  // dau tien la tao ra truong root, truong root lon nhat nen dung ham to.<JsonObject>
      JsonObject State = root.createNestedObject("state");  // sau do truong "state" duoc tao ra trong truong "root" 
      JsonObject Reported = State.createNestedObject("reported");  // long "reported" vao trong "state"
      Reported["light"] = LightState ;  
      Reported["humi"] = h ; 
      Reported["temp"] = t ; 
      serializeJsonPretty(root, payload);      
}


 
void WifiConnect() ;
void AWS_Connect() ;


void setup() {
    Serial.begin(115200);
    delay(500);
    WifiConnect() ;
    AWS_Connect();
    delay(2000);
    pinMode(BUTTON,INPUT_PULLUP) ;
    pinMode(LED ,OUTPUT) ;
    dht.begin();
    h = dht.readHumidity();
    t = dht.readTemperature();
    x=1 ; if(x==1){ LightState = "ON" ; digitalWrite(LED,HIGH) ; } else{LightState = "OFF" ; digitalWrite(LED,LOW) ;}
    
    Time = millis() ;  
   
}

//////////////////////////////////////////////// LOOP //////////////////////////////////////
void loop() 
{  
   if( msgReceived == 1)
    {
      if(x==1){ LightState = "ON" ; digitalWrite(LED,HIGH) ; } else{LightState = "OFF" ; digitalWrite(LED,LOW) ;}
      msgReceived = 0 ;
      h = dht.readHumidity();
       t = dht.readTemperature();
       
       Encode(LightState,h,t) ;   //// sau khi cap nhat du tat ca cac gia tri, ta se ma hoa de publish len 1 lan lun. 
    
       if(hornbill.publish(TOPIC_NAME,payload) == 0)
          { Serial.print("Publish Message:"); Serial.println(payload); }   
          else
              { Serial.println("Publish failed"); }
    }

    
   if( (unsigned long)(millis()- Time) > 15000 )
    {
       if(x==1){ LightState = "ON" ; digitalWrite(LED,HIGH) ; } else{LightState = "OFF" ; digitalWrite(LED,LOW) ;}
       h = dht.readHumidity();
       t = dht.readTemperature();
       
       Encode(LightState,h,t) ;   //// sau khi cap nhat du tat ca cac gia tri, ta se ma hoa de publish len 1 lan lun. 
    
       if(hornbill.publish(TOPIC_NAME,payload) == 0)
          { Serial.print("Publish Message:"); Serial.println(payload); }   
          else
              { Serial.println("Publish failed"); }
       Time = millis();
    }

 
   if( digitalRead(BUTTON) == 0) 
    {
       delay(100) ;
       x = !x ;
       if(x==1){ LightState = "ON" ; digitalWrite(LED,HIGH) ; } else{LightState = "OFF" ; digitalWrite(LED,LOW) ;}
       Encode(LightState,h,t) ;   //// sau khi cap nhat du tat ca cac gia tri, ta se ma hoa de publish len 1 lan lun. 
    
       if(hornbill.publish(TOPIC_NAME,payload) == 0)
          { Serial.print("Publish Message:"); Serial.println(payload); }   
          else
              { Serial.println("Publish failed"); }  
       delay(200) ;       
    }
}
/////////////// END LOOP() ////////////////////////////////////















//******************************************---FUNCTIONS---************************************* //  

void WifiConnect()
{
  while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(WIFI_SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        // wait 5 seconds for connection:
        delay(5000);
    }
    Serial.println("Connected to wifi");
}



void AWS_Connect() 
{
  if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);

        if(0==hornbill.subscribe(TOPIC_NAME,mySubCallBackHandler)) /// Ten Topic muon SUBCRIBE
        {
            Serial.println("Subscribe Successfull");
        }
        else
        {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while(1);
        }
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }
}







//{
//  "version": 862,
//  "timestamp": 1559485666,
//  "state": {
//    "light": "ON",
//    "humi" : 95,
//    "temp" : 13,
//    
//  },
//  "metadata": {
//    "light": {
//      "timestamp": 1559485616
//    }
//  }
//}
