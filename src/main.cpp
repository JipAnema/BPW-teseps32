#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define DIAMETER 40

WiFiServer server(80);
String header;
int count = 0;
int prevcount = -1;
unsigned long interrupt_time;
unsigned long prev_interrupt_time;
String servername = "http://192.168.137.1:1880/update-sensor";
String afstand(int revolutions);
const char* ssid     = "Test";
const char* password = "123456789";
unsigned int lengterol = 250000;

void IRAM_ATTR isr() {
  interrupt_time = millis();
  if (interrupt_time - prev_interrupt_time > 250) {
    count++;
    prev_interrupt_time = interrupt_time;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode (32,INPUT);
  attachInterrupt(32, isr, RISING);
  WiFi.begin(ssid,password);
 // WiFi.softAP(ssid,password);
 // IPAddress IP = WiFi.softAPIP();
 // Serial.println(IP);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  server.begin();
}
void loop() {
if (false) {
WiFiClient client = server.available();   
  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println(afstand(count));
            client.println("</body></html>");
            client.println();
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
  }
  else {
     if(WiFi.status() == WL_CONNECTED) {
       WiFiClient client;
       HTTPClient http;
       // Serial.println(client);
      if (count != prevcount) {
        http.begin(client, servername);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpSendData = "lengte rol 1: " + afstand(count);
        int httpResponseCode = http.POST(httpSendData);
        Serial.println(httpResponseCode);
        http.end();
        prevcount = count;
       }
    }
  }
}

String afstand(int revolutions) {
  if (lengterol <= 0 || lengterol > 250000) {
    return ("error");
  }
  else {
    char str[9];
    int buffer = lengterol - (DIAMETER * 3.1415 * revolutions);
    sprintf(str, "%d", buffer);
   // Serial.println(str);
    return (str);
  }
}
