#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>
#include <ArduinoMqttClient.h>
#include "login.h"

typedef unsigned long us;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
int value = 0;
WiFiSSLClient client;
MqttClient mqttClient(client);
const char rackTopic[] = "arduino/rack";
const char rfidTopic[]   = "arduino/rfid";
//const char pythonTopic[]  = "python/rfid";

const long interval = 80000;
unsigned long previousMillis = 0;
const long JSON_DOC_SIZE = 1024;

RTC_DS3231 rtc;

const int sensorPin[] = {A0, A1, A2, A3, A4, A5};
float distance[6];
float old_distance[6];
const int NO_SAMPLES = 20;
const float MCU_VOLTAGE = 5.0;

void mqttSend(String topic, String payload){
  mqttClient.beginMessage(topic);
  mqttClient.print(payload);
  mqttClient.endMessage();
}

void mqtt_parseSend(String topic, String JSONText){
  int total = 0;
  while(total < JSONText.length()){
    String parsed_text = "";
    for (int i=0; i < 128; i++){
      parsed_text += JSONText[total+i];
    }
    total += 128;
    Serial.println(parsed_text);
    mqttSend(topic, parsed_text);
  }
}

const float THRESHOLD = 4.8; // cm, confirm with Mariam/Sam what the threshold is

bool isThereAVial(int sensor){
  float avg = 0;
  int sensorValue;
  for (int i=0; i < NO_SAMPLES; i++){
    sensorValue = analogRead(sensorPin[sensor]);
    //Serial.println(sensorValue * (MCU_VOLTAGE / 1023.0));
    delay(1);
    avg += (float)sensorValue * (MCU_VOLTAGE / 1023.0);
  }

  avg /= NO_SAMPLES;

  distance[sensor] = 33.9 + -69.5*avg + 62.3*pow(avg, 2) + -25.4*pow(avg, 3) + 3.83*pow(avg, 4);
  Serial.println(distance[sensor]);
  int ret;

  if (distance[sensor] > THRESHOLD){ // distance reads till top of rack = NO VIAL
    ret = false;
    Serial.println("No vial");
  } else { // have vial here
    ret = true;
    Serial.println("Vial");
  }

  old_distance[sensor] = distance[sensor];
  return ret;
}

// This is the function called when the user places the sample in the fridge during registration and checking-in
void regcheckin_putsample(int RFID, char *Rack, int Well){
  DynamicJsonDocument payload1(1024);
  DynamicJsonDocument payload2(1024);

  payload1["filter"]["RFID no"] = RFID;
  char buffer[5];
  sprintf(buffer, "%s%d", Rack, Well);
  payload1["update"]["$set"]["Well no"] = buffer;

  payload2["filter"]["Rack"] = Rack;
  payload2["filter"]["Well"] = Well;
  payload2["update"]["$set"]["Occupied"] = true;
  payload2["update"]["$set"]["Occupied By"] = RFID;

  String JSONText1, JSONText2;
  size_t JSONlength1 = serializeJson(payload1, JSONText1);
  Serial.println(JSONText1);
  size_t JSONlength2 = serializeJson(payload2, JSONText2);
  Serial.println(JSONText2);

  Serial.println("parsed mqtt messages");
  mqtt_parseSend("python/dbupdate/RFID Chips", "[" + JSONText1 + "]");
  mqtt_parseSend("python/dbupdate/Rack Well", "[" + JSONText2 + "]");
}

// This is the function called when the user checks out a sample and have only operated on the user interface GUI end
void checkout_update(int RFID){
  //DateTime now = rtc.now();
  int milliseconds = 0;
  char curTime[30];
  //snprintf(curTime, sizeof(curTime), "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), milliseconds);
  
  DynamicJsonDocument payload1(1024);
  DynamicJsonDocument payload2(1024);
  payload1["filter"]["RFID no"] = RFID;
  payload1["update"]["$set"]["Well no"] = NULL;

  payload2["filter"]["Occupied By"] = RFID;
  payload2["update"]["$set"]["Occupied"] = false;
  payload2["update"]["$set"]["Occupied By"] = NULL;

  String JSONText1, JSONText2;
  size_t JSONlength1 = serializeJson(payload1, JSONText1);
  Serial.println(JSONText1);
  size_t JSONlength2 = serializeJson(payload2, JSONText2);
  Serial.println(JSONText2);

  //mqttSend("python/dbupdtate/updatemany", JSONText1);
  mqtt_parseSend("python/dbupdate/RFID Chips", "[" + JSONText1 + "]");
  mqtt_parseSend("python/dbupdate/Rack Well", "[" + JSONText2 + "]");
  //mqttSend("python/dbupdtate/updatemany", JSONText2);
}

String read_mqtt() {
  Serial.print("Message arrived on topic: ");
  Serial.print(mqttClient.messageTopic());
  String payload = "";

  Serial.print(". Message: ");
  while (mqttClient.available()){
    payload += String((char)mqttClient.read());
  }
  Serial.print(payload);
  return payload;
}

void callback(unsigned int length){
  // read topic
  String topic = mqttClient.messageTopic();

  // read payload
  String payload_mqtt = "";
  while (mqttClient.available()){
    payload_mqtt += String((char)mqttClient.read());
  }

  // trace
  Serial.println(topic);
  Serial.println(payload_mqtt);

  char *rack = "A"; //we will need a function or a diff topic to differentiate between racks A and B but for now I'll just default it here to avoid bugs

  // REGISTRATION
  if (topic == "arduino/rack/registration" \
      || topic == "arduino/rack/checkin") {
    // check for content of received message
    int rfid = payload_mqtt.toInt();
    bool movement_detected = false;

    us past = millis();
    while(movement_detected == false){
      us now = millis();
      if (now - past > 1000){
        for(int i = 0; i <= 1; i++){
          if(isThereAVial(i)){
            // top or bottom rack? will have to determine a way to differentiate
            int well = i;
            regcheckin_putsample(rfid, rack, well);

            movement_detected = true;
            break;
          }
          else{
            if(i == 1){
            movement_detected = true;
            }
          }
        }

      }
    }

  }

  if (topic == "arduino/checkout") {
    int rfid;
    char payload_rack[3]; // top or bottom rack ("A" or "B" in database)
    int payload_well; // well no.
    int well;
    // parse payload into int rfid, char *rack, int well; rfid/rack/well
    const char* payload2 = payload_mqtt.c_str();
    char* token = strtok(payload2, "/");
    rfid = atoi(token);
    token = strtok(NULL, "/");
    strcpy(payload_rack, token);
    token = strtok(NULL, "/");
    payload_well = atoi(token);

    bool movement_detected = false;
    bool correct_removal = false;
    us prev = millis();
    while(movement_detected == false){
      us now = millis();
      if (now - prev > 1000){
          if(!isThereAVial(payload_well)){
            Serial.println("still in loop1");
            movement_detected = true;
            correct_removal = true;
          }
          else{
            Serial.println("still in loop2");
            movement_detected = true;
          }
        }
      }
    if (strcmp(payload_rack, rack) != 0 || correct_removal == false){
      mqttSend("python/rack", "WRONG REMOVAL :(");
    }
    else{
      checkout_update(rfid);
    }
  }
  Serial.println("hi you mdae it out of the loop");
}

void reconnect() {
  // Loop until we’re reconnected
  us past = millis();
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");

    mqttClient.setUsernamePassword(mqtt_username, mqtt_password);
    mqttClient.onMessage(callback);

    if (!mqttClient.connect(mqtt_server, 8883)){

      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      us now = millis();
        if (now - past > 5000){
          past = now;
        }

    } else {

      mqttClient.subscribe("arduino/rack/registration");
      mqttClient.subscribe("arduino/rack/checkin");
      mqttClient.subscribe("arduino/checkout");
      // any additional topics here...

    }
  }
}


void setup() {
  // If the script fails here or during any analogRead let Kenneth immediately know.
  for (int i=0; i<6; i++){
    pinMode(sensorPin[i], INPUT);
  }

  Serial.begin(9600);

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  us past = millis();
  while(WiFi.begin(ssid, password) != WL_CONNECTED){
  //while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
    us now = millis();
    if (now - past > 5000){
      Serial.print(".");
      past = now;
    }
  }
  Serial.println("You're connected to the network");

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  mqttClient.setUsernamePassword(mqtt_username, mqtt_password);
  mqttClient.onMessage(callback);

  if (!mqttClient.connect(mqtt_server, 8883)){
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  mqttClient.subscribe("arduino/rack/registration");
  mqttClient.subscribe("arduino/rack/checkin");
  mqttClient.subscribe("arduino/checkout");
  // any additional topics here...
}

void loop() {
  // put your main code here, to run repeatedly:

  mqttClient.poll();



  reconnect();
}
