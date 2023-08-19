#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define WIFI_SSID "ESP8266"
#define WIFI_PASS "magicnumber"
#define UDP_PORT 1026
WiFiUDP UDP;
char packet[255];
char reply[] = "OK";
   
void setup() {
  pinMode(12,OUTPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.print(WiFi.SSID());
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port");
  Serial.println(UDP_PORT);
}
   
void loop() {
  if(int packetSize = UDP.parsePacket()){
    //Serial.print("接收到資料 Size: ");
    //Serial.println(packetSize); 
    //int len = UDP.read(packet, 255);
    if (int len = UDP.read(packet, 255)){ 
      packet[len] = '\0';         // 補上\0讓arduino覺得他是字串
    }
    Serial.print("資料內容: ");
    Serial.println(packet);/*
    if(packet[3]=='0'&& packet[6]=='F'){
      digitalWrite(12,HIGH);
      Serial.println(packet);
      }
    if(packet[3]=='0'&& packet[6]=='N'){
      digitalWrite(12,LOW);
      Serial.println(packet);
    }*/
    //回傳
    UDP.beginPacket(UDP.remoteIP(),UDP.remotePort());
    UDP.write(reply);
    UDP.endPacket();
  }
}
