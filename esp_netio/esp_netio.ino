#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

/*
  in PubSubClient.h set
  #define MQTT_MAX_PACKET_SIZE xxx
  to
  #define MQTT_MAX_PACKET_SIZE 1024

  Otherwise you never receive some messages
*/
#define RX    5   // *** D1, Pin 2
#define TX    4   // *** D2, Pin 1 I hope

const int ledPin =  14;// the number of the LED pin
const int ledPin2 =  12;


SoftwareSerial Serial2(RX, TX);

const char* ssid = "41";
const char* password = "41414141";
const char* mqtt_server = "mqtt.cgcs.cz";
const size_t capacity = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(2) + 4 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(10) + 350; //from https://arduinojson.org/v6/assistant/
int plugs[4] = {0, 0, 0, 0};

const String netioId = "netio-test";
int zasuvka = 0;
int state = plugs[zasuvka];
const String inTopic = "devices/" + netioId + "/messages/events/";
const String outTopic = "devices/" + netioId + "/messages/devicebound/";
WiFiClient espClient;
PubSubClient client(espClient);
int changeLED = 0;
int ledMode = 0;
unsigned long int modeTimerStart = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void updateLed() {
  int state = plugs[zasuvka];
  digitalWrite(ledPin, state);
  digitalWrite(ledPin2, !state);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived");
  //Serial.print(topic);
  //Serial.print("] ");
  //for (int i = 0; i < length; i++) {
  //  Serial.print((char)payload[i]);
  //}
  Serial.println();
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload, length);
  JsonArray Outputs = doc["Outputs"];
  Serial.print("outputs length: ");
  int len = Outputs.size();
  Serial.println(len);
  for (int i = 0; i < len; i++) {
    JsonObject output = Outputs[i];
    int OutputID = output["ID"];
    int OutputState = output["State"];
    plugs[OutputID - 1] = OutputState;
    Serial.print("Output ID:");
    Serial.print(OutputID);
    Serial.print("; State:");
    Serial.println(OutputState);
  }

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish((char*)outTopic.c_str(), "{\"Operation\":\"GetDescription\"}");
      // ... and resubscribe
      client.subscribe((char*)inTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool led;
int targetState = 0;
// constants won't change. Used here to set a pin number:

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
int payload = 0;
// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

void setup() {
  // set the digital pin as output:
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  Serial.begin(9600);
  Serial2.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void sendData(String id, String num) {
  String json = "text" + num + "othertext";
  String topic = "device/" + id + "/messages/devicebound/";
  //mqttClient.send(topic, json);
}

void loop() {
  // here is where you'd put code that needs to be running all the time.

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  state = plugs[zasuvka];
  if (Serial2.available()) {
    int input = Serial2.read() - '0';
    Serial.println(input);
    if (input == 0) {
      if (ledMode == 1) {
        changeLED = 1;
      }
      ledMode = 1;
      modeTimerStart = millis();
    } else {
      targetState = !state;
      String payload = "{\"Operation\":\"SetOutputs\",\"Outputs\":[{\"ID\":";
      payload += String(zasuvka + 1);
      payload += ",\"Action\":";
      payload += targetState;
      payload += "}]}";
      Serial.print("Publish message: ");
      Serial.print(outTopic);
      Serial.print("; ");
      Serial.println(payload);
      client.publish((char*)outTopic.c_str(), (char*) payload.c_str());
    }
  }
  if (ledMode == 0) { //default mode, leds show state of selected outlet

    digitalWrite(ledPin, state);
    digitalWrite(ledPin2, !state);
  }
  else if (ledMode == 1) {
    if (millis() - modeTimerStart > 1500) { //only show the plug change "menu" for 1,5 seconds
      ledMode = 0;
      changeLED = 0;
      digitalWrite(ledPin, LOW);
      digitalWrite(ledPin2, LOW);
    }
    if (changeLED == 1) {
      zasuvka++;
      changeLED = 0;
      if (zasuvka == 4) {
        zasuvka = 0;
      }
      Serial.print("changed zasuvka to ");
      Serial.println(zasuvka);
    }
    digitalWrite(ledPin, bitRead(zasuvka, 1));
    digitalWrite(ledPin2, bitRead(zasuvka, 0));
  }

  //digitalWrite(ledPin, !ledState);
  //digitalWrite(ledPin2, ledState);
}
