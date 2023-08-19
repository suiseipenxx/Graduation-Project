#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>// Provide the token generation process info.
#include <addons/RTDBHelper.h>// Provide the RTDB payload printing info and other helper functions.
#define API_KEY "AIzaSyBzRAbWytJdZTZlU99rz9sTO1teBdZrVJw"
#define DATABASE_URL "arduino-da53c-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "1026panda@gmail.com"
#define USER_PASSWORD "abcdefg789"
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long Status=0;

#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define AP_SSID "ESP8266"
#define AP_PASS "magicnumber"
#include <WiFiUdp.h>
#define UDP_PORT 1026
WiFiUDP UDP;
IPAddress sendudp(192, 168, 4, 255);
AsyncWebServer server(80);
//0-2 led 3 fan 4 bee 5 lcd 6 電磁鐵
//1  
//D8
int l[] = {12, 13, 14, 15, 16, 0, 2};
//cp2102 D6,D7,D5,D8,D0,D3,D4
String action[] = {"零號燈開燈", "零號燈關燈", "1號燈開燈", "1號燈關燈", "2號燈開燈", "2號燈關燈", "關風扇", "開風扇", "開音響", "關音響", "開電視", "關電視", "鎖門", "開門"};
String obj[] = {"LED0", "LED1", "LED2", "FAN", "BEE", "LCD", "DOOR"};
String temp[]= {"8","9","10","11","12","13","14","15","16"};
String user[] = {"1026panda@gmail.com"};
String password[] = {"123"};
String list = "0000000";
char token;
int Allowed[8];
int Aindex = 0;
void setup() {
  Init();
  Connecting();
  SettingmDNS();
  ConnectRTDB();
  ConnectInfo();
  Fileopen();
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest * request) {
    String response = "false";
    if (request->hasParam("email")) {
      AsyncWebParameter* p = request->getParam("email");
      AsyncWebParameter* p2 = request->getParam("password");
      //toRTDB
      for (int i = 0; i < sizeof(obj) / sizeof(String); i++) {
        if (p->value() == user[i]) {
          if (p2->value() == password[i]) {
            response = "true";
            Allowed[Aindex++] = i;
            token = i + 48;
            Serial.println(token);
          }
        }
      }
    }
    request->send(200, "text/plain", response);
  });
  server.on("/voice", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("data")) {
      AsyncWebParameter* p = request->getParam("data");
      for (int i = 0; i < sizeof(action) / sizeof(String); i++) {
        if (p->value() == action[i]) {
          digitalWrite(l[i / 2], (i % 2) ? LOW : HIGH);
          break;
        }
      }
    }
    request->send(200, "text/plain", "");
  });
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("logout")) {
      //logout
    }
    request->send(200, "text/plain", "");
  });
  server.on("/refresh", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("token")) {
      AsyncWebParameter* p = request->getParam("token");
      String tmp = p->value();
      Serial.println("tmp:" + tmp);
      token = tmp.toInt();
      Serial.println("token:" + token);
      Serial.println("refresh");
    }
    request->send(200);
  });
  server.on("/obj", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("觸發obj");
    if (request->hasParam("obj")) {
      AsyncWebParameter* p = request->getParam("obj");
      Serial.println("value="+p->value());
      //toRTDB
      Status=1;
      for (int i = 0; i < sizeof(obj) / sizeof(String); i++) {
        Serial.println("temp="+temp[i]);
        if (p->value() == temp[i]) {
          Serial.println("match");
          //list[i] = list[i] ^ 1; // 0=>1 1=>0
          AsyncWebParameter* p = request->getParam("state");
          Serial.println(obj[i] + " " + p->value());
          String message = obj[i] + " " + p->value();
          char *mes = (char*)malloc((message.length() + 1) * sizeof(char));
          message.toCharArray(mes, message.length() + 1);
          UDP.beginPacket(sendudp, UDP_PORT);
          UDP.write(mes);
          UDP.endPacket();
          free(mes);
          (p->value() == "ON") ? digitalWrite(l[i], HIGH) : digitalWrite(l[i], LOW);
//          if (i < 3) {
//            (p->value() == "ON") ? digitalWrite(l[i], HIGH) : digitalWrite(l[i], LOW);
//          } else if (i == 4) {
//            // bee();
//          } else if (i == 5) {
//            // lcd (server.arg("onoff")=="ON")?lcd.backlight():lcd.noBacklight();
//          } else if (i < 7) {
//            (p->value() == "ON") ? digitalWrite(l[i], LOW) : digitalWrite(l[i], HIGH);
//          }
          break;
        }
      }
    }
    request->send(200, "text/plain", "");
  });
//  server.on("/list", HTTP_GET, [](AsyncWebServerRequest * request) {
//    StaticJsonDocument<100> data;
//    data["token"] = token;
//    token = 0;
//    data["message"] = list;
//    String response;
//    serializeJson(data, response);
//    request->send(200, "application/json", response);
//  });
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404, "application/json", "{\"message\":\"Not found\"}");
  });
  ServerStart();
}

void loop() {
  MDNS.update();
  if (Firebase.ready() && Status)
  {
    Status=0;
    Serial.printf("Set string... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/string"), F("Hello World!")) ? "ok" : fbdo.errorReason().c_str());
    Serial.println();
  }
}
void Init() {
  Serial.begin(9600);
  SPIFFS.begin();
  WiFi.softAP(AP_SSID, AP_PASS);
  randomSeed(analogRead(0));
  for (int i = 0; i < 7; i++) {
    pinMode(l[i], OUTPUT);
    if (i != 3 && 1 != 6) {
      digitalWrite(l[i], LOW);
    } else {
      digitalWrite(l[i], HIGH);
    }
  }
  
}
void Connecting() {
  wifiMulti.addAP("120-35","12345678");
  wifiMulti.addAP("APAP", "09876543");
  wifiMulti.addAP("D-Link DIR-867", "10260722");
  Serial.print("Connecting ...");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(200);
    Serial.print('.');
  }
  Serial.println("");
}
void SettingmDNS() {
  Serial.print("mDns 設置中");
  if (MDNS.begin("group3")) {
    Serial.println("MDNS started");
  }
}
void ConnectRTDB(){
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

#if defined(ESP8266)
  // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}
void ConnectInfo() {
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.print("IP address for network ");
  Serial.print(AP_SSID);
  Serial.print(" : ");
  Serial.print(WiFi.softAPIP());
}
void Fileopen() {
  server.serveStatic("/", SPIFFS, "/www").setDefaultFile("login.html");
  server.serveStatic("/home", SPIFFS, "/www").setDefaultFile("test2.html");
  server.serveStatic("/favicon", SPIFFS, "/www").setDefaultFile("favicon.ico");
}
void ServerStart() {
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
  UDP.begin(UDP_PORT);
}
