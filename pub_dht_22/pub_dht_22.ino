#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// DHT sensor
#define DHTTYPE DHT22
#define DHTPIN 4

const char* wifi_ssid = "add_value";
const char* wifi_password = "add_value";

const char* mqtt_server = "add_value";
const char* mqtt_user = "add_value";
const char* mqtt_password = "add_value";
const char* mqtt_client_name = "ESP8266BedroomClient";

const char* humidity_topic = "home/env_sensor/humidity";
const char* temperature_topic = "home/env_sensor/temperature";
const long publish_interval = 5000; // 5 seconds in millis

WiFiClient espClient;
PubSubClient client(espClient);

float temperature;
float humidity;

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP8266 IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempt to connect to mqtt server " + String(mqtt_server));
    if (client.connect(mqtt_client_name, mqtt_user, mqtt_password)) {
      Serial.println("Connected");
    } else {
      Serial.print(".");
      delay(5000);
    }
  }
}

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  
  dht.begin();
  
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

long now = millis();
long lastMeasure = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  now = millis();
  // publish every second
  if (now - lastMeasure > publish_interval) {
    lastMeasure = now;
    
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Warning: failed to read from DHT sensor");
      return;
    }

    Serial.println("Publishing events for temperature [" + String(temperature) + "] and humidity [" + String(humidity) + "]");

    client.publish(temperature_topic, String(temperature).c_str(), true);
    client.publish(humidity_topic, String(humidity).c_str(), true);

    Serial.println("Finished publishing events");
  }
}
