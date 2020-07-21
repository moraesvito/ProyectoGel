#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "Moraes"
#define WIFI_PASSWORD "agusvito"
#define NODE_NAME "77cf0f10-bbd8-11ea-b4ad-47e5929eed78"
#define NODE_TOKEN "XmDXe1XjbtDKomZ2Bga5"

char thingsboardServer[] = "demo.thingsboard.io";
char telemetryTopic[] = "v1/devices/me/telemetry";
char requestTopic[] = "v1/devices/me/rpc/request/+";
char attributesTopic[] = "v1/devices/me/attributes";

LiquidCrystal_I2C lcd(0x3F, 16, 2);
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  
  Serial.begin(9600);

  conectarWIFI();
  client.setServer(thingsboardServer, 1883);
  client.setCallback(on_message);
  
  delay(10);

  lcd.begin(16,2);
  lcd.init();
  imprimirBienvenido();
}


void loop() {
  
  if ( !client.connected() ) {
    reconnect();
  }
  delay(1000);
  Serial.println("-");
  client.loop();
}

 

void getAndSendData() {
  
  Serial.println("Collecting data.");

  String msj = "personaDetectada";

  //Debug messages
  Serial.println( "Sending: " + msj);
  
  String payload = "{";
  payload += "\"";
  payload += msj;
  payload += "\":";
  payload += "true";
  payload += "}";
  

  // Enviar el payload al servidor usando el topico para telemetria
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  if (client.publish( telemetryTopic, attributes ) == true)
    Serial.println("publicado ok");
  
  Serial.println( attributes );
}

 

void on_message(const char* topic, byte* payload, unsigned int length) {
  
  Serial.println("On message");

  char json[length + 1];
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject((char*)json);

  if (!data.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  String methodName = String((const char*)data["method"]);
  Serial.print("Nombre metodo:");
  Serial.println(methodName);
  
  if (methodName.equals("personaDetectada")) {
    if (data["params"]) {
        Serial.println("Acerque su mano al display.");
        imprimirAcercarse();
        getAndSendData();
    }
  }

  if (methodName.equals("alcoholDispensado")) {
    if (data["params"]) {
        Serial.println("Bienvenido.");
        imprimirBienvenido();
    }
  }
 
}

void reconnect() {
  int statusWifi = WL_IDLE_STATUS;
  // Loop until we're reconnected
  while (!client.connected()) {
    statusWifi = WiFi.status();
    //conectarWIFI();
    
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect(NODE_NAME, NODE_TOKEN, NULL) ) {
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

void imprimirAcercarse(){
  lcd.clear();
  // Turn on the backlight.
  lcd.backlight();

  // Move cursor
  lcd.setCursor(0, 0);
  lcd.print("ACERQUE SU MANO");

  // Move the cursor to the next line
  lcd.setCursor(0, 1);      
  lcd.print("AL DISPENSADOR");
}

void imprimirBienvenido(){
  lcd.clear();
  // Turn on the backlight.
  lcd.backlight();

  // Move cursor
  lcd.setCursor(3, 0);
  lcd.print("BIENVENIDO");

}

 
