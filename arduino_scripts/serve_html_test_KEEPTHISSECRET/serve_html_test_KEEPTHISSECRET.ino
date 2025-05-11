#include <WiFiNINA.h>

char ssid[] = "UofT";
char name[] = "YOUR_UTOR_HERE";             //  your network SSID (name) between the " "
char pass[] = "YOUR_PASS_HERE";      // your network password between the " "

int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket
WiFiClient client = server.available();

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.beginEnterprise(ssid, name, pass);
    // wait 10 seconds for connection:
    delay(2500);
  }

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  server.begin();
}


void loop() {
  client = server.available();

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client

    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor

        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:

          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
         
            //create the buttons
            client.print("Serving html from the iGEM Arduino Wifi Rev2!");

            // The HTTP response ends with another blank line:
            client.println();
            break;
          }

          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }

        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}