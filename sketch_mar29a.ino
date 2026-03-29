#include <WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h> // Library for reliable HC-SR04 ultrasonic sensor operation

// --- WiFi Credentials ---
const char* ssid = "Sammy";
const char* password = "cactusjuice101";

// --- MQTT Broker Details ---
const char* mqtt_server = "10.219.27.26"; // e.g., IP of your Node-RED server
const char* mqtt_client_id = "DriveThruESP32";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


// --- Pin Definitions (Adjust as needed for your ESP32 board) ---

// Kiosk 1 Sensors
#define IR1_PIN 26           // GPIO 15 for Kiosk 1 Customer IR sensor
#define TRIG1_PIN 12         // GPIO 13 for Kiosk 1 Ultrasonic Trigger
#define ECHO1_PIN 13         // GPIO 12 for Kiosk 1 Ultrasonic Echo
#define MAX_DISTANCE 200     // Max distance for ultrasonic sensor (cm)
NewPing sonar1(TRIG1_PIN, ECHO1_PIN, MAX_DISTANCE);

// Kiosk 2 Sensors
#define IR2_PIN 25           // GPIO 14 for Kiosk 2 Customer IR sensor
#define TRIG2_PIN 27         // GPIO 27 for Kiosk 2 Ultrasonic Trigger
#define ECHO2_PIN 14         // GPIO 26 for Kiosk 2 Ultrasonic Echo
NewPing sonar2(TRIG2_PIN, ECHO2_PIN, MAX_DISTANCE);
#define LDR_PIN 15           // GPIO 34 (ADC1_CH6) for LDR sensor

// Motor Driver Pins (using L298N/L293D, adjust based on your wiring)
#define M1_IN1 33           // Main Gate Motor N20 motor 1
#define M1_IN2 32
#define M2_IN1 35          // Kiosk 2 Gate Motor N20 motor 2
#define M2_IN2 34
#define enA 22
#define enB 23

// --- MQTT Topics ---
#define TOPIC_PUBLISH_KIOSK1_CUST "drive_thru/kiosk1/customer_present"
#define TOPIC_PUBLISH_KIOSK1_CARS "drive_thru/kiosk1/cars_in_line"
#define TOPIC_PUBLISH_KIOSK2_CUST "drive_thru/kiosk2/customer_present"
#define TOPIC_PUBLISH_KIOSK2_CARS "drive_thru/kiosk2/cars_in_line"
#define TOPIC_PUBLISH_LDR "drive_thru/ambient_light"

#define TOPIC_SUBSCRIBE_M1_CMD "drive_thru/motor1/command"
#define TOPIC_SUBSCRIBE_M2_CMD "drive_thru/motor2/command"

// --- Function Declarations ---
void setup_wifi();
void reconnect_mqtt();
void callback(char* topic, byte* payload, unsigned int length);
float readUltrasonic(NewPing* sonar);
void controlMotor(int in1Pin, int in2Pin, bool open);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Initialize sensors pins
  pinMode(IR1_PIN, INPUT);
  pinMode(IR2_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  // Ultrasonic pins handled by NewPing library

  pinMode(enA,OUTPUT);
  pinMode(enB,OUTPUT);

  // Initialize motor driver pins
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastMsg > 5000) { // Publish data every 5 seconds
    lastMsg = currentMillis;

    // Read sensor data
    int ir1_state = digitalRead(IR1_PIN);
    float dist1 = readUltrasonic(&sonar1);
    int ir2_state = digitalRead(IR2_PIN);
    float dist2 = readUltrasonic(&sonar2);
    int ldr_value = analogRead(LDR_PIN);

    // Publish data to Node-RED
    client.publish(TOPIC_PUBLISH_KIOSK1_CUST, ir1_state == LOW ? "CUSTOMER_PRESENT" : "NO_CUSTOMER");
    
    // Convert distance to car count (example logic, refine in Node-RED)
    // Assume < 50cm means 3 cars are in line (adjust distance logic based on testing)
    client.publish(TOPIC_PUBLISH_KIOSK1_CARS, dist1 < 50 ? "3" : "0"); 
    
    client.publish(TOPIC_PUBLISH_KIOSK2_CUST, ir2_state == LOW ? "CUSTOMER_PRESENT" : "NO_CUSTOMER");
    client.publish(TOPIC_PUBLISH_KIOSK2_CARS, dist2 < 50 ? "3" : "0");
    
    client.publish(TOPIC_PUBLISH_LDR, String(ldr_value).c_str());

    Serial.println("Data published to MQTT broker.");
  }
}

// Function to control DC motor (e.g. L298N motor driver)
void controlMotor(int in1Pin, int in2Pin, bool open, int en) {
    if (open) {
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, LOW);
        analogWrite(en,100);

    } else {
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, HIGH);
        analogWrite(en,0);

    }
    // Note: motors will run continuously. Add logic to stop them after the gate reaches its limit (e.g., limit switches)
}

// Function to read ultrasonic distance
float readUltrasonic(NewPing* sonar) {
  delay(30); // Wait between pings
  unsigned int uS = sonar->ping();
  return (uS / US_ROUNDTRIP_CM); // Convert to centimeters
}

// Function to handle incoming MQTT messages (for motor commands from Node-RED)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.println(messageTemp);

  if (String(topic) == TOPIC_SUBSCRIBE_M1_CMD) {
    if (messageTemp == "OPEN") {
      controlMotor(M1_IN1, M1_IN2, true, enA);
    } else if (messageTemp == "CLOSE") {
      controlMotor(M1_IN1, M1_IN2, false, enA);
    }
  } else if (String(topic) == TOPIC_SUBSCRIBE_M2_CMD) {
    if (messageTemp == "OPEN") {
      controlMotor(M2_IN1, M2_IN2, true, enB);
    } else if (messageTemp == "CLOSE") {
      controlMotor(M2_IN1, M2_IN2, false, enB);
    }
  }
}

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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      // Subscribe to motor command topics
      client.subscribe(TOPIC_SUBSCRIBE_M1_CMD);
      client.subscribe(TOPIC_SUBSCRIBE_M2_CMD);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}