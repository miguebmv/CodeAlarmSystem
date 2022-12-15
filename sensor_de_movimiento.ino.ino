//PLACA DE MIGUE--> ESP_3097139 
//Placa de pablo (usada aquí) -----> ESP_3097253

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <ArduinoJson.h>
DHTesp dht;

ADC_MODE(ADC_VCC);

WiFiClient wClient;
PubSubClient mqtt_client(wClient);

// Introducimos los valores de nuestra red WiFi
const char* ssid = "Redmi Note 9"; //"Redmi Note 9"; //"DIGIFIBRA-f2CX";
const char* password = "lol99999"; //"lol99999"; // "ekhXP6XD4S";
const char* mqtt_server = "iot.ac.uma.es";

const char* mqtt_user = "II3";
const char* mqtt_pass = "79qUYsdM";

// La placa que vamos a utilizar en el proyecto es ESP_3097253
// cadenas para topics e ID correspondiente con la placa utilizada

char ID_PLACA[16];
char topic_PUB[256];
char topic_SUB[256];

// GPIOs
int LED1 = 2;  
int LED2 = 16; 

//-----------------------------------------------------
byte sensor=5; //GPIO del sensor
int mov; 
int mov_tot=0;
int par;


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

//-----------------------------------------------------

// Esta funcion va a ser la que usemos para que se establezca la conexion MQTT

void conecta_mqtt() {
  char cadenaConectado[128];  // cadena de 128 caracteres 
  char cadenaDesconectado[128];  // cadena de 128 caracteres 
  // A continuación, creamos una cadena de caracteres de 128 de longitud donde guardaremos 
  // el mensaje de ultima voluntad indicando que nos hemos desconectado repentinamente
  snprintf(cadenaConectado, 128,"{\"online\": false}");
  snprintf(cadenaDesconectado, 128,"{\"online\": true}");
  // Mientras que no establezcamos la conexion no saldremos del bucle
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(ID_PLACA,mqtt_user,mqtt_pass,"II3/ESP3097253/conexion",2,true,cadenaDesconectado)) {
      Serial.printf(" conectado a broker: %s\n",mqtt_server);
      mqtt_client.subscribe("II3/ESP3097253/LED");
      mqtt_client.publish("II3/ESP3097139/conexion",cadenaConectado);
    } else {
      Serial.printf("failed, rc=%d  try again in 5s\n", mqtt_client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//-----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Empieza setup...");
  pinMode (sensor, INPUT); //dice que el sensor es de tipo entrada  
  digitalWrite(LED1, HIGH); // apaga el led
  digitalWrite(LED2, HIGH); 
  dht.setup(5, DHTesp::DHT11); // Connect DHT sensor to GPIO 17
  // crea topics usando id de la placa
  sprintf(ID_PLACA, "%d", ESP.getChipId());
  conecta_wifi();
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setBufferSize(512); // para poder enviar mensajes de hasta X bytes
  conecta_mqtt();
  Serial.printf("Identificador placa: %s\n", ID_PLACA );
  Serial.printf("Termina setup en %lu ms\n\n",millis());
  
}

//-----------------------------------------------------
//     LOOP
//-----------------------------------------------------
void loop() {
  delay(dht.getMinimumSamplingPeriod());
  if (!mqtt_client.connected()) conecta_mqtt();
  mqtt_client.loop(); // esta llamada para que la librería recupere el control
  unsigned long ahora = millis();
  if(digitalRead(sensor)==HIGH)
  {
    //TARDA 1:20 MINUTOS APROX 
    Serial.println("Detectado movimiento por el sensor");
    mov=1;
    mov_tot=mov_tot+1;
    char cadena [128];
    Serial.println (mov_tot);
    if (mov_tot % 2 == 0)
    { // La condición que hemos impuesto
      par = 1;
     Serial.println("Es par.") ; 
   }
    else {
      par = 0;
      Serial.println("Es impar");
    
        }
    snprintf(cadena,512,"{\"mov\":%d,\"mov_tot\":%d,\"par\":%d}",mov, mov_tot,par);
    mqtt_client.publish("II3/ESP3097253/movimiento",cadena);
    delay(5000);
    mov = 0;
  }
}
