//PLACA DE MIGUE--> ESP_3097139
//PLACA DE PABLO--> ESP_3097253
//PLACA DE TELMO--> ESP_9590678

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <ArduinoJson.h>
DHTesp dht;

ADC_MODE(ADC_VCC);

struct registro_datos {
        float temperatura;
        float humedad;
        String ip;
        };


WiFiClient wClient;
PubSubClient mqtt_client(wClient);

// Introducimos los valores de nuestra red WiFi
const char* ssid =  "iPhone";//"DIGIFIBRA-f2CX";
const char* password = "aaaaaaaa";//"ekhXP6XD4S";
const char* mqtt_server = "iot.ac.uma.es";

const char* mqtt_user = "II3";
const char* mqtt_pass = "79qUYsdM";

// La placa que vamos a utilizar en el proyecto es ESP_3097139

char ID_PLACA[16];

// GPIOs
int LED1 = 2;  
int LED2 = 16; 


byte sensor=4;
char mensaje_pulsacion[10];

//-----------------------------------------------------

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
  // Mienrtas que no establezcamos la conexion no saldremos del bucle
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(ID_PLACA,mqtt_user,mqtt_pass,"II3/ESP3097139/conexion",2,true,cadenaDesconectado)) {
      Serial.printf(" conectado a broker: %s\n",mqtt_server);
      mqtt_client.subscribe("II3/ESP9590678/LED");
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      //mqtt_client.publish("II3/ESP3097139/conexion",cadenacONECTADO);
    } else {
      Serial.printf("failed, rc=%d  try again in 5s\n", mqtt_client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//-----------------------------------------------------
String serializa_JSON (struct registro_datos datos)
    {
      StaticJsonDocument<300> jsonRoot;
      String jsonString;
     
      JsonObject DHT11=jsonRoot.createNestedObject("DHT11");
      DHT11["temp"] = datos.temperatura;
      DHT11["hum"] = datos.humedad;
      JsonObject Wifi=jsonRoot.createNestedObject("Wifi");
      Wifi["IP"]=datos.ip;
      
      
      serializeJson(jsonRoot,jsonString);
      return jsonString;
    }
//-----------------------------------------------------
//     SETUP
//-----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Empieza setup...");
  pinMode(LED1, OUTPUT);    // inicializa GPIO como salida
  pinMode(LED2, OUTPUT);    
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


  pinMode (sensor,INPUT);
}
//-----------------------------------------------------
#define TAMANHO_MENSAJE 128
unsigned long ultimo_mensaje=0;
//-----------------------------------------------------
//     LOOP
//-----------------------------------------------------
void loop() {
  delay(dht.getMinimumSamplingPeriod());
  if (!mqtt_client.connected()) conecta_mqtt();
  mqtt_client.loop(); // esta llamada para que la librería recupere el control
  unsigned long ahora = millis();

if (ahora - ultimo_mensaje >= 15000) {
    char mensaje[TAMANHO_MENSAJE];
    ultimo_mensaje = ahora;
    snprintf (mensaje, TAMANHO_MENSAJE, "Mensaje enviado desde %s en %6lu ms", ID_PLACA, ahora);
    Serial.println(mensaje);    
    struct registro_datos misdatos;
    
    misdatos.humedad = dht.getHumidity();
    misdatos.temperatura = dht.getTemperature();
    misdatos.ip=WiFi.localIP().toString().c_str();

    mqtt_client.publish("II3/ESP3097139/TempHum",serializa_JSON(misdatos).c_str());
  
  }


  if(digitalRead(sensor) == HIGH)
  {
    Serial.println ("Has tocado el sensor");
    snprintf (mensaje_pulsacion, TAMANHO_MENSAJE, "1");
    mqtt_client.publish("II3/ESP3097139/pulsacion",mensaje_pulsacion);
    delay (10000);

  }
  
  
}
