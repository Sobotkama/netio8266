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
#define RX    5
#define TX    4

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

  randomSeed(micros()); //lol

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived:");
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload, length);
  JsonArray Outputs = doc["Outputs"];
  int len = Outputs.size();
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
      // Once connected, request netio state...
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

int targetState = 0;

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

void loop() {
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
}
