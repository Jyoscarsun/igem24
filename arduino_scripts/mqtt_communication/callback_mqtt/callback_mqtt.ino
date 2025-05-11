#include <ArduinoMQTTClient.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
const char* ssid = "your_wifi_name";
const char* password = "your_wifi_password";
const char* mqtt_server = "ddaa1f93a61a4de7aaf7465c2984155c.s1.eu.hivemq.cloud";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }

  while(WiFi.beginEnterprise(ssid, username, password) != WL_CONNECTED){
    // Failed, retry
  Serial.print(".");
  delay(5000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //pinmode stuff to get prox readings
}


void setDateTime() {
  // You can use your own timezone, but the exact time is not used at all.
  // Only the date is needed for validating the certificates.
  configTime(TZ_Europe_Berlin, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(100);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if the first character is present
  if ((char)payload[0] != NULL) {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  } else {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH

    if (String(topic) == "Arduino/Rack") {
      Serial.print("Changing output to ");
      // check for content of received message
      if (messageTemp == "OCCUPIED") {
        Serial.println("OCCUPIED");
        // post/update on database
      }
      else if (messageTemp == "UNOCCUPIED") {
        Serial.println("UNOCCUPIED");
        // post/update on database
        // OR .. if messageTemp != the stored occupation variable ... will have to figure out how to do this
      }
    }
  }
}


void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");

    mqtt_client.setUsernamePassword("USER PLACEHOLDER", "PASSWORD PLACEHOLDER");
    mqtt_client.onMessage(callback);

    if (!mqtt_client.connect(mqtt_server, 8883)){

      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      delay(5000);

    } else {

      mqtt_client.subscribe("TOPIC PLACEHOLDER")
      // any additional topics here...

    }
  }
}

WifiSSLClient mqtt_wifi_client;
WifiSSLClient db_wifi_client;

MqttClient mqtt_client(mqtt_wifi_client);

void setup() {
  delay(500);
  // When opening the Serial Monitor, select 9600 Baud
  Serial.begin(9600);
  while (!Serial){;}
  delay(500);

  setup_wifi();
  setDateTime();

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  mqtt_client.setUsernamePassword("USER PLACEHOLDER", "PASSWORD PLACEHOLDER");
  mqtt_client.onMessage(callback);

  if (!mqtt_client.connect(mqtt_server, 8883)){
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  mqtt_client.subscribe("TOPIC PLACEHOLDER")
  // any additional topics here...


}

void loop() {
  mqtt_client.poll();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client->publish("testTopic", msg);
  }

  reconnect();
}
