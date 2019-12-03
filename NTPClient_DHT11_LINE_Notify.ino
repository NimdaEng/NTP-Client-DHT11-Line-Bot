#include <TridentTD_LineNotify.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHTesp.h"


#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif


DHTesp dht;

#define SSID        "kid_2.4GHz"
#define PASSWORD    "xx3xx3xx"

#define DHTPIN 2
#define DHTTYPE DHT11 // DHT 11 
float t = 0.00;
float h = 0.00;
unsigned long previousMillis = 0;  
const long interval = 1000*60*1;

boolean _state = true;
int arr[12] = {5,10,15,20,25,30,35,40,45,50,55,59};

#define LINE_TOKEN "TftxtM2bfiZkqZjEpIbEURIT3gIDtNLGcwdm82QzwX0"

const long utcOffsetInSeconds = 7 * 3600; 
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

void setup() {
  Serial.begin(115200); 
  Serial.println();
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);
  
  WiFi.begin(SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED) { 
    Serial.print("."); delay(400);    
    }
    Serial.println();
    Serial.println("WiFi success.");
    dht.setup(2, DHTesp::DHT11);
   delay(dht.getMinimumSamplingPeriod());
   t = dht.getTemperature();
   h = dht.getHumidity();
   
  LINE.setToken(LINE_TOKEN);  // กำหนด Line Token  
  timeClient.begin();  
}// END

void NotifyNormal(float t, float h){ 
   LINE.notifySticker("หนาวจังเลยวันนี้",2,29);  
   LINE.notify("อุณหภูมิขณะนี้ "+String(t)+" องศา");    
   LINE.notify("ความชื้นขณะนี้ "+String(h)+" %");     
}

void NotifyAlarm(float t, float h){
   LINE.notifySticker("งานเข้าแล้ว เร็วๆๆๆๆๆๆ",2,27);
   LINE.notify("อุณหภูมิมีปัญหา "+String(t)+" องศา");    
   LINE.notify("ความชื้นมีปัญหา "+String(h)+" %");  
}

void loop() {
  timeClient.update();  
  unsigned long currentMillis = millis();  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;    
    delay(dht.getMinimumSamplingPeriod());    
    t = dht.getTemperature();
    h = dht.getHumidity();    
    if (isnan(t) || isnan(h)){ 
       LINE.notify("Failed to read from DHT");
    }else{
        if((int)t >= 32){ 
          _state = false;
          NotifyAlarm(t,h);    
        }
    } 
  }//End interval  
  

  
  if(_state){ //ส่งข้อมูลตอนจ่ายไฟให้ ESP8266 ครั้งแรก    
    _state = false;
    NotifyNormal(t, h);   return;       
  }  
 
  
  //ส่งข้อมูลทุก 7:55 นาที
  if(timeClient.getHours() == 7 && timeClient.getMinutes() == 55 && timeClient.getSeconds() <= 2){
    LINE.notify("ส่งข้อมูลทุก 7:55 นาที");  
    _state = true;   return;         
  } 

  //ส่งข้อมูลทุก 16:59 นาที
  if(timeClient.getHours() == 15 && timeClient.getMinutes() == 59 && timeClient.getSeconds() <= 2){
   LINE.notify("ส่งข้อมูลทุก 16:59 นาที");  
   _state = true;    return;       
  } 

  //ส่งข้อมูลทุก 00:00 นาที
  if(timeClient.getHours() == 0 && timeClient.getMinutes() == 0 && timeClient.getSeconds() <= 2){    
    LINE.notify("ส่งข้อมูลทุก 00:00 นาที");  
    _state = true;  return;       
  }
  
}//End loop
