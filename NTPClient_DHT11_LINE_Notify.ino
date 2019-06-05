#include <TridentTD_LineNotify.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>


#define DHTPIN 2         //pin connect DHT
#define DHTTYPE DHT11     //DHT11, DHT22 type of Sensor
#define SSID        "MT_WF"
#define PASSWORD    "##8888#"


boolean _state = true;
boolean _state8 = true;
boolean _state00 = true;
boolean _state16 = true;
int arr[12] = {5,10,15,20,25,30,35,40,45,50,55,59};

#define LINE_TOKEN "WImpkVlEK262c9TaNuQsxwBuh1FgbUEUY6WrNsr10" //ใส่ Line Token 
DHT dht(DHTPIN, DHTTYPE);

const long utcOffsetInSeconds = 7 * 3600; 
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  Serial.begin(115200); Serial.println();
  Serial.println(LINE.getVersion());
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  
  LINE.setToken(LINE_TOKEN);  // กำหนด Line Token
  dht.begin();
  timeClient.begin();
}

void Notify(float t, float h){
   LINE.notify("อุณหภูมิขณะนี้ "+String(t)+" องศา"); 
   delay(60);
   LINE.notify("ความชื้นขณะนี้ "+String(h)+" %");     
   delay(6000);   
}

void NotifyErr(float t, float h){
   LINE.notify("อุณหภูมิมีปัญหา "+String(t)+" องศา"); 
   delay(60);
   LINE.notify("ความชื้นมีปัญหา"+String(h)+" %");     
   delay(6000);   
}


void loop() {
  delay(60);
  timeClient.update();
  delay(100);
  //Serial.println("Minutes "+timeClient.getMinutes());
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
  delay(100); 
  if (isnan(t) || isnan(h)){ 
    int times = timeClient.getMinutes();
      for (byte i = 0; i < 11; i = i++) {
        if(times == arr[i]){
          Serial.println("Error reading DHT!");
          delay(60);
          LINE.notify("Error reading DHT!");  
          delay(60);  
          return;        
        } 
      }        
  } 
  
  if(_state){ //ส่งข้อมูลตอนจ่ายไฟให้ ESP8266 ครั้งแรก    
    _state = false;
    Notify(t,h);   return;       
  }  
 
  
  //ส่งข้อมูลทุก 7:55 นาที
  if(timeClient.getHours() == 7 && timeClient.getMinutes() == 55 && timeClient.getSeconds() <= 2){
    _state = true;   return;       
  } 

  //ส่งข้อมูลทุก 16:59 นาที
  if(timeClient.getHours() == 15 && timeClient.getMinutes() == 59 && timeClient.getSeconds() <= 2){
   _state = true;    return;       
  } 

  //ส่งข้อมูลทุก 00:00 นาที
  if(timeClient.getHours() == 0 && timeClient.getMinutes() == 0 && timeClient.getSeconds() <= 2){    
    _state = true;  return;       
  } 
  
  if((int)t >= 35){ //ส้งข้อมูลถ้า อุณหภูมิ มากกว่า หรือ เท่ากับ 35 องศา
    int times = timeClient.getMinutes();
    for (byte i = 0; i < 11; i = i++) {
        if(times == arr[i]){
           NotifyErr(t,h);  return;       
        } 
      }    
  }  
}
