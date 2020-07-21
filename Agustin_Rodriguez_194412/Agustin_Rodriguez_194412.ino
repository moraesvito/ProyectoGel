#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID "Whisky"
#define WIFI_PASSWORD "ponecualquiercosa"
#define THINGSBOARD_DEVICE_NAME "86690120-bbd8-11ea-b4ad-47e5929eed78"
#define THINGSBOARD_DEVICE_TOKEN "rCWooAf2uiTT8DVBT8Jb"

#define PIN_PIR 6

char thingsBoardServer[] = "demo.thingsboard.io";
char telemetryTopic[] = "v1/devices/me/telemetry";
char requestTopic[] = "v1/devices/me/rpc/request/+";
char attributesTopic[] = "v1/devices/me/attributes";

WiFiClient wifiClient;
PubSubClient thingsBoardClient(wifiClient);

bool personaDetectada = true;

void setup() {
  Serial.begin(9600);

  setupWiFi();
  setupThingsboard();

  delay(5000);
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("[OK] Connected: ");
  Serial.println(WiFi.localIP());
}

void setupThingsboard() {
  thingsBoardClient.setServer(thingsBoardServer, 1883);
  // thingsBoardClient.setCallback(on_message);
}

void loop() {
  connectThingsboard();
  detectMovement();

  thingsBoardClient.loop();
  
  delay(5000);
}

void connectThingsboard() {
  while (!thingsBoardClient.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    // Attempt to connect (clientId, username, password)
    if (thingsBoardClient.connect(THINGSBOARD_DEVICE_NAME, THINGSBOARD_DEVICE_TOKEN, NULL)) {
      Serial.println("[OK] Connected.");

      thingsBoardClient.subscribe(requestTopic);
      
    } else {
      Serial.print("[FAILED] [ rc = ");
      Serial.print(thingsBoardClient.state());
      Serial.println(" : retrying in 5 seconds]");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void detectMovement() {
  int val;
  int ledPin = 2;
  bool motionState = false;
  // Read out the pirPin and store as val:
  val = digitalRead(PIN_PIR);
  // If motion is detected (pirPin = HIGH), do the following:
  if (val == HIGH) {
    digitalWrite(ledPin, HIGH); // Turn on the on-board LED.
    // Change the motion state to true (motion detected):
    if (motionState == false) {
      Serial.println("Motion detected!");
      motionState = true;
      sendData("true");
    }
  }
  // If no motion is detected (pirPin = LOW), do the following:
  else {
    digitalWrite(ledPin, LOW); // Turn off the on-board LED.
    // Change the motion state to false (no motion):
    if (motionState == true) {
      Serial.println("Motion ended!");
      motionState = false;
      sendData("false");
    }
  }
}

void sendData(String attr) {

  // personaDetectada = !personaDetectada;
  // String attr = "true";

  // if (!personaDetectada) {
  //  attr = "false";
  // }
  
  String payload = "{";
  payload += "\"personaDetectada\":"; payload += attr;
  payload += "}";

  // Enviar el payload al servidor usando el topico para telemetria
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  if (thingsBoardClient.publish( telemetryTopic, attributes ) == true)
    Serial.println("publicado ok");
  
  Serial.println( attributes );
}