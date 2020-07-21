#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <cp437font.h>
#include <LedMatrix.h>
#define NUMBER_OF_DEVICES 1
#define CS_PIN 2

#define __DEBUG__
 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANCHO_PANTALLA 128 // ancho pantalla OLED
#define ALTO_PANTALLA 64 // alto pantalla OLED

#define WIFI_SSID "Redmi"
#define WIFI_PASSWORD "2pac2furius"
#define NODE_NAME "7f1a1fd0-bbd8-11ea-b4ad-47e5929eed78"
#define NODE_TOKEN "Ywin4nzSkai2KwIpPfj4"


char thingsboardServer[] = "demo.thingsboard.io";
char telemetryTopic[] = "v1/devices/me/telemetry";
char requestTopic[] = "v1/devices/me/rpc/request/+";
char attributesTopic[] = "v1/devices/me/attributes";


Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);


WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  ////DEBUG DEL OLED
  #ifdef __DEBUG__
  Serial.begin(9600);
  delay(100);
  Serial.println("Iniciando pantalla OLED");
  #endif
  conectarWIFI();
  client.setServer(thingsboardServer, 1883);
  client.setCallback(on_message);
  
  delay(10);

  ledMatrix.init();///Iniciamos matriz de led
  
  
  
  // Iniciar pantalla OLED en la dirección 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  #ifdef __DEBUG__
  Serial.println("No se encuentra la pantalla OLED");
  #endif

}
}

void loop() {

  
  ledMatrix.clear();
  ledMatrix.scrollTextLeft();
  ledMatrix.drawText();
  ledMatrix.commit();
  ledMatrix.setNextText("<-<-"); 
  
  
  if ( !client.connected() ) {
    reconnect();
  }
  delay(50);
  Serial.println("-");
  client.loop();

}

 

void getAndSendData() {
  
  Serial.println("Collecting data.");

  
  String msg = "true";
  String name = "alcoholDispensado";

  ///Debug
  Serial.println("Enviando "+ msg); 
  
  String payload = "{";
  payload += "\"";
  payload += name;
  payload += "\":";
  payload += msg;
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
  

  if (methodName.equals("alcoholDispensado")){

    if(data["params"]){
      Serial.println("Gracias, que pase un buen día.");
      imprimirGracias();
      getAndSendData(); 
      delay(5000);
      imprimirAlerta();
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

void imprimirAlerta(){
  // Limpir buffer
  display.clearDisplay();
 
  // Tamaño del texto
  display.setTextSize(2);
  // Color del texto
  display.setTextColor(SSD1306_WHITE);
  // Posición del texto
  display.setCursor(0, 18);
  // Escribir texto
  display.println("Alerta");
  display.setCursor(14, 30);
  display.println("Covid-19");
  
  
  // Enviar a pantalla
  display.display();





}

void imprimirGracias(){

  // Limpir buffer
  display.clearDisplay();
 
  // Tamaño del texto
  display.setTextSize(2);
  // Color del texto
  display.setTextColor(SSD1306_WHITE);
  // Posición del texto
  display.setCursor(0, 18);
  // Escribir texto
  display.println("Gracias");
  display.setCursor(14, 30);
  display.println("Buen dia");
  
  // Enviar a pantalla
  display.display();
  delay(5000);
 


}

 
