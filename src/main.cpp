#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// ---------- Pinos ----------
#define DHTPIN   15
#define DHTTYPE  DHT11
#define LDR_PIN  34      
#define SOLO_PIN 35      
#define LED_VERMELHO 4   
#define LED_VERDE    5   

// ---------- Rede propria do ESP32 ----------
const char* ssid  = "Estufa_ESP32";
const char* senha = "12345678";   

)
int LDR_ESCURO = 200;     
int LDR_CLARO  = 3500;    


int SOLO_SECO  = 3300;    
int SOLO_UMIDO = 1400;    

// ---------- Pagina (HTML + CSS + JavaScript) ----------
#include "pagina.h"   // contem a constante PAGINA (HTML + CSS + JavaScript)

// ---------- Objetos ----------
WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);
Preferences prefs;

// ---------- Variaveis ----------
float temp = 0, hum = 0;
bool lido = false;            // ja houve leitura valida do DHT11?
int ldrBruto = 0;
float luz = 0;                // luminosidade em %
int soloBruto = 0;
float solo = 0;               // umidade do solo em %

// ---------- Planta escolhida e valores ideais ----------
String plantaId = "";             
float ideal[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};   
const char* CHAVES[8] = {"tmin", "tmax", "umin", "umax", "lmin", "lmax", "smin", "smax"};

String ledAtual = "off";         

unsigned long tLeitura = 0;

// ---------- LDR ----------
// Media de 16 leituras para reduzir ruido
int lerAnalogico(int pino) {
  long soma = 0;
  for (int i = 0; i < 16; i++) {
    soma += analogRead(pino);
    delay(2);
  }
  return soma / 16;
}

float luzPercentual(int bruto) {
  float p = (bruto - LDR_ESCURO) * 100.0 / (LDR_CLARO - LDR_ESCURO);
  return constrain(p, 0, 100);
}

// ---------- Solo ----------
float soloPercentual(int bruto) {
  float p = (SOLO_SECO - bruto) * 100.0 / (SOLO_SECO - SOLO_UMIDO);
  return constrain(p, 0, 100);
}

// ---------- LEDs ----------

void atualizarLeds() {
  if (!lido || plantaId.length() == 0) {
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, LOW);
    ledAtual = "off";
    return;
  }

  float valor[4] = {temp, hum, luz, solo};
  bool fora = false;

  for (int i = 0; i < 4; i++) {
    if (valor[i] < ideal[i][0] || valor[i] > ideal[i][1]) fora = true;
  }

  digitalWrite(LED_VERMELHO, fora ? HIGH : LOW);
  digitalWrite(LED_VERDE, fora ? LOW : HIGH);
  ledAtual = fora ? "vermelho" : "verde";
}

// ---------- Rotas ----------
// Envia a pagina em pedacos de 1 KB (mais seguro que um envio unico de ~10 KB)
void paginaInicial() {
  Serial.println("GET /");

  size_t total = strlen_P(PAGINA);

  server.sendHeader("Cache-Control", "no-store");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=utf-8", "");

  for (size_t i = 0; i < total; i += 1024) {
    size_t n = total - i;
    if (n > 1024) n = 1024;
    server.sendContent_P(PAGINA + i, n);
  }

  server.sendContent("");   // encerra o envio
  Serial.printf("Pagina enviada (%u bytes)\n", (unsigned)total);
}

void dados() {
  Serial.println("GET /dados");
  String json = "{";
  json += "\"temperatura\":" + (lido ? String(temp, 1) : String("null")) + ",";
  json += "\"umidade\":" + (lido ? String(hum, 1) : String("null")) + ",";
  json += "\"luz\":" + String(luz, 0) + ",";
  json += "\"solo\":" + String(solo, 0) + ",";
  json += "\"ldrBruto\":" + String(ldrBruto) + ",";
  json += "\"soloBruto\":" + String(soloBruto) + ",";
  json += "\"lido\":" + String(lido ? "true" : "false") + ",";
  json += "\"planta\":\"" + plantaId + "\",";
  json += "\"led\":\"" + ledAtual + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// Exemplo: /planta?id=rucula&tmin=15&tmax=25&umin=50&umax=70&lmin=50&lmax=80&smin=60&smax=80
// Sem id (/planta?id=) = nenhuma planta escolhida
void definirPlanta() {
  String id = server.arg("id");

  if (id.length() == 0) {
    plantaId = "";
    prefs.putString("planta", "");
    atualizarLeds();
    server.send(200, "text/plain", "Nenhuma planta");
    return;
  }

  bool ok = id.length() <= 20;
  for (unsigned i = 0; i < id.length() && ok; i++) {
    if (!isalnum((unsigned char)id[i])) ok = false;
  }

  float v[8];
  for (int i = 0; i < 8 && ok; i++) {
    if (!server.hasArg(CHAVES[i])) { ok = false; break; }
    v[i] = server.arg(CHAVES[i]).toFloat();
  }

  for (int i = 0; i < 4 && ok; i++) {
    if (v[2 * i] > v[2 * i + 1]) ok = false;   // minimo maior que maximo
  }

  if (!ok) {
    server.send(400, "text/plain", "Dados invalidos");
    return;
  }

  plantaId = id;
  prefs.putString("planta", id);

  for (int i = 0; i < 4; i++) {
    ideal[i][0] = v[2 * i];
    ideal[i][1] = v[2 * i + 1];
    prefs.putFloat(CHAVES[2 * i], ideal[i][0]);
    prefs.putFloat(CHAVES[2 * i + 1], ideal[i][1]);
  }

  Serial.printf("Planta: %s | LED depende de: temp %.0f-%.0f, umid %.0f-%.0f, luz %.0f-%.0f, solo %.0f-%.0f\n",
                id.c_str(), v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);

  atualizarLeds();
  server.send(200, "text/plain", "Planta salva");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);
  pinMode(SOLO_PIN, INPUT);

  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_VERDE, LOW);

  dht.begin();

  // Recupera a planta escolhida (continua valendo se o ESP32 for desligado)
  prefs.begin("estufa", false);
  plantaId = prefs.getString("planta", "");
  for (int i = 0; i < 4; i++) {
    ideal[i][0] = prefs.getFloat(CHAVES[2 * i], 0);
    ideal[i][1] = prefs.getFloat(CHAVES[2 * i + 1], 0);
  }

  WiFi.softAP(ssid, senha);

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", paginaInicial);
  server.on("/dados", dados);
  server.on("/planta", definirPlanta);
  server.begin();
}

// ---------- Loop ----------
void loop() {
  server.handleClient();

  // Leitura dos sensores a cada 2 s
  if (millis() - tLeitura >= 2000) {
    tLeitura = millis();

    // LDR
    ldrBruto = lerAnalogico(LDR_PIN);
    luz = luzPercentual(ldrBruto);

    // Solo
    soloBruto = lerAnalogico(SOLO_PIN);
    solo = soloPercentual(soloBruto);

    // DHT11
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temp = t;
      hum = h;
      lido = true;
    }

    // Decide qual LED acender
    atualizarLeds();

    // Use os valores brutos para calibrar LDR_ESCURO/LDR_CLARO e SOLO_SECO/SOLO_UMIDO
    Serial.printf("LDR bruto: %d (%.0f %%) | Solo bruto: %d (%.0f %%) | Temp: %.1f C | Umid ar: %.1f %% | Planta: %s | LED: %s\n",
                  ldrBruto, luz, soloBruto, solo, temp, hum,
                  plantaId.length() ? plantaId.c_str() : "nenhuma", ledAtual.c_str());
  }
}
