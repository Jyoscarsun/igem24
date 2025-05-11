#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>

const char *ssid = "";
const char *username = "";
const char *password = "";
const char *base_rest_url = "https://us-east-2.aws.data.mongodb-api.com/app/data-kpoqbux/endpoint/data/v1/action";
const char *AtlasAPIKey = "3rSblJ5na5qoa5rSS08qGeZMSzAnTsMbZsKKE58QOITtn1aJxNaE7gbohTBPI5fR";

char serverAddress[] = "us-east-2.aws.data.mongodb-api.com";
int port = 443;
WiFiSSLClient client;
HttpClient http = HttpClient(client, serverAddress, port);

const long interval = 80000;
unsigned long previousMillis = 0;
const long JSON_DOC_SIZE = 1024;

// RTC clock currently does not work yet
RTC_DS3231 rtc;

// This is the function called when the user taps the RFID using the scanner during registration
void registration_RFIDtap(int RFID){
  char rest_api_url[200];
  sprintf(rest_api_url, "%s/updateOne", base_rest_url);
  DateTime now = rtc.now();
  int milliseconds = 0;
  char curTime[30];

  snprintf(curTime, sizeof(curTime), "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), milliseconds);

  DynamicJsonDocument payload(1024);
  payload["dataSource"] = "Wetlab-Fridge-Management";
  payload["database"] = "igemhardware";
  payload["collection"] = "RFID Chips";
  payload["filter"]["RFID no"] = RFID;
  payload["update"]["$set"]["In Use?"] = true;
  payload["update"]["$set"]["Last tapped time"]["$date"] = curTime;
  payload["update"]["$set"]["Registration time"]["$date"] = curTime;

  String JSONText;
  size_t JSONlength = serializeJson(payload, JSONText);
  Serial.println(JSONText);

  http.beginRequest();
  http.post(rest_api_url, "application/json", JSONText);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest;

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}


// This is the function called when the user checks in the RFID using the scanner during registration
void checkin_RFIDtap(int RFID){
  char rest_api_url[200];
  sprintf(rest_api_url, "%s/updateOne", base_rest_url);
  DateTime now = rtc.now();
  int milliseconds = 0;
  char curTime[30];

  snprintf(curTime, sizeof(curTime), "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), milliseconds);

  DynamicJsonDocument payload(1024);
  payload["dataSource"] = "Wetlab-Fridge-Management";
  payload["database"] = "igemhardware";
  payload["collection"] = "RFID Chips";
  payload["filter"]["RFID no"] = RFID;
  payload["update"]["$set"]["In Use?"] = true;
  payload["update"]["$set"]["Last tapped time"]["$date"] = curTime;

  String JSONText;
  size_t JSONlength = serializeJson(payload, JSONText);
  Serial.println(JSONText);

  http.beginRequest();
  http.post(rest_api_url, "application/json", JSONText);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest;

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

// This is the function called when the user places the sample in the fridge during registration and checking-in
void regcheckin_putsample(int RFID, char *Rack, int Well){
  char rest_api_url[200];
  sprintf(rest_api_url, "%s/updateMany", base_rest_url);
  DynamicJsonDocument payload1(1024);
  DynamicJsonDocument payload2(1024);

  payload1["dataSource"] = "Wetlab-Fridge-Management";
  payload1["database"] = "igemhardware";
  payload1["collection"] = "RFID Chips";
  payload1["filter"]["RFID no"] = RFID;
  char buffer[5];
  sprintf(buffer, "%s%d", Rack, Well);
  payload1["update"]["$set"]["Well no"] = buffer;

  payload2["dataSource"] = "Wetlab-Fridge-Management";
  payload2["database"] = "igemhardware";
  payload2["collection"] = "Rack Well";
  payload2["filter"]["Rack"] = Rack;
  payload2["filter"]["Well"] = Well;
  payload2["update"]["$set"]["Occupied"] = true;
  payload2["update"]["$set"]["Occupied By"] = RFID;

  String JSONText1, JSONText2;
  size_t JSONlength1 = serializeJson(payload, JSONText1);
  Serial.println(JSONText1);
  size_t JSONlength2 = serializeJson(payload, JSONText2);
  Serial.println(JSONText2);

  http.beginRequest();
  http.post(rest_api_url, "application/json", JSONText1);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest;

  http.beginRequest();
  http.post(rest_api_url, "application/json", JSONText2);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest;

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

// This is the function called when the user checks out a sample and have only operated on the user interface GUI end
void checkout_update(int RFID){
  DateTime now = rtc.now();
  int milliseconds = 0;
  char curTime[30];
  snprintf(curTime, sizeof(curTime), "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), milliseconds);
  
  char rest_api_url[200];
  sprintf(rest_api_url, "%s/updateMany", base_rest_url);
  DynamicJsonDocument payload1(1024);
  DynamicJsonDocument payload2(1024);
  payload1["dataSource"] = "Wetlab-Fridge-Management";
  payload1["database"] = "igemhardware";
  payload1["collection"] = "RFID Chips";
  payload1["filter"]["RFID no"] = RFID;
  payload1["update"]["$set"]["Well no"] = NULL;

  payload2["dataSource"] = "Wetlab-Fridge-Management";
  payload2["database"] = "igemhardware";
  payload2["collection"] = "Rack Well";
  payload2["filter"]["Occupied By"] = RFID;
  payload2["update"]["$set"]["Occupied"] = false;
  payload2["update"]["$set"]["Occupied By"] = NULL;

    String JSONText1, JSONText2;
  size_t JSONlength1 = serializeJson(payload, JSONText1);
  Serial.println(JSONText1);
  size_t JSONlength2 = serializeJson(payload, JSONText2);
  Serial.println(JSONText2);

  http.beginRequest();
  http.post(rest_api_url, "application/json", JSONText1);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest;

  http.beginRequest();
  http.post(rest_api_url, "application/json", JSONText2);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest;

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

// This is the function called when the user scans the RFID during the check-out process
void checkout_RFIDtap(int RFID){
  DateTime now = rtc.now();
  int milliseconds = 0;
  char curTime[30];
  snprintf(curTime, sizeof(curTime), "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), milliseconds);
  
  char rest_api_url[200];
  sprintf(rest_api_url, "%s/updateOne", base_rest_url);

  DynamicJsonDocument payload(1024);
  payload["dataSource"] = "Wetlab-Fridge-Management";
  payload["database"] = "igemhardware";
  payload["collection"] = "RFID Chips";
  payload["filter"]["RFID no"] = RFID;
  payload["update"]["$set"]["Last tapped time"]["$date"] = curTime;

  String JSONText;
  size_t JSONlength = serializeJson(payload, JSONText);
  Serial.println(JSONText);

  http.beginRequest();
  http.post(rest_api_url, "application/json", JSONText);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("apiKey", AtlasAPIKey);
  http.endRequest;

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}



void setup() {
  Serial.begin(9600);

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while(WiFi.begin(ssid, password) != WL_CONNECTED){
  // while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
    Serial.print(".");
    delay(5000);
  }
  Serial.println("You're connected to the network");

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // Set up the timer
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

void loop() {


}
