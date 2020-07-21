#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define WIFI_SSID "PRINA"
#define WIFI_PASSWORD "TQ4TRTJKQC"
#define NODE_NAME "f6cb2100-8fc8-11ea-a128-4bbb219d4a25"
#define NODE_TOKEN "6bLfQvt6NtySs99pR8Oq"
#define TRIG_PIN D4
#define ECHO_PIN D3
#define MOTOR_PIN D2

bool test = false;
bool actualizarContGelAMaxSiLLegaACero = false;
bool first = true;

const unsigned int distanciaActivacion = 8; //cm
const long tiempoMedicion = 5000; //milisegundos
const int tiempoDispension = 500; //milisegundos
const int maxGel = 10;

char thingsboardServer[] = "demo.thingsboard.io";
char telemetryTopic[] = "v1/devices/me/telemetry";
char requestTopic[] = "v1/devices/me/rpc/request/+";
char attributesTopic[] = "v1/devices/me/attributes";

bool procesandoMsj = false;
int contGel;

unsigned long tiempoInicial;
unsigned long tiempoActual;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {

  Serial.begin(9600);
  EEPROM.begin(512);

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(MOTOR_PIN, LOW);

  conectarWIFI();
  client.setServer(thingsboardServer, 1883);
  client.setCallback(on_message);

  delay(10);
  tiempoActual = tiempoInicial = millis();
}

void loop() {

  if (!test) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }

  if (first) {
    setConGel();
    first = false;
  }

  if (Serial.available()) {
    char c = Serial.read();
    Serial.print("Comando recibido: ");
    Serial.println(c);
    switch (c) {
      case '0':
        while (procesandoMsj) {}
        setConGel();
      break;
      case '1': test = true; break;
      case '2': test = false; break;
      case '3': actualizarContGelAMaxSiLLegaACero = true; break;
      case '4': actualizarContGelAMaxSiLLegaACero = false; break;
      default: break;
    }
  }
  
  if (procesandoMsj) {
    //Comenzar a detectar manos
    tiempoActual = millis();
    comenzarMedicion();
  } else {
    tiempoInicial = millis();
    if (test) {
      procesandoMsj = true;
    }
  }

  //Serial.println("-");
  delay(300);
}

void enviarInformacion(String name, String msg) {
  String payload = "{";
  payload += "\"";
  payload += name;
  payload += "\":";
  payload += msg;
  payload += "}";
  
  char attributes[200];
  payload.toCharArray(attributes, 200);
  if (client.publish(telemetryTopic, attributes) == true) {
    Serial.print("Enviado -> ");
  } else {
    Serial.print("Error -> ");
  }
  Serial.println(attributes);
}

void on_message(const char* topic, byte* payload, unsigned int length) {
  Serial.println(procesandoMsj);
  if (!procesandoMsj) {
    procesandoMsj = true;
    char json[length + 100];
    strncpy (json, (char*)payload, length);
    json[length] = '\0';

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parseObject((char*)json);

    if (!data.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    String methodName = String((const char*)data["method"]);
    Serial.print("Nombre metodo: ");
    Serial.println(methodName);
    
    if (methodName.equals("personaDetectada")) {
      if (data["params"]) {
        Serial.println("Midiendo distancia ...");
      }
    } else if (methodName.equals("maxGel")) {
      contGel = maxGel;
      Serial.println("Volumen de gel actualizado ...");
      procesandoMsj = false;
    } else {
      procesandoMsj = false;
    }
  }
}

void comenzarMedicion() {
  if (tiempoActual - tiempoInicial <= tiempoMedicion) {
    if (distanciaMenorA(distanciaActivacion)) {
      dispensarAlcohol();
      if (!test) {
        String s = String(contGel);
        enviarInformacion("prenderLed", s);
      }
      actualizarContGelSiEsCero();
      procesandoMsj = false;
    }
  } else {
    procesandoMsj = false;
    Serial.println("Manos no detectadas");
  }
}

void dispensarAlcohol() {
  if (contGel > 0) {
    digitalWrite(MOTOR_PIN, HIGH);
    delay(tiempoDispension);
    digitalWrite(MOTOR_PIN, LOW);
    contGel--;
    EEPROM.write(0, contGel);
    EEPROM.commit();
    Serial.print("Gel dispensado. Descargas restantes: ");
    Serial.print(contGel);
    Serial.println(".");
  } else {
    Serial.println("No hay mas gel.");
  }
}

void actualizarContGelSiEsCero() {
  if (actualizarContGelAMaxSiLLegaACero) {
    if (contGel == 0) {
      contGel = maxGel;
    }
  }
}

bool distanciaMenorA(int x) {
  const byte cantidadDeMedidas = 5;
  unsigned int sumaDistancias = 0;
  for (byte i = 0; i < cantidadDeMedidas; i++) {
    sumaDistancias += leerDistanciaSensor();
    delay(1);
  }
  unsigned int promedio = sumaDistancias / cantidadDeMedidas;
  
  //Serial.print("Promedio: "); Serial.println(promedio);
  return promedio < x;
}

unsigned int leerDistanciaSensor() {
  unsigned int maxDistancia = 500;
  unsigned long timeout = 14705; //500/0.034
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, timeout);
  unsigned int distance = duration * 0.034 / 2; //cm

  if (distance == 0) {
    distance = maxDistancia;
  }

  //Serial.print("Distance: "); Serial.println(distance);
  return distance;
}

void setConGel() {
  procesandoMsj = true;
  const long tMax = 7000; //milisegundos
  bool actualizarContenidoDeGel = false;
  tiempoInicial = tiempoActual = millis();
  Serial.println("Actualizar valor de gel al maximo?");
  Serial.print("Acerque su mano al sensor (");
  Serial.print(tMax / 1000);
  Serial.println("s.) ");
  while (!actualizarContenidoDeGel && (tiempoActual - tiempoInicial <= tMax)) {
    if (distanciaMenorA(distanciaActivacion)) {
      actualizarContenidoDeGel = true;
    } else {
      tiempoActual = millis();
    }
  }
  if (actualizarContenidoDeGel) {
    contGel = maxGel;
    EEPROM.write(0, contGel);
    EEPROM.commit();
    Serial.print("Valor actualizado a: ");
    Serial.print(maxGel);
    Serial.println(" descargas.");
    String s = String(contGel);
    enviarInformacion("prenderLed", s);
  } else {
    contGel = EEPROM.read(0);
    Serial.print("Nivel del gel: ");
    Serial.print(contGel);
    Serial.println(" descargas restantes.");
  }
  delay(3000);
  procesandoMsj = false;
}

void conectarWIFI() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Conectado: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  int statusWifi = WL_IDLE_STATUS;
  // Loop until we're reconnected
  while (!client.connected()) {
    statusWifi = WiFi.status();
    //conectarWIFI();

    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if (client.connect(NODE_NAME, NODE_TOKEN, NULL)) {
      Serial.println("[DONE]");

      // Suscribir al Topico de request
      client.subscribe(requestTopic);
      //client.subscribe(attributesTopic);

    } else {
      Serial.print("[FAILED] [ rc = ");
      Serial.print(client.state());
      Serial.println(" : retrying in 5 seconds]");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

