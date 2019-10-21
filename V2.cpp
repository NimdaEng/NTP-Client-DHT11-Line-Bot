#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureAxTLS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>


#define DHTPIN 2          //pin connect DHT
#define DHTTYPE DHT11     //DHT11, DHT22 type of Sensor
#define SSID        "MT_WF"
#define PASSWORD    "##mtwf8888#"

AsyncWebServer server(80);

unsigned long previousMillis = 0;       //กำหนดตัวแปรเก็บค่า เวลาสุดท้ายที่ทำงาน  
  
const long interval = 10000 * 3;       //กำหนดค่าตัวแปร ให้ทำงานทุกๆ 10 วินาที


float h = 0.00;
float t = 0.00;

String ip = "ไม่ได้รับไอพี";

boolean _state = true;
boolean _state8 = true;
boolean _state00 = true;
boolean _state16 = true;
int arr[12] = {5,10,15,20,25,30,35,40,45,50,55,59};

#define LINE_TOKEN "WImpkVlEK262c9TaNuQsxwBuh1FgbUEUY6WrNsreM10" //ใส่ Line Token 
DHT dht(DHTPIN, DHTTYPE);

const long utcOffsetInSeconds = 7 * 3600; 
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  Serial.begin(115200);  
  WiFi.begin(SSID, PASSWORD);  
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }  
  ip = WiFi.localIP().toString(); 
  
  dht.begin();
  timeClient.begin();

  if(!SPIFFS.begin()){
    Line_Notify("An Error has occurred while mounting SPIFFS");  
    return;  
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send(200, "text/plain", (String)t);
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send(200, "text/plain", (String)h);
  });
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.begin();
}

void Notify(float t, float h){
   Line_Notify("อุณหภูมิขณะนี้ "+String(t)+" องศา" + "/ความชื้นขณะนี้ "+String(h)+" %");
   
   delay(60);
   Line_Notify(ip);
}

void NotifyErr(float t, float h){
   Line_Notify("อุณหภูมิมีปัญหา "+String(t)+" องศา" + "/ความชื้นมีปัญหา"+String(h)+" %");   
   delay(60);
   Line_Notify(ip);
}


/* ส่ง line */
void Line_Notify(String message) {
  axTLS::WiFiClientSecure client; // กรณีขึ้น Error ให้ลบ axTLS:: ข้างหน้าทิ้ง

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;   
  }
  
  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Connection: close\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + message;  
  client.print(req);     
  delay(10);


  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
   
  }

  client.stop();
 
}



void loop() {
  delay(60);
  timeClient.update();
  delay(100);
  //Serial.println("Minutes "+timeClient.getMinutes());
  h = dht.readHumidity();
  t = dht.readTemperature(); 
  
  delay(100); 
  if (isnan(t) || isnan(h)){ 
      h = 0.00;
      t = 0.00;
      unsigned long currentMillis = millis();  /* อ่านค่าเวลาที่ ESP เริ่มทำงานจนถึงเวลาปัจจุบัน */
    if(currentMillis - previousMillis >= interval) /*ถ้าหากเวลาปัจจุบันลบกับเวลาก่อหน้านี้ มีค่า
                              มากกว่าค่า interval ให้คำสั่งภายใน if ทำงาน*/   
    {
        previousMillis = currentMillis; /*ให้เวลาปัจจุบัน เท่ากับ เวลาก่อนหน้าเพื่อใช้คำนวณเงื่อนไขในรอบถัดไป*/      
        Line_Notify("Error reading DHT!");           
    } 
    return; 
  } 
  
  if(_state){ //ส่งข้อมูลตอนจ่ายไฟให้ ESP8266 ครั้งแรก    
    _state = false;    
    Notify(t,h); return;       
  }  
 
  //ส่งข้อมูลทุก 7:55 นาที
  if(timeClient.getHours() == 7 && timeClient.getMinutes() == 55 && timeClient.getSeconds() <= 2){
    _state = true; return;       
  } 

  //ส่งข้อมูลทุก 16:59 นาที
  if(timeClient.getHours() == 15 && timeClient.getMinutes() == 59 && timeClient.getSeconds() <= 2){
    _state = true; return;       
  } 

  //ส่งข้อมูลทุก 00:00 นาที
  if(timeClient.getHours() == 0 && timeClient.getMinutes() == 0 && timeClient.getSeconds() <= 2){    
    _state = true; return;       
  } 
  
  if((int)t >= 35){ //ส้งข้อมูลถ้า อุณหภูมิ มากกว่า หรือ เท่ากับ 35 องศา
    int minute = timeClient.getMinutes();    
    for (byte i = 0; i < 11; i = i++) {
        if(minute == arr[i]){
           NotifyErr(t,h); return;       
        } 
     }    
  }  
}
