//PLACA DE MIGUE--> ESP_3097139 
//Placa de pablo --> ESP_3097253 (movimiento)
//placa de telmo ---> ESP_9590678 (zumbador)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <ArduinoJson.h>
DHTesp dht;

ADC_MODE(ADC_VCC);

WiFiClient wClient;
PubSubClient mqtt_client(wClient);

// Introducimos los valores de nuestra red WiFi
const char* ssid ="Redmi Note 9";//"DIGIFIBRA-f2CX"; //"iPhone";
const char* password ="lol99999"; //"ekhXP6XD4S"; // "aaaaaaaa";
const char* mqtt_server = "iot.ac.uma.es";

const char* mqtt_user = "II3";
const char* mqtt_pass = "79qUYsdM";

// La placa que vamos a utilizar en el proyecto es ??
// cadenas para topics e ID correspondiente con la placa utilizada

char ID_PLACA[16];
char topic_PUB[256];
char topic_SUB[256];

// GPIOs
int LED1 = 2;  
int LED2 = 16; 
byte zumbador=5; //GPIO del sensor
byte lectura=16;

// Esta funcion va a ser la que usemos para que se establezca la conexion WiFi

void conecta_wifi() {
  Serial.printf("\nConnecting to %s:\n", ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected, IP address: %s\n", WiFi.localIP().toString().c_str());
}
//----------------------------------------------
void conecta_mqtt() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(ID_PLACA, mqtt_user, mqtt_pass)) {
      Serial.printf(" conectado a broker: %s\n",mqtt_server);
      mqtt_client.subscribe("II3/ESP3097253/mov_zumbador");
      mqtt_client.subscribe("II3/ESP9590678/LED");
    } else {
      Serial.printf("failed, rc=%d  try again in 5s\n", mqtt_client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//-----------------------------------------------------
// Creamos funcion para recibir mensajes por MQTT
//-----------------------------------------------------
  void callback(char* topic, byte* payload, unsigned int length) {
  char *mensaje = (char *)malloc(length+1); // reservo memoria para copia del mensaje
  strncpy(mensaje, (char*)payload,length); // copio el mensaje en cadena de caracteres
  Serial.printf("Mensaje recibido [%s] %s\n", topic, mensaje);

//LED --------------------------
  
  if(strcmp(topic,"II3/ESP9590678/LED")==0) {

    StaticJsonDocument<512> root; // el tamaño tiene que ser adecuado para el mensaje
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(root, mensaje,length);

    // Compruebo si no hubo error
    if (error) {
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }
    else
    if(root.containsKey("level"))  // comprobar si existe el campo/clave que estamos buscando
    { 
     int valor = root["level"];
     int valor2=255-valor*2.55;
     Serial.print("Mensaje OK, level = ");
     Serial.println(valor);
     analogWrite(LED1, valor2);
     char cadena[128];
     snprintf(cadena, 128,"{\"led\":%i}",valor);
     mqtt_client.publish("II3/ESP9590678/Estado_LED",cadena);
    }
    else
    {
      Serial.print("Error : ");
      Serial.println("\"level\" key not found in JSON");
    }
  }

 //ZUMBADOR --------------------------
 
  if(strcmp(topic,"II3/ESP3097253/mov_zumbador")==0) {
    
    StaticJsonDocument<512> root; // el tamaño tiene que ser adecuado para el mensaje
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(root, mensaje,length);

    // Compruebo si no hubo error
    if (error) {
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }
    else
    if(root.containsKey("level"))  // comprobar si existe el campo/clave que estamos buscando
    { 
     int valor = root["level"];
     
     if (valor == 1)
     {
      for (int i=0;i<5;i++){
      analogWrite (zumbador,10);
      //float lec = digitalRead(lectura);
       //Serial.println (lec);
      delay (1000);
      analogWrite (zumbador,0);
      delay (1000);
      Serial.println ("ha sonado");
      }
       //float lec = digitalRead(lectura);
       //Serial.println (lec);
     }
     Serial.print("Mensaje OK, level = ");
     valor = 0;
     
     
    }
    else
    {
      Serial.print("Error : ");
      Serial.println("\"level\" key not found in JSON");
    }

  }

  free(mensaje);
  }

  //--------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Empieza setup...");
  
  dht.setup(5, DHTesp::DHT11); // Connect DHT sensor to GPIO 17
  sprintf(ID_PLACA, "%d", ESP.getChipId());
  pinMode (zumbador,OUTPUT);
  conecta_wifi();
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setBufferSize(512); // para poder enviar mensajes de hasta X bytes
  
  mqtt_client.setCallback(callback);
  conecta_mqtt();
 
  Serial.printf("Identificador placa: %s\n", ID_PLACA );
  Serial.printf("Termina setup en %lu ms\n\n",millis());

}

void loop() {
  delay(dht.getMinimumSamplingPeriod());
  if (!mqtt_client.connected()) conecta_mqtt();
  mqtt_client.loop(); // esta llamada para que la librería recupere el control
  unsigned long ahora = millis();
}
