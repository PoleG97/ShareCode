#include <esp_now.h>
#include <WiFi.h>

// Definicion de pines
const int camara = 23;

// Estructura para el envío de datos
typedef struct struct_message {
  bool disparo;
} struct_message;;

// Creamos un estructura
struct_message myData;

// Callback cuando recibimos datos
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  int valorDisparo = myData.disparo;

  if (valorDisparo == 1){
    // 1 solo disparo
    digitalWrite(camara, HIGH);
    delay(10);
    digitalWrite(camara, LOW);
  }
  else if (valorDisparo == 2){
    // Vídeo
    int tiempo = 0;

    digitalWrite(camara, HIGH);
    delay(10);
    

    while(tiempo <= 16){
      tiempo++;
      delay(1000);

      if(tiempo == 15){
        digitalWrite(camara, HIGH);
        delay(10);
        digitalWrite(camara, LOW);
        break;
      }
    }
    tiempo = 0;
  }
}
 
void setup() {
  // Iniciamos el puerto serie
  Serial.begin(115200);
  
  // Definimos el WiFi modo estación
  WiFi.mode(WIFI_STA);

  // Inicializar el espnow
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Con el espnow inicializado registramos para recibir cb, y comprobamos el estado
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(camara, OUTPUT);
}
 
void loop() {

}