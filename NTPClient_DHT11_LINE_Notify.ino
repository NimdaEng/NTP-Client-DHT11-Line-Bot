#include <TridentTD_LineNotify.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>


#define DHTPIN 2         //pin connect DHT
#define DHTTYPE DHT11     //DHT11, DHT22 type of Sensor
#define SSID        "ชื่อ WiFi"
#define PASSWORD    "รหัส WiFi"

#define LINE_TOKEN "Line Token" //ใส่ Line Token 
DHT dht(DHTPIN, DHTTYPE);

const long utcOffsetInSeconds = 7 * 3600;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

boolean _state = true;
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
void loop() {
  timeClient.update();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(t) || isnan(h)){    
      LINE.notify("Error reading DHT!");   
      delay(60000);
      return;    
  }else{    
    if(timeClient.getMinutes() == 1 && timeClient.getSeconds() <= 2){
      LINE.notify("อุณหภูมิขณะนี้ "+String(t)+" องศา"); 
      LINE.notify("ความชื้นขณะนี้ "+String(h)+" %"); 
    } 
  }
  if(_state){
    LINE.notify("อุณหภูมิขณะนี้ "+String(t)+" องศา"); 
    LINE.notify("ความชื้นขณะนี้ "+String(h)+" %");  
    _state = false;
  } 

  if((int)t >= 28){
    //LINE.notifySticker(3,240);
    LINE.notify("อุณหภูมิขณะนี้ "+String(t)+" องศา"); 
    LINE.notify("ความชื้นขณะนี้ "+String(h)+" %");
    delay(60000);
    return;
  }
  
}
