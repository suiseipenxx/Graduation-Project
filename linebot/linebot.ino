#include <IRremote.hpp>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define UDP_PORT 3787

WiFiUDP UDP;

int RECV_PIN = D2; // 使用數位腳位2接收紅外線訊號
IRrecv irrecv(RECV_PIN); // 初始化紅外線訊號輸入
decode_results results; // 儲存訊號的結構

bool state;

const char* ssid = "Asus z6";
const char* pass = "cat891021";

#define AP_SSID "ESP8266"
#define AP_PASS "magicnumber"


int i,j,x;

String appliances[]={"家電控制-欄位一","家電控制-欄位二","家電控制-欄位三","家電控制-欄位四","家電控制-欄位五","家電控制-欄位六","家電控制-欄位七","家電控制-欄位八"};
String infrared[]={"紅外線控制-欄位一","紅外線控制-欄位二","紅外線控制-欄位三","紅外線控制-欄位四","紅外線控制-欄位五","紅外線控制-欄位六","紅外線控制-欄位七","紅外線控制-欄位八"};
String recvname[8];
int Send[]={D0,D1,D3,D4,D5,D6,D7,D8};
int orignum[]={0,0,0,0,0,0,0,0};
int recvnum[8];
const byte LED_PIN = 13;

IPAddress sendudp(192,168,4,255);

unsigned long rec;

String code;
HTTPClient http;
WiFiClient client;
void setname(){
  http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=save");
  int httpCode = http.GET();
  code=http.getString();
  state=false;
  x=0;
  if(code=="更改指令名稱") {
    Serial.println(code);
    http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=sure");
    httpCode = http.GET();
    code=http.getString();
    Serial.println(code);
  }
  while(code=="更改指令名稱"){
    int httpCode = http.GET();
    String code1=http.getString();
    if(code1==appliances[x]){
      Serial.println(code1);
    }
    while(code1==appliances[x]){
      int httpCode = http.GET();
      code1=http.getString();
      if(code1!=appliances[x]){
        Serial.println(code1);
        recvname[x]=code1;
        state=true;
        code="0";
        break;
      }
    }
    if(x==8){
      x=0;
    }else if(state==true){
      http.end();
      break;
    }
    else{
      x++;
    }
  } 
}

void Reset(){
  http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=Reset");
  int httpCode = http.GET();
  String Reset=http.getString();
  Serial.println(Reset);
  if(Reset=="重置紅外線"){
    for(i=0;i<8;i++){
      recvnum[i]=orignum[i];
      Serial.println(recvnum[i]);
    }
    http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=sure");
    int httpCode = http.GET();
    code=http.getString();
  }else if(Reset=="重置指令"){
    for(i=0;i<8;i++){
      recvname[i]=appliances[i];
      Serial.println(recvname[i]);
    }
    http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=sure");
    int httpCode = http.GET();
    code=http.getString();
  }
  http.end();  
}

void Display(){
  http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=display");
  int httpCode = http.GET();
  String Disp=http.getString();
  if(Disp=="顯示紅外線儲存"){
    Serial.println(Disp);
    for(i=0;i<8;i++){
      Serial.println(recvnum[i]);
    }
  }else if(Disp=="顯示指令名稱"){
    Serial.println(Disp);
    for(i=0;i<8;i++){
      Serial.println(recvname[i]);
    }
  }
  http.end();
}

void Save(){
  http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=save");
  int httpCode = http.GET();
  code=http.getString();
  http.end();
  if(code=="紅外線偵測") {
    Serial.println(code);
  }
  while(code=="紅外線偵測"){
    if (irrecv.decode(&results)) { // 接收紅外線訊號並解碼
      rec=results.value;
      http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=recv");
      httpCode = http.GET();
      Serial.println(String("\nHTTP code:")+httpCode);
      String payload=http.getString();
      Serial.println(payload+": "+rec);
      irrecv.resume(); // 準備接收下一個訊號  
      http.end();
    }
    http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=save");
    int httpCode = http.GET();
    code=http.getString();
    if(code=="儲存") Serial.println(code);
    while(code=="儲存"){  
      int httpCode = http.GET();
      code=http.getString();
      for(i=0;i<8;i++){
        if(code==infrared[i]){
          Serial.println(code);
          recvnum[i]=rec;
          rec=0;  
          code="0";
          break;
        }
      }
    }
  }
  http.end();
}

void send_contral(){
  http.begin(client,"arduinolinebot1.herokuapp.com",80,"/main?key=send");
  int httpCode = http.GET();
  code=http.getString();
  for(i=0;i<8;i++){
    if(code==appliances[i]){
        int switchStatus = digitalRead(Send[i]);
        digitalWrite(Send[i],!switchStatus);
        Serial.print(Send[i]);
        Serial.print(",");
        Serial.println(appliances[i]);
    }
    if(code==infrared[i]){
      Serial.println(recvnum[i]);
      String message=String(recvnum[i]);
      char *mes=(char*)malloc((message.length()+1)*sizeof(char));
      message.toCharArray(mes,message.length()+1);
      UDP.beginPacket(sendudp,UDP_PORT);
      UDP.write(mes);
      UDP.endPacket();
    }
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN,INPUT);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid,pass);
  WiFi.softAP(AP_SSID,AP_PASS);

  while(WiFi.status() !=WL_CONNECTED){
    Serial.print(".");
    delay(500);  
  }

  for(i=0;i<8;i++){
    pinMode(Send[i],OUTPUT);
  }

  irrecv.blink13(true); // 設為true的話，當收到訊號時，腳位13的LED便會閃爍
  irrecv.enableIRIn(); // 啟動接收

  http.begin(client,"arduinolinebot1.herokuapp.com",80,"/");
  int httpCode = http.GET();
  code=http.getString();
  http.end();

  UDP.begin(UDP_PORT);
}

void loop() {
  setname();
  Save();
  send_contral();
  Reset();
  Display();
}
