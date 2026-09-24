#include <Arduino.h>
#include <DHT.h>
#include <TM1637Display.h>
#include "BluetoothSerial.h"

#define LED 4
#define DHTPIN 15
#define DHTTYPE DHT11
#define CLK 18
#define DIO 19

BluetoothSerial BT;
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

float limiteT = 30, limiteH = 70;
float temp = 0, hum = 0;
bool led = false;
bool mostraTemp = true;

unsigned long tLeitura = 0;
unsigned long tDisplay = 0;

enum Estado { MENU, TEMP, HUM };
Estado estado = MENU;

String buffer;

const uint8_t GRAU = SEG_A | SEG_B | SEG_F | SEG_G;
const uint8_t U = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;

void menu() {
  BT.println("\n=== ESP32 DHT11 ===");
  BT.println("1 - Limite temperatura");
  BT.println("2 - Limite umidade");
  BT.println("3 - Status");
  BT.println("0 - Menu");
  BT.println("Atalhos: T28.5 / U65");
}

void status() {
  BT.printf("\nTemp: %.1f C\n", temp);
  BT.printf("Umid: %.1f %%\n", hum);
  BT.printf("Limite T: %.1f C\n", limiteT);
  BT.printf("Limite U: %.1f %%\n", limiteH);
  BT.printf("LED: %s\n", led ? "LIGADO" : "DESLIGADO");
}

bool numero(String s, float &v) {
  s.trim();
  s.replace(',', '.');

  if (!s.length()) return false;

  char *fim;
  v = strtod(s.c_str(), &fim);

  return *fim == '\0';
}

void comando(String c) {
  c.trim();

  if (!c.length()) return;

  float v;

  if (estado == TEMP) {
    if (numero(c, v) && v >= 0 && v <= 50) {
      limiteT = v;
      BT.printf("Limite T: %.1f C\n", v);
      estado = MENU;
    } else {
      BT.println("Valor invalido: 0 a 50");
    }

    return;
  }

  if (estado == HUM) {
    if (numero(c, v) && v >= 0 && v <= 100) {
      limiteH = v;
      BT.printf("Limite U: %.1f %%\n", v);
      estado = MENU;
    } else {
      BT.println("Valor invalido: 0 a 100");
    }

    return;
  }

  if (c == "1") {
    BT.println("Digite o limite de temperatura:");
    estado = TEMP;
  }
  else if (c == "2") {
    BT.println("Digite o limite de umidade:");
    estado = HUM;
  }
  else if (c == "3") {
    status();
  }
  else if (c == "0") {
    menu();
  }
  else if ((c[0] == 'T' || c[0] == 't') &&
           numero(c.substring(1), v) &&
           v >= 0 && v <= 50) {

    limiteT = v;
    BT.printf("Limite T: %.1f C\n", limiteT);
  }
  else if ((c[0] == 'U' || c[0] == 'u') &&
           numero(c.substring(1), v) &&
           v >= 0 && v <= 100) {

    limiteH = v;
    BT.printf("Limite U: %.1f %%\n", limiteH);
  }
  else {
    BT.println("Comando invalido");
  }
}

void bluetooth() {
  while (BT.available()) {
    char c = BT.read();

    if (c == '\n' || c == '\r') {

      if (buffer.length()) {
        comando(buffer);
        buffer = "";
      }

    } else {
      buffer += c;
    }
  }
}

void mostrar(float valor, uint8_t simbolo) {

  int n = valor * 10 + 0.5;

  uint8_t d[4] = {

    display.encodeDigit((n / 100) % 10),

    (uint8_t)(
      display.encodeDigit((n / 10) % 10) | 0x80
    ),

    display.encodeDigit(n % 10),

    simbolo
  };

  display.setSegments(d);
}

void setup() {

  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);

  Serial.begin(115200);

  dht.begin();

  BT.begin("ESP32_DHT11");

  display.setBrightness(7);
  display.clear();
}

void loop() {

  static bool conectado = false;

  bool c = BT.hasClient();

  if (c && !conectado) {
    delay(500);
    menu();
  }

  conectado = c;

  // Leitura do DHT11
  if (millis() - tLeitura >= 2000) {

    tLeitura = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {

      temp = t;
      hum = h;
    }
  }

  // Bluetooth
  bluetooth();

  // LED acende quando temperatura OU umidade ultrapassar o limite
  bool novoAlerta = temp > limiteT || hum > limiteH;

  if (novoAlerta != led) {

    led = novoAlerta;

    digitalWrite(LED, led);

    BT.printf(
      "LED %s - %.1f C / %.1f %%\n",
      led ? "LIGADO" : "DESLIGADO",
      temp,
      hum
    );
  }

  // Alterna temperatura e umidade no display
  if (millis() - tDisplay >= 3000) {

    tDisplay = millis();

    mostraTemp = !mostraTemp;
  }

  mostrar(
    mostraTemp ? temp : hum,
    mostraTemp ? GRAU : U
  );

  delay(50);
}
