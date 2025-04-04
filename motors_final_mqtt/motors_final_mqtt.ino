#include <WiFi.h>
#include <PubSubClient.h>

// Configuración de WiFi y MQTT
const char* ssid = "ETNetwork";               // Nombre de la red WiFi
const char* password = "etmanager";           // Contraseña de la red WiFi
const char* mqtt_broker = "broker.emqx.io";   // Dirección del broker MQTT
const int mqtt_port = 1883;                   // Puerto MQTT
const char* mqtt_username = "emqx";           // Usuario MQTT
const char* mqtt_password = "public";         // Contraseña MQTT

// Tópicos MQTT
const char* topicControl = "motor/control";   // Tópico para controlar los motores
const char* topicSelect = "motor/select";     // Tópico para seleccionar el motor

// Pines de control de los motores (ejemplo para 4 motores)
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

unsigned long startTime = 0; // Para registrar el tiempo de inicio del motor
int selectedMotor = 1;       // Motor seleccionado por defecto

void setup() {
    Serial.begin(115200); // Inicializa la comunicación serial

    // Inicializa los pines como salida
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

    // Conexión a la red WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Conectando a WiFi...");
    }
    Serial.println("¡Conectado a WiFi!");

    // Conexión al broker MQTT
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback); // Establece la función de callback para mensajes recibidos

    while (!client.connected()) {
        String client_id = "esp32-client-" + String(WiFi.macAddress()); // ID único del cliente
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("¡Conectado al broker MQTT!");
        } else {
            Serial.print("Fallo al conectar con MQTT, reintentando en 5s. Estado: ");
            Serial.println(client.state());
            delay(5000);
        }
    }

    // Suscripción a los tópicos
    client.subscribe(topicControl);
    client.subscribe(topicSelect);
}

void loop() {
    client.loop(); // Mantiene activa la conexión con el broker MQTT
}

// Función callback que se ejecuta al recibir un mensaje MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    // Si el mensaje es para seleccionar el motor
    if (String(topic) == topicSelect) {
        selectedMotor = message.toInt(); // Actualiza el motor seleccionado
        Serial.print("Motor seleccionado: ");
        Serial.println(selectedMotor);

    // Si el mensaje es para controlar el motor
    } else if (String(topic) == topicControl) {
        int motorNumber = message.substring(0, message.indexOf(':')).toInt(); // Obtiene el número de motor
        String command = message.substring(message.indexOf(':') + 1);         // Obtiene el comando (FORWARD, BACKWARD, STOP)

        // Solo se ejecuta si el motor es el seleccionado actualmente
        if (motorNumber == selectedMotor) {
            if (command == "FORWARD") {
                startMotor(motorNumber, true); // Inicia el motor hacia adelante
                startTime = millis();         // Registra el tiempo de inicio
                Serial.print("Motor ");
                Serial.print(motorNumber);
                Serial.println(" iniciado: FORWARD");

            } else if (command == "BACKWARD") {
                startMotor(motorNumber, false); // Inicia el motor hacia atrás
                startTime = millis();           // Registra el tiempo de inicio
                Serial.print("Motor ");
                Serial.print(motorNumber);
                Serial.println(" iniciado: BACKWARD");

            } else if (command == "STOP") {
                stopMotor(motorNumber);                         // Detiene el motor
                unsigned long duration = (millis() - startTime) / 1000; // Calcula duración en segundos
                Serial.print("Motor ");
                Serial.print(motorNumber);
                Serial.print(" detenido. Duración: ");
                Serial.print(duration);
                Serial.println(" segundos");
            }
        }
    }
}

// Función para iniciar un motor en una dirección específica
void startMotor(int motorNumber, bool isForward) {
    int in1, in2, en;

    // Asigna los pines correspondientes al motor
    switch (motorNumber) {
        case 1: in1 = MOTOR1_IN1; in2 = MOTOR1_IN2; en = MOTOR1_ENA; break;
        case 2: in1 = MOTOR2_IN1; in2 = MOTOR2_IN2; en = MOTOR2_ENB; break;
        case 3: in1 = MOTOR3_IN1; in2 = MOTOR3_IN2; en = MOTOR3_ENA; break;
        case 4: in1 = MOTOR4_IN1; in2 = MOTOR4_IN2; en = MOTOR4_ENB; break;
        default: return; // Motor inválido
    }

    // Establece la dirección del motor
    digitalWrite(in1, isForward ? HIGH : LOW);
    digitalWrite(in2, isForward ? LOW : HIGH);
    analogWrite(en, 255); // Activa el motor a velocidad máxima
}

// Función para detener un motor
void stopMotor(int motorNumber) {
    int in1, in2, en;

    // Asigna los pines correspondientes al motor
    switch (motorNumber) {
        case 1: in1 = MOTOR1_IN1; in2 = MOTOR1_IN2; en = MOTOR1_ENA; break;
        case 2: in1 = MOTOR2_IN1; in2 = MOTOR2_IN2; en = MOTOR2_ENB; break;
        case 3: in1 = MOTOR3_IN1; in2 = MOTOR3_IN2; en = MOTOR3_ENA; break;
        case 4: in1 = MOTOR4_IN1; in2 = MOTOR4_IN2; en = MOTOR4_ENB; break;
        default: return; // Motor inválido
    }

    // Apaga el motor
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(en, 0); // Desactiva el motor
}
