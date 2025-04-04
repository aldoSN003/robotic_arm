#include <WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT Configuration
const char* ssid = "ETNetwork";
const char* password = "etmanager";
const char* mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_username = "emqx";
const char* mqtt_password = "public";

// MQTT Topics
const char* topicControl = "motor/control";
const char* topicSelect = "motor/select";

// Motor Control Pins (Example for 4 motors)
#define MOTOR1_IN1 19
#define MOTOR1_IN2 21
#define MOTOR1_ENA 22

#define MOTOR2_IN1 16
#define MOTOR2_IN2 5
#define MOTOR2_ENB 4

#define MOTOR3_IN1 15
#define MOTOR3_IN2 14
#define MOTOR3_ENA 13

#define MOTOR4_IN1 12
#define MOTOR4_IN2 27
#define MOTOR4_ENB 26

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long startTime = 0; // To track button press duration
int selectedMotor = 1; // Default selected motor

void setup() {
    Serial.begin(115200);

    // Initialize motor pins
    pinMode(MOTOR1_IN1, OUTPUT);
    pinMode(MOTOR1_IN2, OUTPUT);
    pinMode(MOTOR1_ENA, OUTPUT);

    pinMode(MOTOR2_IN1, OUTPUT);
    pinMode(MOTOR2_IN2, OUTPUT);
    pinMode(MOTOR2_ENB, OUTPUT);

    pinMode(MOTOR3_IN1, OUTPUT);
    pinMode(MOTOR3_IN2, OUTPUT);
    pinMode(MOTOR3_ENA, OUTPUT);

    pinMode(MOTOR4_IN1, OUTPUT);
    pinMode(MOTOR4_IN2, OUTPUT);
    pinMode(MOTOR4_ENB, OUTPUT);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Connect to MQTT Broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-" + String(WiFi.macAddress());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT Broker!");
        } else {
            Serial.print("MQTT connection failed, retrying in 5s. State: ");
            Serial.println(client.state());
            delay(5000);
        }
    }
    client.subscribe(topicControl);
    client.subscribe(topicSelect);
}

void loop() {
    client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == topicSelect) {
        selectedMotor = message.toInt(); // Update selected motor
        Serial.print("Selected Motor: ");
        Serial.println(selectedMotor);
    } else if (String(topic) == topicControl) {
        int motorNumber = message.substring(0, message.indexOf(':')).toInt();
        String command = message.substring(message.indexOf(':') + 1);

        if (motorNumber == selectedMotor) {
            if (command == "FORWARD") {
                startMotor(motorNumber, true);
                startTime = millis(); // Record start time
                Serial.print("Motor ");
                Serial.print(motorNumber);
                Serial.println(" started: FORWARD");
            } else if (command == "BACKWARD") {
                startMotor(motorNumber, false);
                startTime = millis(); // Record start time
                Serial.print("Motor ");
                Serial.print(motorNumber);
                Serial.println(" started: BACKWARD");
            } else if (command == "STOP") {
                stopMotor(motorNumber);
                unsigned long duration = (millis() - startTime) / 1000; // Calculate duration in seconds
                Serial.print("Motor ");
                Serial.print(motorNumber);
                Serial.print(" stopped. Duration: ");
                Serial.print(duration);
                Serial.println(" seconds");
            }
        }
    }
}

// Function to start a motor
void startMotor(int motorNumber, bool isForward) {
    int in1, in2, en;
    switch (motorNumber) {
        case 1: in1 = MOTOR1_IN1; in2 = MOTOR1_IN2; en = MOTOR1_ENA; break;
        case 2: in1 = MOTOR2_IN1; in2 = MOTOR2_IN2; en = MOTOR2_ENB; break;
        case 3: in1 = MOTOR3_IN1; in2 = MOTOR3_IN2; en = MOTOR3_ENA; break;
        case 4: in1 = MOTOR4_IN1; in2 = MOTOR4_IN2; en = MOTOR4_ENB; break;
        default: return;
    }
    digitalWrite(in1, isForward ? HIGH : LOW);
    digitalWrite(in2, isForward ? LOW : HIGH);
    analogWrite(en, 255); // Enable the motor at full speed
}

// Function to stop a motor
void stopMotor(int motorNumber) {
    int in1, in2, en;
    switch (motorNumber) {
        case 1: in1 = MOTOR1_IN1; in2 = MOTOR1_IN2; en = MOTOR1_ENA; break;
        case 2: in1 = MOTOR2_IN1; in2 = MOTOR2_IN2; en = MOTOR2_ENB; break;
        case 3: in1 = MOTOR3_IN1; in2 = MOTOR3_IN2; en = MOTOR3_ENA; break;
        case 4: in1 = MOTOR4_IN1; in2 = MOTOR4_IN2; en = MOTOR4_ENB; break;
        default: return;
    }
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(en, 0); // Disable the motor
}