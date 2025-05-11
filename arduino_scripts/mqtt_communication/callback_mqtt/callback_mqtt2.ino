#include <ArduinoMqttClient.h>
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

// Update these with values suitable for your network.
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
int value = 0;
WiFiSSLClient client;
MqttClient mqttClient(client);

const char rackTopic[] = "arduino/rack";
const char rfidTopic[]   = "arduino/rfid";
const char threeTopic[]  = "arduino/three";

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
  //   // Failed, retry
  // Serial.print(".");
  // delay(5000);
  // }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //pinmode stuff to get prox readings
}

String read_mqtt() {
  Serial.print("Message arrived on topic: ");
  Serial.print(mqttClient.messageTopic());

  Serial.print(". Message: ");
  while (mqttClient.available()){
    payload += String((char)mqttClient.read());
  }
    Serial.print(payload);
}

void callback(unsigned int length) {
  read_mqtt();
  // Switch on the LED if the first character is present
  if ((char)payload[0] != NULL) {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  } else {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH

    if (String(rackTopic) == "arduino/rack") {
      // check for content of received message
      if (payload == "OCCUPIED") {
        Serial.println("OCCUPIED");
        // post/update on database
      }
      else if (payload == "UNOCCUPIED") {
        Serial.println("UNOCCUPIED");
        // post/update on database
        // OR .. if payload != the stored occupation variable ... will have to figure out how to do this
      }
    }
  }
}


void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");

    mqttClient.setUsernamePassword("", "");
    mqttClient.onMessage(callback);

    if (!mqttClient.connect(mqtt_server, 8883)){

      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      delay(5000);

    } else {

      mqttClient.subscribe("TOPIC PLACEHOLDER");
      // any additional topics here...

    }
  }
}

void setup() {
  delay(500);
  // When opening the Serial Monitor, select 9600 Baud
  Serial.begin(9600);
  while (!Serial){;}
  delay(500);

  setup_wifi();
  //setDateTime();

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  mqttClient.setUsernamePassword("", "");
  mqttClient.onMessage(callback);

  if (!mqttClient.connect(mqtt_server, 8883)){
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  mqttClient.subscribe("testTopic");
  // any additional topics here...
}

void loop() {
  delay(5000);
  mqttClient.poll();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println("msg");

    mqttClient.beginMessage("testTopic");
    mqttClient.print("msg");
    mqttClient.endMessage();
  }

  reconnect();
}
