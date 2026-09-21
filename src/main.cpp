#include <Arduino.h>
#include <DHT.h>
#include <TM1637Display.h>
#include "BluetoothSerial.h"

#define Relay_Pin 2
#define DHT_PIN 15
#define DHT_TYPE DHT11
#define CLK 18
#define DIO 19

BluetoothSerial SerialBT;
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK, DIO);

// Limites padrão (evita ligar o relé no boot)
float TemperaturaLimite = 30.0;
float HumidadeLimite = 70.0;

const uint8_t SEG_GRAU = SEG_A | SEG_B | SEG_F | SEG_G;
const uint8_t SEG_U    = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;

unsigned long ultimaTrocaDisplay = 0;
unsigned long ultimaLeitura = 0;
bool mostrandoTemperatura = true;
bool releLigado = false;

float temperature = 0;
float humidity = 0;

enum Estado { MENU, AGUARDA_TEMP, AGUARDA_HUM };
Estado estado = MENU;
String buffer = "";

// ---------- Interface Bluetooth ----------
void linha() {
  SerialBT.println("══════════════════════════");
}

void enviarMenu() {
  SerialBT.println();
  linha();
  SerialBT.println("   🌡️ CONTROLE ESP32 DHT11");
  linha();
  SerialBT.println(" [1] Definir temp. limite");
  SerialBT.println(" [2] Definir umid. limite");
  SerialBT.println(" [3] Ver status");
  SerialBT.println(" [0] Mostrar este menu");
  SerialBT.println("──────────────────────────");
  SerialBT.println(" Atalhos: T28.5  |  U65");
  linha();
}

void enviarStatus() {
  SerialBT.println();
  linha();
  SerialBT.println("        📊 STATUS");
  linha();
  SerialBT.printf(" Temperatura : %.1f °C\n", temperature);
  SerialBT.printf(" Umidade     : %.1f %%\n", humidity);
  SerialBT.println("──────────────────────────");
  SerialBT.printf(" Limite temp : %.1f °C\n", TemperaturaLimite);
  SerialBT.printf(" Limite umid : %.1f %%\n", HumidadeLimite);
  SerialBT.println("──────────────────────────");
  SerialBT.printf(" Lâmpada/Relé: %s\n", releLigado ? "🟢 LIGADO" : "🔴 DESLIGADO");
  linha();
}

// ---------- Utilidades ----------
bool lerNumero(const String &s, float &valor) {
  if (s.length() == 0) return false;
  char *fim;
  String t = s;
  t.replace(',', '.');
  valor = strtod(t.c_str(), &fim);
  return (*fim == '\0');
}

void definirTemp(float v) {
  if (v < 0 || v > 50) {
    SerialBT.println("❌ Valor inválido! Use 0 a 50 °C.");
    return;
  }
  TemperaturaLimite = v;
  SerialBT.printf("✅ Limite de temperatura: %.1f °C\n", v);
}

void definirHum(float v) {
  if (v < 0 || v > 100) {
    SerialBT.println("❌ Valor inválido! Use 0 a 100 %.");
    return;
  }
  HumidadeLimite = v;
  SerialBT.printf("✅ Limite de umidade: %.1f %%\n", v);
}

void tratarComando(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  float v;

  // Aguardando valor após escolher opção 1 ou 2
  if (estado == AGUARDA_TEMP) {
    if (lerNumero(cmd, v)) { definirTemp(v); estado = MENU; }
    else SerialBT.println("❌ Digite apenas o número (ex: 28.5)");
    return;
  }
  if (estado == AGUARDA_HUM) {
    if (lerNumero(cmd, v)) { definirHum(v); estado = MENU; }
    else SerialBT.println("❌ Digite apenas o número (ex: 65)");
    return;
  }

  // Menu
  if (cmd == "1") {
    SerialBT.println("🌡️ Digite a temperatura limite (°C):");
    estado = AGUARDA_TEMP;
  } else if (cmd == "2") {
    SerialBT.println("💧 Digite a umidade limite (%):");
    estado = AGUARDA_HUM;
  } else if (cmd == "3") {
    enviarStatus();
  } else if (cmd == "0") {
    enviarMenu();
  } else if ((cmd[0] == 'T' || cmd[0] == 't') && lerNumero(cmd.substring(1), v)) {
    definirTemp(v);
  } else if ((cmd[0] == 'U' || cmd[0] == 'u') && lerNumero(cmd.substring(1), v)) {
    definirHum(v);
  } else {
    SerialBT.println("❓ Comando inválido. Digite 0 para o menu.");
  }
}

void lerBluetooth() {
  while (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (buffer.length() > 0) {
        tratarComando(buffer);
        buffer = "";
      }
    } else {
      buffer += c;
    }
  }
}

// ---------- Display ----------
void mostrarValor(float valor, uint8_t simboloFinal) {
  int inteiro = (int)(valor * 10 + 0.5);
  uint8_t digitos[4];
  digitos[0] = display.encodeDigit((inteiro / 100) % 10);
  digitos[1] = display.encodeDigit((inteiro / 10) % 10) | 0x80;
  digitos[2] = display.encodeDigit(inteiro % 10);
  digitos[3] = simboloFinal;
  display.setSegments(digitos);
}

// ---------- Setup / Loop ----------
void setup() {
  pinMode(Relay_Pin, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  SerialBT.begin("ESP32_DHT11");
  display.setBrightness(7);
}

void loop() {
  // Novo cliente conectou? Mostra o menu
  static bool conectadoAntes = false;
  bool conectado = SerialBT.hasClient();
  if (conectado && !conectadoAntes) {
    delay(500);
    enviarMenu();
  }
  conectadoAntes = conectado;

  // DHT11 precisa de ~2s entre leituras
  if (millis() - ultimaLeitura >= 2000) {
    ultimaLeitura = millis();
    float novaTemp = dht.readTemperature();
    float novaHum = dht.readHumidity();
    if (isnan(novaTemp) || isnan(novaHum)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      temperature = novaTemp;
      humidity = novaHum;
    }
  }

  lerBluetooth();

  // Controle do relé
  bool deveLigar = (temperature > TemperaturaLimite || humidity > HumidadeLimite);
  digitalWrite(Relay_Pin, deveLigar ? HIGH : LOW);

  // Avisa no Bluetooth quando o relé muda de estado
  if (deveLigar != releLigado) {
    releLigado = deveLigar;
    SerialBT.println();
    SerialBT.printf(releLigado ? "🟢 Relé LIGADO  (%.1f°C / %.1f%%)\n"
                               : "🔴 Relé DESLIGADO (%.1f°C / %.1f%%)\n",
                    temperature, humidity);
  }

  // Alterna display
  if (millis() - ultimaTrocaDisplay >= 3000) {
    ultimaTrocaDisplay = millis();
    mostrandoTemperatura = !mostrandoTemperatura;
  }
  if (mostrandoTemperatura) mostrarValor(temperature, SEG_GRAU);
  else mostrarValor(humidity, SEG_U);

  delay(50);
}