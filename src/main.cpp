#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#define DIAMETER 39
#define MAGNEETperREV 0.5  // doe 1/aantalmagneten en donder het hier neer
int printernummer = 1;

WiFiServer server(80);
String header;
int count = 0;
int prevcount = -1;
unsigned long interrupt_time;
unsigned long prev_interrupt_time;
String servername = "http://192.168.137.1:1880/update-sensor";
const char* getservername = "http://192.168.137.1:1880/get-sensor";
String afstand(int revolutions);
String maxlengte;
const char* ssid     = "Test";
const char* password = "123456789";
unsigned int lengterol = 250000;    //Let op! mm
float actlengte = lengterol;
float sensorReadingsArr[9];

void IRAM_ATTR isr() {
  interrupt_time = millis();
  if (interrupt_time - prev_interrupt_time > 100) {
    count++;
    prev_interrupt_time = interrupt_time;
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
    lengterol = 250000;
  }
  http.end();
  return payload;
}


void setup() {
  Serial.begin(9600);
  pinMode (32,INPUT_PULLUP);
  attachInterrupt(32, isr, RISING);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    maxlengte = httpGETRequest(getservername);
    JSONVar myObject = JSON.parse(maxlengte);
    if (JSON.typeof(myObject) == "undefined") {
      lengterol = 250000;
      actlengte = 250000;
    }
    else {
     JSONVar keys = myObject.keys();
     for (int i = 0; i < keys.length(); i++) {
       JSONVar value = myObject[keys[i]];
       sensorReadingsArr[i] = double(value);
     } 
     lengterol = sensorReadingsArr[printernummer-1];
     actlengte = lengterol;
   }
  }
  server.begin();
}

void loop() {
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