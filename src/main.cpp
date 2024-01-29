#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define DIAMETER 40

WiFiServer server(80);
String header;
int count = 0;
unsigned long interrupt_time;
unsigned long prev_interrupt_time;
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
  Serial.begin(115200);
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
WiFiClient client = server.available();   
  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
           // String buffer;
           // client.println("<p>Count =" + count + "test</p>");
            client.println(afstand(count));
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
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

String afstand(int revolutions) {
  if (lengterol <= 0) {
    return ("error");
  }
  else {
    char str[9];
    int buffer = lengterol - (DIAMETER * 3.1415 * revolutions);
    sprintf(str, "%d", buffer);
    Serial.println(revolutions);
    Serial.println(str);
    return (str);
  }
}