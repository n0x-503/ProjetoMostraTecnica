#include <Arduino.h>
#include <DHT.h>
#include <TM1637Display.h>
#include "BluetoothSerial.h"

#define Relay_Pin 2
#define DHT_PIN 15
#define DHT_TYPE DHT11
#define CLK 18   // pino CLK do TM1637
#define DIO 19   // pino DIO do TM1637

BluetoothSerial SerialBT;
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK, DIO);

float TemperaturaLimite = 0;
float HumidadeLimite = 0;

// Segmentos customizados (bits: A B C D E F G DP)
const uint8_t SEG_GRAU = SEG_A | SEG_B | SEG_F | SEG_G;         // símbolo °
const uint8_t SEG_U    = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F; // letra U

unsigned long ultimaTrocaDisplay = 0;
bool mostrandoTemperatura = true;

float temperature = 0;
float humidity = 0;

void mostrarValor(float valor, uint8_t simboloFinal) {
  int inteiro = (int)(valor * 10 + 0.5); // ex: 22.2 -> 222 (com arredondamento)

  uint8_t digitos[4];
  digitos[0] = display.encodeDigit((inteiro / 100) % 10);
  digitos[1] = display.encodeDigit((inteiro / 10) % 10) | 0x80; // liga o ponto decimal
  digitos[2] = display.encodeDigit(inteiro % 10);
  digitos[3] = simboloFinal;

  display.setSegments(digitos);
}

void setup() {
  pinMode(Relay_Pin, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  SerialBT.begin("ESP32_DHT11");
  display.setBrightness(7);
}

void loop() {
  float novaTemp = dht.readTemperature();
  float novaHum = dht.readHumidity();

  if (isnan(novaTemp) || isnan(novaHum)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    temperature = novaTemp;
    humidity = novaHum;
  }

  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');
    int separatorIndex = data.indexOf(',');
    if (separatorIndex != -1) {
      String tempStr = data.substring(0, separatorIndex);
      String humStr = data.substring(separatorIndex + 1);
      TemperaturaLimite = tempStr.toFloat();
      HumidadeLimite = humStr.toFloat();
    }
  }

  if (temperature > TemperaturaLimite || humidity > HumidadeLimite) {
    digitalWrite(Relay_Pin, HIGH);
  } else {
    digitalWrite(Relay_Pin, LOW);
  }

  // Alterna temperatura/umidade no display a cada 3 segundos,
  // sem travar a leitura do Bluetooth (nada de delay(3000) aqui)
  if (millis() - ultimaTrocaDisplay >= 3000) {
    ultimaTrocaDisplay = millis();
    mostrandoTemperatura = !mostrandoTemperatura;
  }

  if (mostrandoTemperatura) {
    mostrarValor(temperature, SEG_GRAU);
  } else {
    mostrarValor(humidity, SEG_U);
  }

  delay(200);
}