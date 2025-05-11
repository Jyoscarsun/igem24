#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

const char *ssid = "UofT";
const char *username = "";
const char *password = "";
const char *base_rest_url = "https://us-east-2.aws.data.mongodb-api.com/app/data-kpoqbux/endpoint/data/v1/action/insertOne";
const char *AtlasAPIKey = "3rSblJ5na5qoa5rSS08qGeZMSzAnTsMbZsKKE58QOITtn1aJxNaE7gbohTBPI5fR";

char serverAddress[] = "us-east-2.aws.data.mongodb-api.com";  // server address
char IPAddress[] = "100.66.17.145";
int port = 443;
WiFiSSLClient client;
HttpClient http = HttpClient(client, serverAddress, port);

const long interval = 80000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  
  // Connection to Wifi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
    // Failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  http.setTimeout(20000);
}

void resetHttpClient() {
  client.stop();  // Close any open connections
  http = HttpClient(client, serverAddress, port);  // Reinitialize the HTTP client
}

void postReqRackWell(const char *Rack, int Well)
{
  DynamicJsonDocument payload (1024);
  payload["dataSource"] = "Wetlab-Fridge-Management";
  payload["database"] = "Wetlab-Fridge-Management"; // or "igemhardware" for the correct database. This is a trial-and-error database
  payload["collection"] = "Rack Well";
  payload["document"]["Rack"] = Rack;
  payload["document"]["Well"] = Well;

  String JSONText;
  size_t JSONlength = serializeJson(payload, JSONText);
  Serial.println(JSONText);

  bool success = false;
  int maxRetries = 3;
  int attempt = 0;

  while (!success && attempt < maxRetries) {
    resetHttpClient();  // Reset the HTTP client before each attempt

    http.beginRequest();
    http.post(base_rest_url, "application/json", JSONText);
    http.sendHeader("Content-Type", "application/json");
    http.sendHeader("apiKey", AtlasAPIKey);
    // http.beginBody();
    // http.print(JSONText);
    http.endRequest();

    int statusCode = http.responseStatusCode();
    String response = http.responseBody();

    if(int(statusCode) < 0){
      resetHttpClient();
    }

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if (statusCode >= 200 && statusCode < 300) {
      success = true;  // Successful request
    } else {
      Serial.println("Request failed, retrying...");
      attempt++;
      payload.clear();
      delay(5000);  // Wait before retrying
    }
  }
  if (!success) {
    Serial.println("Failed to send request after multiple attempts.");
  }

}


void loop() {
  Serial.println("Wait five seconds");
  delay(5000);   

  Serial.println("sending info over");
  const char *Rack = "C";
  int Well = 300;

  postReqRackWell(Rack, Well);

  // String contentType = "application/json";
  // String postData = "{\"dataSource\": \"mongodb-atlas\",";
  // postData += "\"database\": \"Wetlab-Fridge-Management\",";
  // postData += "\"collection\": \"Rack Well\",";
  // postData += "\"document\": {\"Rack\": \"" + String(Rack) + "\",";
  // postData += "\"Well\": " + String(Well) + "}}";

  // client.post( "/post", contentType, postData );
  // int statusCode = client.responseStatusCode();
  // Serial.print( "Status code: " );
  // Serial.println( statusCode );
  // String response = client.responseBody();
  // Serial.print( "Response: " );
  // Serial.println( response );

  // Serial.println( "Wait 30 seconds" );
  // delay( 3000 );
}
