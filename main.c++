#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- WiFi and MQTT Configuration ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_broker = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32_FloodDrone"; // Unique client ID for MQTT
const char* mqtt_topic_publish = "sensor/monitoramento"; // Topic to publish data to

// --- Sensor Pin Definitions ---
#define DHTPIN 13       // Data pin for DHT22 sensor
#define DHTTYPE DHT22   // Type of DHT sensor
#define LEDPIN 23       // LED pin (e.g., ESP32's built-in LED or an external one)
#define ECHO_PIN 19     // Echo pin for ultrasonic sensor
#define TRIGGER_PIN 18  // Trigger pin for ultrasonic sensor

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// --- Function Prototypes ---
void setup_wifi();
void reconnect_mqtt();

// --- Setup Function ---
void setup() {
  Serial.begin(115200); // Initialize serial communication

  // Initialize sensors and pins
  dht.begin();
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW); // Ensure LED is off initially

  // Connect to WiFi
  setup_wifi();

  // Configure MQTT client
  client.setServer(mqtt_broker, mqtt_port);
  // No callback set for client.setCallback because we are only publishing
}

// --- Main Loop Function ---
void loop() {
  // Ensure MQTT connection is active
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop(); // Required to maintain MQTT connection and process messages

  // --- Read Sensor Data ---

  // Ultrasonic sensor (distance)
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout for ~5m max distance
  int distance = duration / 58; // Convert duration to distance in cm

  // DHT22 sensor (humidity and temperature)
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if DHT sensor readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    // You might want to skip publishing if sensor data is invalid
    delay(2000);
    return;
  }

  // --- Print Sensor Data to Serial ---
  Serial.print("Umidade: ");
  Serial.print(humidity);
  Serial.print(" %, Temperatura: ");
  Serial.print(temperature);
  Serial.print(" °C, Distância: ");
  Serial.print(distance);
  Serial.println(" cm");

  // --- Alert Condition ---
  // Activate LED if humidity > 70% AND distance is between 1cm and 50cm
  // (distance > 0 check added to avoid false positives from duration=0)
  if (humidity > 70.0 && distance > 0 && distance < 50) {
    digitalWrite(LEDPIN, HIGH); // Turn LED on
    Serial.println("ALERT! High humidity and close distance detected.");
  } else {
    digitalWrite(LEDPIN, LOW); // Turn LED off
  }

  // --- Prepare JSON Payload for MQTT ---
  StaticJsonDocument<256> doc; // Allocate a JSON document
  doc["umidade"] = humidity;
  doc["temperatura"] = temperature;
  doc["distancia"] = distance;

  String jsonString;
  serializeJson(doc, jsonString); // Serialize JSON document to a string

  // --- Publish Data to MQTT ---
  Serial.print("Publicando dados MQTT no topico '");
  Serial.print(mqtt_topic_publish);
  Serial.print("': ");
  Serial.println(jsonString);

  if (client.publish(mqtt_topic_publish, jsonString.c_str())) {
    Serial.println("MQTT Publish successful!");
  } else {
    Serial.print("MQTT Publish failed, rc=");
    Serial.println(client.state());
  }

  delay(5000); // Wait 5 seconds before next reading/publish (adjust as needed)
}

// --- WiFi Connection Function ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- MQTT Reconnection Function ---
void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      // You can subscribe to topics here if needed, for example:
      // client.subscribe("sensor/commands");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}