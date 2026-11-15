#include <WiFiS3.h>
#include "Arduino_LED_Matrix.h"

// ====== DATOS QUE CADA EQUIPO DEBE CAMBIAR ======
const char* RED_WIFI   = "NOMBRE_DE_TU_RED";
const char* CLAVE_WIFI = "CLAVE_DE_TU_RED";
const char* HOST_MAKE  = "hook.us2.make.com";                      // dominio de tu webhook
const char* RUTA_MAKE  = "/wxtsuhdykyxb20du76oonnx0bkn37ptf";      // lo que sigue después del dominio
// =================================================

const int PIN_DEMANDA = A0;
const int PIN_OFERTA  = A1;
const unsigned long ESPERA_ALERTA = 60000;   // max 1 alerta por minuto

ArduinoLEDMatrix matriz;
WiFiSSLClient cliente;
bool enEscasez = false;
unsigned long ultimaAlerta = 0;

// Dígitos 3x5 para la matriz LED (cada número = 5 filas de 3 bits)
const byte DIGITOS[10][5] = {
  {7,5,5,5,7}, {2,6,2,2,7}, {7,1,7,4,7}, {7,1,7,1,7}, {5,5,7,1,1},
  {7,4,7,1,7}, {7,4,7,5,7}, {7,1,1,1,1}, {7,5,7,5,7}, {7,5,7,1,7}
};

uint8_t frame[8][12];

void dibujarDigito(int d, int xInicio) {
  for (int fila = 0; fila < 5; fila++) {
    for (int col = 0; col < 3; col++) {
      frame[fila + 1][xInicio + col] = (DIGITOS[d][fila] >> (2 - col)) & 1;
    }
  }
}

void mostrarPrecio(int precio, bool escasez) {
  for (int f = 0; f < 8; f++) for (int c = 0; c < 12; c++) frame[f][c] = 0;
  precio = constrain(precio, 0, 999);
  dibujarDigito(precio / 100, 0);
  dibujarDigito((precio / 10) % 10, 4);
  dibujarDigito(precio % 10, 8);
  if (escasez) frame[0][11] = 1;            // punto en la esquina = escasez
  matriz.renderBitmap(frame, 8, 12);
}

void enviarAMake(int demanda, int oferta, int precio) {
  if (!cliente.connect(HOST_MAKE, 443)) {
    Serial.println("No se pudo conectar con Make");
    return;
  }
  String ruta = String(RUTA_MAKE) + "?demanda=" + String(demanda) + "&oferta=" + String(oferta)
              + "&precio=" + String(precio) + "&estado=escasez";
  cliente.println("GET " + ruta + " HTTP/1.1");
  cliente.println("Host: " + String(HOST_MAKE));
  cliente.println("Connection: close");
  cliente.println();
  delay(500);
  cliente.stop();
  Serial.println("Alerta enviada a Make");
}

void setup() {
  Serial.begin(9600);
  matriz.begin();
  mostrarPrecio(888, false);               // prueba: enciende todos los dígitos
  Serial.println("Conectando a WiFi...");
  while (WiFi.begin(RED_WIFI, CLAVE_WIFI) != WL_CONNECTED) {
    Serial.println("Reintentando WiFi...");
    delay(3000);
  }
  Serial.println("WiFi conectado");
}

void loop() {
  int demanda = map(analogRead(PIN_DEMANDA), 0, 1023, 10, 90);
  int oferta  = map(analogRead(PIN_OFERTA), 0, 1023, 10, 90);

  // Modelo lineal: demanda P = a - Q ; oferta P = Q - b
  int a = 100 + demanda;
  int b = oferta - 50;
  int precio = ((a - b) / 2) * 4;   // pesos por kilo

  bool escasez = (demanda - oferta) >= 25;
  mostrarPrecio(precio, escasez);

  Serial.print("Demanda: ");  Serial.print(demanda);
  Serial.print("  Oferta: "); Serial.print(oferta);
  Serial.print("  Precio: $"); Serial.println(precio);

  if (escasez && !enEscasez && millis() - ultimaAlerta > ESPERA_ALERTA) {
    enviarAMake(demanda, oferta, precio);
    ultimaAlerta = millis();
  }
  enEscasez = escasez;
  delay(300);
}
