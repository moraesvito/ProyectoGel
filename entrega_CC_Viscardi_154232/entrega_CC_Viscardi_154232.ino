#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>


#define WIFI_SSID "Robawifi"
#define WIFI_PASSWORD "rata123."
// lo mio #define NODE_NAME "b112ef00-b18b-11ea-b4ad-47e5929eed78"
// lo mio #define NODE_TOKEN "zDADY3SphjHFJKc8FUMf"
#define NODE_NAME "6c85cea0-bbd8-11ea-b4ad-47e5929eed78"
#define NODE_TOKEN "6ujwDHW4gG0V2bRg2um3"

//Nombro de forma amistosa los leds
int verde = D5; //VERDE
int amarillo = D6; //AMARILLO
int rojo = D7; //ROJO

bool aa = false; // bandera para controlar el loop

char thingsboardServer[] = "demo.thingsboard.io";
char telemetryTopic[] = "v1/devices/me/telemetry";
char requestTopic[] = "v1/devices/me/rpc/request/+";
char attributesTopic[] = "v1/devices/me/attributes";


WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  
  Serial.begin(9600);

  conectarWIFI();
  client.setServer(thingsboardServer, 1883);
  client.setCallback(on_message);
  
  delay(10);

inicioLeds();
apagarTodos(); 
}


void loop() {
  if ( !client.connected() ) {
    reconnect();
  }
  delay(3000);
  //Serial.println("-");
  // getAndSendData();
  client.loop(); 
}

void getAndSendData() {
  
  Serial.println("Collecting data.");

  String prender = "prender";
  String apagar = "apagar";

  //Debug messages
    
  String payload = "{";
  payload += "\"alcoholDispensado\":"; payload += "true";
  payload += "}";

  // Enviar el payload al servidor usando el topico para telemetria
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  if (client.publish( telemetryTopic, attributes ) == true)
    Serial.println("publicado ok");
  
  Serial.println( attributes );
}

 

void on_message(const char* topic, byte* payload, unsigned int length) {
  if (!aa){
    aa = true;
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
  
      if (methodName.equals("prenderLed")) 
          apagarTodos ();
          String a = data["params"];
          if (a == "rojo"){
            getAndSendData();
            digitalWrite(rojo, HIGH);
            Serial.println("prende rojo"); 
          }
           if (a == "amarillo"){
            getAndSendData();
            digitalWrite(amarillo, HIGH);
            Serial.println("prende amarillo");
          }
           if (a == "verde"){
            getAndSendData();
            digitalWrite(verde, HIGH);
            Serial.println("prende verde");
          }
                   Serial.print(a);
          aa = false;
  }
}
void apagarTodos (){
  digitalWrite(amarillo, LOW);
  digitalWrite(rojo, LOW);
   digitalWrite(verde, LOW);
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
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void inicioLeds(){
  // initialize the digital pin as an output.
  pinMode(verde, OUTPUT);
  pinMode(amarillo, OUTPUT);
  pinMode(rojo, OUTPUT); 
  apagarTodos();
}

 
