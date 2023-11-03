#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Servo.h> 
#include <Wire.h>

#define pinServoDirecc 13 //D7
#define pinServoCam 15 //D8

enum acciones {AVANZAR, RETROCEDER, REINICIO, IZQUIERDA, DERECHA, LENTO, MEDIO, RAPIDO};

int velMotIzq = 14, // 
    motIzqAvz = 0,  //  
    motIzqRet = 2,  // 

    motDerRet = 5,  // 
    velMotDer = 4, // 
    motDerAvz = 16,  //

    velocidad;  //

// Variables de configuración 
const char* mqttBroker = "192.168.68.229";
const char* ssid = "laboratorioFinal";
const char* password = "12345678";
WiFiClient espClient;
PubSubClient client(espClient);

long duracion;
Servo direccion;
Servo servoCam;

// Declaración de topics
const char* topicVel = "car/vel";
const char* topicDir = "car/dir";
const char* topicMov = "car/mov";
const char* topicCam = "car/cam";
const char* topicAutomatico = "automatico";

int indiceMovimientos = 0;
const int maxMovimientos = 20;
String accionesAutomaticas[2][maxMovimientos];

void setup() {
  Serial.begin(115200);
  direccion.attach(pinServoDirecc);
  direccion.attach(pinServoCam );
  pinMode(motIzqAvz, OUTPUT);
  pinMode(motIzqRet, OUTPUT);
  pinMode(motDerAvz, OUTPUT);
  pinMode(motDerRet, OUTPUT);
  pinMode(velMotIzq, OUTPUT);
  pinMode(velMotDer, OUTPUT);

  digitalWrite(motIzqAvz, LOW);
  digitalWrite(motDerAvz, LOW);
  digitalWrite(motIzqRet, LOW);
  digitalWrite(motDerRet, LOW);


  setup_wifi();

  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

/* Función que gestiona los mensajes recibidos por MQTT. 
  Cuando el ESP recibe un mensaje se registra el contenido del mensaje en 
  payload, el largo de caracteres en la variable length y la dirección del
  topic a donde fue enviado.
  Ej: mosquitto_pub -h 192.168.1.1 -t car/vel -m 3 
      Si el ESP está suscripto al topic car/vel, cuándo llegue el mensaje 
      del ejemplo las variables toman estos valores 
      topic = car/vel
      payload = 3
      lenght = 2 ( toma el caracter 3 y el fin de línea '\n') */
void callback(char* topic, byte* payload, unsigned int length) {
  char buffer[10];
  String msg;

  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }  

  // Acciones para topic = car/vel
  if(String(topicVel).equals(topic)){
    if(!msg.compareTo("1")){
      setVelocidad(LENTO);
    }
    else if (!msg.compareTo("2")){
      setVelocidad(MEDIO);
    }
    if(!msg.compareTo("3")){
      setVelocidad(RAPIDO);
    }
  }

  // Acciones para topic = car/dir
  if(String(topicDir).equals(topic)){
    if(!msg.compareTo("izq")){
      setDireccion(IZQUIERDA);
    }
    else if(!msg.compareTo("der")){
      setDireccion(DERECHA);
    }
  }

  // Acciones para topic = car/mov
  if(String(topicMov).equals(topic)){
    if(!msg.compareTo("avanzar")){
      avanzar();
    }
    else if(!msg.compareTo("retroceder")){
      retroceder();
    }
    else if(!msg.compareTo("reiniciar")){
      reiniciar();
    }
  }
  
  // Acciones para topic  = automatico
  if(String(topicAutomatico).equals(topic)){
    if (!msg.compareTo("graba")){
      for(int i = 0; i = maxMovimientos; i++){
        accionesAutomaticas[0][i] = "";
        accionesAutomaticas[1][i] = "";
      }      
      indiceMovimientos = 0;
    }
    else if (!msg.compareTo("activo")){
      // Ejecuta las acciones cargadas previamente
      automatico();
      Serial.println("Hola mundo");
    }
  }

  accionesAutomaticas[0][indiceMovimientos] = topic;
  accionesAutomaticas[1][indiceMovimientos] = msg;
  indiceMovimientos++;
}
void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Conectando a Wifi..");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  randomSeed(micros());
  
  Serial.println("WiFi conectado");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}
// Configuración de MQTT
void reconnect() {
  while (!client.connected()) {
    String clientId = "car";

    if (client.connect(clientId.c_str())) {
      Serial.print("MQTT conectado a broker: ");
      Serial.println(mqttBroker);

      // Lista de topics que se suscribe el vehiculo para actuar.
      client.subscribe(topicVel);
      client.subscribe(topicDir);
      client.subscribe(topicMov);
      client.subscribe(topicCam);
      client.subscribe(topicAutomatico);
    }
  }
}
void automatico(){
  for(int i =0; i<indiceMovimientos; i++){
    Serial.print("Topic: ");
    Serial.print(accionesAutomaticas[0][i]);
    Serial.print("   Mensaje: ");
    Serial.println(accionesAutomaticas[1][i]);
    
    client.publish(accionesAutomaticas[0][i].c_str(), accionesAutomaticas[1][i].c_str());
    delay(100);
  }
}
void setVelocidad(int unaVelocidad){
  switch(unaVelocidad){ 
    case LENTO:
      velocidad = 85; // Equivalente a 33 %
      Serial.println("LENTO.");
    break;
    case MEDIO:
      velocidad = 170; // Equivalente a 66 %
      Serial.println("MEDIO.");
    break;
    case RAPIDO:
      velocidad = 255; // Equivale a 100%
      Serial.println("RAPIDO.");
    break;
  }
  analogWrite(velMotIzq, velocidad);
  analogWrite(velMotDer, velocidad);
}
void setDireccion(int direcc){
  switch (direcc){
    case IZQUIERDA:
      analogWrite(velMotIzq, 0.66*velocidad);
      analogWrite(velMotDer, velocidad);
      Serial.println("IZQUIERDA.");
      direccion.write(0);
    break;
    case DERECHA:
      analogWrite(velMotIzq, velocidad);
      analogWrite(velMotDer, 0.66*velocidad);
      Serial.println("DERECHA.");
      direccion.write(180);
    break;
  }
}
void retroceder(){
  Serial.println("RETROCEDER.");
  digitalWrite(motIzqAvz, HIGH);
  digitalWrite(motDerAvz, HIGH);
  delay(1000);
  digitalWrite(motIzqAvz, LOW);
  digitalWrite(motDerAvz, LOW);
}
void avanzar(){
  Serial.println("AVANZAR.");
  digitalWrite(motIzqRet, HIGH);
  digitalWrite(motDerRet, HIGH);
  delay(1000);
  digitalWrite(motIzqRet, LOW);
  digitalWrite(motDerRet, LOW);
}
void reiniciar(){
  Serial.println("REINICIAR.");
  analogWrite(velMotIzq, velocidad);
  analogWrite(velMotDer, velocidad);
  direccion.write(90);
}




























