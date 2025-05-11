#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <RTCZero.h>
#include <ArduinoMqttClient.h>
#include <MFRC522.h>
#include <SPI.h>
#include "login.h"

typedef unsigned long us;

WiFiSSLClient client;
MqttClient mqttClient(client);
const int maxRFIDs = 50;
long rfidNumbers[maxRFIDs]; // Array to store the RFID numbers
int numRFIDs = 0; // Index of RFID in the array

const char rackTopic[] = "arduino/rack";
const char rfidTopic[]   = "arduino/rfid";
const char pythonrfidTopic[]  = "python/rfid";
const char rfidrackTopic[] = "arduino/rack/checkin";

RTCZero rtc;

/*
RFID module wiring schema:
MKR      MFRC522
6     -> RST
7     -> SDA
8     -> MOSI
10    -> MISO
9     -> SCK
GND   -> GND
3.3V  -> VCC/Vin
*/
MFRC522 mfrc522(7, 6);
MFRC522::MIFARE_Key key;
byte readbackblock[18];

void mqttSend(String topic, String payload){
  mqttClient.beginMessage(topic);
  mqttClient.print(payload);
  mqttClient.endMessage();
}

void mqtt_parseSend(String topic, String JSONText){
  int total = 0;
  String parsed_text = "";
  while(total < JSONText.length()){
    parsed_text += JSONText[total];
    total++;
    
    if (total%128==0 | JSONText[total]==']'){
      mqttSend(topic, parsed_text);
      parsed_text = "";
    }
  }
}

int readRFID (byte arrayAddress[]) {
  /*
  Reads the identification number (as we determine it) of the RFID.
  All identification numbers are stored in block 2, zeroth member.
  */
  const int block_number = 2;
  const int trailer_block = 3;

  while (!mfrc522.PICC_IsNewCardPresent()){}
  while (!mfrc522.PICC_ReadCardSerial()){}

  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 3, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
    return -1;
  }

  Serial.println("Current data in sector:");
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, 0);

  byte buffer_size = 18;
  status = mfrc522.MIFARE_Read(block_number, arrayAddress, &buffer_size);
  if (status != MFRC522::STATUS_OK){
    return -1;
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return (int) arrayAddress[0];
}

// Checks whether or not an RFID is in memory, if not return false
bool addRFID(long rfid) {
  for(int i = 0; i < numRFIDs; i++){
    if(rfidNumbers[i] == rfid){
      return false;
    }
  }
  if (numRFIDs < maxRFIDs) {
    rfidNumbers[numRFIDs] = rfid;
    numRFIDs++;
    return true;
  }
  else {
    return false;
  }
}

bool rfidExist(long rfid){
  for(int i = 0; i < numRFIDs; i++){
    if(rfidNumbers[i] == rfid){
      return true;
    }
  }
  return false;
}

// This is the function called when the user taps the RFID using the scanner during registration
String registration_RFIDtap(int RFID){
  int milliseconds = 0;
  char curTime[30];

  snprintf(curTime, sizeof(curTime), "20%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), milliseconds);

  DynamicJsonDocument payload_db(1024);
  payload_db["filter"]["RFID no"] = RFID;
  payload_db["update"]["$set"]["In Use?"] = true;
  payload_db["update"]["$set"]["Last tapped time"]["$date"] = curTime;
  payload_db["update"]["$set"]["Registration time"]["$date"] = curTime;

  String JSONText;
  size_t JSONlength = serializeJson(payload_db, JSONText);
  Serial.println("[" + JSONText + "]");

  mqtt_parseSend("python/dbupdate/RFID Chips", "[" + JSONText + "]");
  return JSONText;
}


// This is the function called when the user checks in the RFID using the scanner during registration
String checkin_RFIDtap(int RFID){
  int milliseconds = 0;
  char curTime[30];

  snprintf(curTime, sizeof(curTime), "20%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), milliseconds);

  DynamicJsonDocument payload_db(1024);
  payload_db["filter"]["RFID no"] = RFID;
  payload_db["update"]["$set"]["In Use?"] = true;
  payload_db["update"]["$set"]["Last tapped time"]["$date"] = curTime;

  String JSONText;
  size_t JSONlength = serializeJson(payload_db, JSONText);
  Serial.println("[" + JSONText + "]");

  mqtt_parseSend("python/dbupdate/RFID Chips", "[" + JSONText + "]");
  return JSONText;
}

// This is the function called when the user scans the RFID during the check-out process
String checkout_RFIDtap(int RFID){
  int milliseconds = 0;
  char curTime[30];
  snprintf(curTime, sizeof(curTime), "20%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), milliseconds);

  DynamicJsonDocument payload_db(1024);
  payload_db["filter"]["RFID no"] = RFID;
  payload_db["update"]["$set"]["Last tapped time"]["$date"] = curTime;

  String JSONText;
  size_t JSONlength = serializeJson(payload_db, JSONText);
  Serial.println("[" + JSONText + "]");

  mqtt_parseSend("python/dbupdate/RFID Chips", "[" + JSONText + "]");
  return JSONText;
}

void callback(int length) {
  Serial.println("callback fired");

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
  if (topic == "arduino/rfid") {
    // check for content of received message
    if (payload_mqtt == "REGISTRATION") { // User clicks on GUI to register
      // priming the RIFD reader to read
      int rfid = readRFID(readbackblock);// A function that obtains the RFID value
      if(addRFID(rfid) == false){
        mqttSend("python/rfid", "RFID REGISTERED");
      }
      else{
        registration_RFIDtap(rfid);
        String rfid_str = String(rfid);
        mqttSend("python/rfid", rfid_str);
      }
    }
    else if (payload_mqtt == "CHECK-IN") {
      // priming the RFID reader to read
      int rfid = readRFID(readbackblock);
      if (rfidExist(rfid) == false){
        mqttSend("python/rfid", "The RFID has not been registered yet.");
      }
      else{
        checkin_RFIDtap(rfid);
        String rfid_str = String(rfid);
        mqttSend("arduino/rack/checkin", rfid_str);
      }
    }
    else if (payload_mqtt == "DEREGISTRATION"){
      // priming the RFID reader to read
      int rfid = readRFID(readbackblock);
      String rfid_str = String(rfid);
      mqttSend("python/rfid", rfid_str);
    }
  }

  if(topic == "arduino/checkout"){
    // priming the RFID reader to read
    int rfid = readRFID(readbackblock);
    checkout_RFIDtap(rfid);
    String rfid_str = String(rfid);
    mqttSend("python/rfid", rfid_str);
  }
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
    } 
    
    else {
      mqttClient.subscribe("arduino/rfid");
      mqttClient.subscribe("arduino/checkout");
    }
  }
}

byte cheese[6] = {0,0,0,0,0,0};
void dateparser(byte block[]){
  const char date[] = __DATE__;
  Serial.println(date);
  const char time[] = __TIME__;

  byte year = (date[9]-48)*10 + (date[10]-48);
  byte dat = (date[4]-48)*10 + date[5]-48;
  char mon_str[3] = {(char) date[0], (char) date[1], (char) date[2]}; byte mon;
  if (mon_str == "Jan") mon = 1;
  else if (mon_str == "Feb") mon = 2;
  else if (mon_str == "Mar") mon = 3;
  else if (mon_str == "Apr") mon = 4;
  else if (mon_str == "May") mon = 5;
  else if (mon_str == "Jun") mon = 6;
  else if (mon_str == "Jul") mon = 7;
  else if (mon_str == "Aug") mon = 8;
  else if (mon_str == "Sep") mon = 9;
  else if (mon_str == "Oct") mon = 10;
  else if (mon_str == "Nov") mon = 11;
  else if (mon_str == "Dec") mon = 12;

  byte hour = (time[0]-48)*10 + (time[1]-48);
  byte min = (time[3]-48)*10 + (time[4]-48);
  byte sec = (time[6]-48)*10 + (time[7]-48);

  block[0] = year;
  block[1] = mon;
  block[2] = dat;
  block[3] = hour;
  block[4] = min;
  block[5] = sec;

  Serial.println(block[0]);
}

void setup() {
  SPI.begin();
  Serial.begin(9600);

  delay(5000);

  rtc.begin();
  dateparser(cheese);
  rtc.setDate(cheese[2], cheese[1], cheese[0]);
  rtc.setTime(cheese[5], cheese[4], cheese[3]);

  Serial.println(rtc.getYear());
  // Set up the timer

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  us past = millis();
//  while(WiFi.begin(ssid, password) != WL_CONNECTED){
  while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
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

  mqttClient.subscribe("arduino/rfid");
  mqttClient.subscribe("arduino/checkout");

  // Setup MFRC522 unit
  mfrc522.PCD_Init();
  for (byte i=0; i<6; i++){
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Successful subscriptions");
}

void loop() {
  static us past = 0;
  us now = millis();
  if (now - past > 5000){
      past = now;
  }
  mqttClient.poll();

  reconnect();
}
