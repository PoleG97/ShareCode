#include <Arduino.h>
#include <esp_now.h>
#include <wifi.h>

// Definicion de los interruptores del modo
const int modebit1 = 39;
const int modebit2 = 36;
// Definicion de los pines de los sensores
const int pirPin = 26;
const int barreraPin = 27;
const int ldrPin = 32;
const int lluviaPin = 33;         // LOW -> mojado
// Almacenar el valor de los pines de los sensores
int pirValor;
int barreraValor;
int ldrValor;
int lluviaValor;
int ref = 3500;
int modo;

// Comunicar con toda la red alcanzable
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Definir la estructura para el envío
typedef struct struct_message {
  bool disparo;
} struct_message;

// Crear la estructura
struct_message myData;

esp_now_peer_info_t peerInfo;

// Función de envío
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void trabajo(int modotrabajo){
  // Lectura de los sensores
  pirValor = digitalRead(pirPin);
  barreraValor = digitalRead(barreraPin);
  ldrValor = analogRead(ldrPin);
  lluviaValor = digitalRead(lluviaPin);

  // Modo de trabajo
  switch (modotrabajo){
  case 1:
    // Carrera
    if(barreraValor == LOW){myData.disparo = 1;}
    break;
  
  case 2:
    // Animales
    if((pirValor == HIGH)|| (barreraValor == LOW)){myData.disparo = 1;}
    break;

  case 3:
    // Vigilancia
    if((lluviaValor == LOW)|| (ldrValor > ref)){                              // Llueve o es de noche
      if(pirValor == HIGH){myData.disparo = 2;}                               // Detectamos gente solo con barrera
    } 
    else if((lluviaValor == HIGH) && (ldrValor < ref)){                       // NO Llueve o NO es de noche
      if((pirValor == HIGH)){myData.disparo = 2;}                             // Normalmente, dispara barrera o pir
      if((barreraValor== LOW)){myData.disparo = 2;}
    }
    break;
  
  case 4:
    // Parado
    myData.disparo = 0;
    break;
  
  default:
    myData.disparo = 0;
    break;
  }
}


void setup() {
  // Inicializamos el puerto serie
  Serial.begin(115200);

  //Definimos I/O
  pinMode(modebit1, INPUT);
  pinMode(modebit2, INPUT);
  pinMode(barreraPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(lluviaPin, INPUT);

  //------------------------------------------ RED
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
         
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  //-----------------------------------------------

  if(digitalRead(modebit1)==0 && digitalRead(modebit2)==0){modo=1;} // Carrera
  if(digitalRead(modebit1)==0 && digitalRead(modebit2)==1){modo=2;} // Animales
  if(digitalRead(modebit1)==1 && digitalRead(modebit2)==0){modo=3;} // Vigilancia
  if(digitalRead(modebit1)==1 && digitalRead(modebit2)==1){modo=4;} // Stop
}


void loop() {
  // Funcion de trabajo
  trabajo(modo);

  // Enviamos el mensaje y guardamos el estado de la llegada
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  myData.disparo = 0;
   
  // Comprobación del envío
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  Serial.println(myData.disparo);
}