#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

const char *ssid = "UofT";
const char *username = "";
const char *password = "";
const char *base_rest_url = "https://us-east-2.aws.data.mongodb-api.com/app/data-kpoqbux/endpoint/data/v1/action/findOne";
const char *AtlasAPIKey = "3rSblJ5na5qoa5rSS08qGeZMSzAnTsMbZsKKE58QOITtn1aJxNaE7gbohTBPI5fR";

char serverAddress[] = "us-east-2.aws.data.mongodb-api.com";  // server address
int port = 443;
WiFiSSLClient client;
HttpClient http = HttpClient(client, serverAddress, port);

const long interval = 80000;
unsigned long previousMillis = 0;
const long JSON_DOC_SIZE = 1024;

void setup() {
  Serial.begin(9600);

  // Connection to Wifi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  // while(WiFi.begin(ssid, password) != WL_CONNECTED){
  while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
    // Failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
}


StaticJsonDocument<JSON_DOC_SIZE> getRackWell(int Well)
{
  DynamicJsonDocument payload (1024);
  payload["dataSource"] = "Wetlab-Fridge-Management";
  payload["database"] = "Wetlab-Fridge-Management"; // or "igemhardware" for the correct database. This is a trial-and-error database
  payload["collection"] = "Rack Well";
  payload["filter"]["Well"] = Well;

  String JSONText;
  size_t JSONlength = serializeJson(payload, JSONText);
  Serial.println(JSONText);

  http.beginRequest();
  http.post(base_rest_url, "application/json", JSONText);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest();


  int statusCode = http.responseStatusCode();
  String response = http.responseBody();
  StaticJsonDocument<JSON_DOC_SIZE> responseDoc;
  DeserializationError error = deserializeJson(responseDoc, response);
  serializeJson(responseDoc, Serial);
  Serial.println();
  return responseDoc;


  // Serial.println(statusCode);
  // if(statusCode > 0){
  //   String response = http.responseBody();
  //   Serial.print("Status code: ");
  //   Serial.println(statusCode);
  //   Serial.print("Response: ");
  //   Serial.println(response);

  //   StaticJsonDocument<JSON_DOC_SIZE> responseDoc;
  //   DeserializationError error = deserializeJson(responseDoc, response);

  //   if (error) {
  //     Serial.print("deserializeJson() failed: ");
  //     Serial.println(error.c_str());
  //     return;
  //   }
  //   serializeJson(responseDoc, Serial);
  //   Serial.println();
  //   return responseDoc;
  // }
}


void loop() {
  Serial.println("Wait five seconds");
  delay(5000);   

  Serial.println("Displaying rack well info");
  StaticJsonDocument<JSON_DOC_SIZE> responseDoc = getRackWell(300);
  serializeJson(responseDoc, Serial);
  Serial.println();
}
