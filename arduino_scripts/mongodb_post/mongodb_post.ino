#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>

const char *ssid = "";
const char *username = "";
const char *password = "";
const char *base_rest_url = "https://us-east-2.aws.data.mongodb-api.com/app/data-kpoqbux/endpoint/data/v1/action/insertOne";
const char *AtlasAPIKey = "3rSblJ5na5qoa5rSS08qGeZMSzAnTsMbZsKKE58QOITtn1aJxNaE7gbohTBPI5fR";

char serverAddress[] = "us-east-2.aws.data.mongodb-api.com";  // server address
int port = 443;
WiFiSSLClient client;
HttpClient http = HttpClient(client, serverAddress, port);

RTC_DS3231 rtc;

const long interval = 80000;
unsigned long previousMillis = 0;


void setup() {
  Serial.begin(9600);
  
  // Connection to Wifi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  // while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
  while(WiFi.begin(ssid, password) != WL_CONNECTED){
    // Failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    // Following lines set the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void postReqRackWell(const char *Rack, int Well)
{
  DateTime now = rtc.now();
  int milliseconds = 0;
  char curTime[30];

  snprintf(curTime, sizeof(curTime), "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), milliseconds);

  DynamicJsonDocument payload (1024);
  payload["dataSource"] = "Wetlab-Fridge-Management";
  payload["database"] = "Wetlab-Fridge-Management"; // or "igemhardware" for the correct database. This is a trial-and-error database
  payload["collection"] = "Rack Well";
  payload["document"]["Rack"] = Rack;
  payload["document"]["Well"] = Well;

  // Use Extended JSON format for the date
  JsonObject dateObject = payload["document"].createNestedObject("Time");
  dateObject["$date"] = curTime;

  String JSONText;
  size_t JSONlength = serializeJson(payload, JSONText);
  Serial.println(JSONText);

  http.beginRequest();
  http.post(base_rest_url, "application/json", JSONText);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  // http.beginBody();
  // http.print(JSONText);
  http.endRequest();

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  Serial.println("Wait five seconds");
  delay(5000);
}


void loop() {
  // Serial.println("Wait five seconds");
  // delay(5000);   

  Serial.println("sending info over");
  char *Rack = "C";
  int Well = 1500;

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
