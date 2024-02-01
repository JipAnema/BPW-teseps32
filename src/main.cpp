#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Preferences.h>
#define DIAMETER 53
#define MAGNEETperREV 0.5  // doe 1/aantalmagneten en donder het hier neer
int printernummer = 1;

WiFiServer server(80);
String header;
int count;// = 0;
int prevcount = -1;
unsigned long interrupt_time;
unsigned long prev_interrupt_time;
String servername = "http://192.168.137.1:1880/update-sensor";
const char* getservername = "http://192.168.137.1:1880/get-sensor";
String afstand(int revolutions);
String httpGETRequest(const char* serverName);
int GetMaxLengte();
String maxlengte;
const char* ssid     = "Test";
const char* password = "123456789";
unsigned int lengterol = 250000;    //Let op! mm
float actlengte = lengterol;
float sensorReadingsArr[9];
bool GetNewLength = false;
Preferences preferences;

void IRAM_ATTR isr() {
  interrupt_time = millis();
  if (interrupt_time - prev_interrupt_time > 100) {
    count++;
    prev_interrupt_time = interrupt_time;
  }
}

void IRAM_ATTR isr2() {
  count = 0;
  prevcount = -1;
  GetNewLength = true;
}

void setup() {
  Serial.begin(9600);  
  preferences.begin("counter", false);
  count = preferences.getInt("count",0);
  lengterol = preferences.getInt("maxlengte",250);
  preferences.end();
  pinMode (32,INPUT_PULLUP);
  attachInterrupt(32, isr, RISING);
  attachInterrupt(33, isr2, FALLING);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  pinMode (33,INPUT_PULLUP);
  server.begin();
}

void loop() {
  if (GetNewLength) {
    lengterol = GetMaxLengte();
    actlengte = lengterol;
    GetNewLength = false;
    preferences.begin("counter", false);
    preferences.putInt("maxlengte",lengterol);
    preferences.end();
  }
  if(WiFi.status() == WL_CONNECTED) {
   WiFiClient client;
   HTTPClient http;
    if (count != prevcount) {
      http.begin(client, servername);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpSendData = "lengte="+afstand(count)+"&rolnummer=" + printernummer;
      int httpResponseCode = http.POST(httpSendData);
      //Voeg hier dan error handeling toe, maar fuck dat
      http.end();
      prevcount = count;
      preferences.begin("counter", false);
      preferences.putInt("count", count);
      preferences.end();
    }
  }
}

String afstand(int revolutions) {
  if (actlengte <= (DIAMETER * 3.1415 * MAGNEETperREV) || actlengte > lengterol) {
    return ("error");
  }
  else {
    char str[20];
    actlengte = lengterol - ((DIAMETER * 3.1415 * revolutions) * MAGNEETperREV);
    float buffer = ((lengterol- (DIAMETER * 3.1415 * revolutions * MAGNEETperREV))/1000);
    sprintf(str, "%.1f", buffer);
    return (str);
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}"; 
  if (httpResponseCode>0) {
    payload = http.getString();
  }
  else {
    lengterol = 0;
  }
  http.end();
  return payload;
}

int GetMaxLengte () {
    if (WiFi.status() == WL_CONNECTED) {
    maxlengte = httpGETRequest(getservername);
    JSONVar myObject = JSON.parse(maxlengte);
    if (JSON.typeof(myObject) == "undefined") {
      return 0;
    }
    else {
     JSONVar keys = myObject.keys();
     for (int i = 0; i < keys.length(); i++) {
       JSONVar value = myObject[keys[i]];
       sensorReadingsArr[i] = double(value);
     }
     return sensorReadingsArr[printernummer-1];
   }
  }
  return 0;
}