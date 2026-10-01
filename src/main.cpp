#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>

// ---------- Pinos ----------
#define DHTPIN    15
#define DHTTYPE   DHT11
#define LDR_PIN   34      // use pino ADC1 (32 a 39): o ADC2 nao funciona com Wi-Fi ligado
#define LED_STATUS 2      // LED interno da maioria das placas ESP32 (ou conecte um LED verde no GPIO 2)

// ---------- Rede propria do ESP32 ----------
const char* ssid  = "Estufa_ESP32";
const char* senha = "12345678";   // minimo 8 caracteres

// ---------- Calibracao do LDR (leitura bruta, 0 a 4095) ----------
int LDR_ESCURO = 200;     // leitura com o sensor no escuro  -> 0 %
int LDR_CLARO  = 3500;    // leitura com a luz maxima        -> 100 %

// ---------- Pagina (HTML + CSS + JavaScript) ----------
const char PAGINA[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Estufa Inteligente</title>
<style>
*{box-sizing:border-box}
body{margin:0;padding:22px 14px 40px;background:#e6f2e8;
font-family:Arial,Helvetica,sans-serif;color:#1d2b22}
.wrap{max-width:640px;margin:0 auto}
h1{text-align:center;margin:0;font-size:28px;color:#1f5e3c}
.sub{text-align:center;margin:4px 0 18px;opacity:.7}
.box{background:#fff;border-radius:14px;padding:18px;margin-bottom:14px;
box-shadow:0 3px 10px rgba(0,0,0,.12)}
.box:empty{display:none}
.box h3{margin:0 0 10px;font-size:17px}
.box p{margin:0 0 12px;font-size:15px;line-height:1.45}
.box ul{margin:0;padding-left:20px;font-size:15px;line-height:1.5}
label{font-weight:bold;font-size:15px}
input{width:100%;margin-top:8px;padding:11px;font-size:18px;border:2px solid #7fb58f;
border-radius:8px;background:#f4faf5}
input:focus{outline:3px solid rgba(47,125,79,.35)}
#np{margin:8px 0 0;color:#c4393b;font-size:14px;font-weight:bold;min-height:18px}
.chips{display:flex;flex-wrap:wrap;gap:8px;margin-top:10px}
.chip{padding:8px 14px;border:2px solid #2f7d4f;border-radius:20px;background:#fff;
color:#2f7d4f;font-weight:bold;font-size:14px;cursor:pointer;transition:.2s}
.chip:hover{background:#e3f3e8}
.chip.on{background:#2f7d4f;color:#fff}
#res{padding:16px;border-radius:14px;text-align:center;font-size:19px;font-weight:bold;
color:#fff;background:#6b7280;margin-bottom:14px;box-shadow:0 3px 10px rgba(0,0,0,.2);
transition:background .4s}
#res.ok{background:#2e9e5b}
#res.at{background:#d89a00}
#res.aj{background:#c4393b}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(170px,1fr));gap:12px;margin-bottom:14px}
.card{background:#fff;border-radius:14px;padding:14px;border-top:5px solid #cbd5d0;
box-shadow:0 3px 10px rgba(0,0,0,.12)}
.card.ok{border-color:#2e9e5b}
.card.at{border-color:#d89a00}
.card.aj{border-color:#c4393b}
.card h2{margin:0;font-size:13px;text-transform:uppercase;letter-spacing:1px;opacity:.65}
.v{font-size:36px;font-weight:bold;margin:6px 0 2px}
.v small{font-size:16px;font-weight:normal}
.ideal{font-size:13px;opacity:.75;margin-bottom:12px}
.bar{position:relative;height:10px;border-radius:5px;background:#dde5e0}
.zona{position:absolute;top:0;bottom:0;background:#8fd0a5;border-radius:5px}
.mk{position:absolute;top:-4px;width:4px;height:18px;border-radius:2px;
background:#1d2b22;transform:translateX(-50%)}
.est{font-size:13px;font-weight:bold;margin-top:10px}
.nota{text-align:center;font-size:12px;opacity:.6;margin-top:6px}
</style>
</head>
<body>
<div class="wrap">
<h1>Estufa Inteligente</h1>
<p class="sub">Monitoramento com ESP32, DHT11 e LDR</p>

<div class="box">
<label for="pl">Planta cultivada</label>
<input id="pl" list="lista" placeholder="Digite ou escolha (ex.: Rúcula)" autocomplete="off">
<datalist id="lista"></datalist>
<div class="chips" id="chips"></div>
<p id="np"></p>
</div>

<div id="res">Escolha uma planta para comparar</div>
<div class="grid" id="cards"></div>
<div class="box" id="exp"></div>
<div class="box" id="aj"></div>
<p class="nota">Valores de referência gerais. A luminosidade é relativa ao sensor calibrado (100 % = luz máxima).</p>
</div>

<script>
const PLANTAS=[{"id":"rucula","nome":"Rúcula","nome_cientifico":"Eruca vesicaria subsp. sativa","luz_pct":{"min":50,"max":80,"motivo":"Folhosa de ciclo curto: precisa de boa luz para fotossíntese e folhas vigorosas, mas tolera meia-sombra. Luz excessiva junto com calor acelera o florescimento."},"temperatura_c":{"min":15,"max":25,"motivo":"Planta de clima ameno. Acima de cerca de 25 a 28 °C ela emite a haste floral (pendoamento), e as folhas ficam mais amargas e picantes."},"umidade_pct":{"min":50,"max":70,"motivo":"Umidade moderada mantém as folhas firmes sem favorecer fungos. Umidade muito alta aumenta o risco de míldio e podridões."}},{"id":"alface","nome":"Alface","nome_cientifico":"Lactuca sativa","luz_pct":{"min":50,"max":70,"motivo":"Cresce bem com luz moderada. Luz excessiva com calor estressa a planta e estimula o pendoamento; pouca luz deixa as folhas alongadas e fracas (estiolamento)."},"temperatura_c":{"min":15,"max":22,"motivo":"Faixa em que a alface forma folhas macias e doces. Acima de uns 25 °C a planta emite o pendão floral e produz látex, deixando as folhas amargas."},"umidade_pct":{"min":60,"max":80,"motivo":"Umidade mais alta reduz a perda de água das folhas, que são finas e delicadas. Acima de 80% crescem doenças fúngicas como míldio e podridão."}},{"id":"manjericao","nome":"Manjericão","nome_cientifico":"Ocimum basilicum","luz_pct":{"min":70,"max":100,"motivo":"Precisa de sol pleno (6 a 8 horas por dia). A luz intensa aumenta a produção dos óleos essenciais responsáveis pelo aroma e pelo sabor."},"temperatura_c":{"min":20,"max":30,"motivo":"Espécie tropical e sensível ao frio: abaixo de uns 10 °C as folhas escurecem e a planta pode morrer. O crescimento é melhor em clima quente."},"umidade_pct":{"min":40,"max":60,"motivo":"Prefere o ar mais seco que as folhosas. Umidade alta com pouca ventilação favorece fungos como míldio e mofo."}},{"id":"tomate","nome":"Tomate","nome_cientifico":"Solanum lycopersicum","luz_pct":{"min":80,"max":100,"motivo":"Exige muita luz: ela determina a fotossíntese, o número de flores e o acúmulo de açúcares nos frutos. Pouca luz gera plantas alongadas e poucos frutos."},"temperatura_c":{"min":20,"max":28,"motivo":"Entre cerca de 20 e 28 °C a polinização e o pegamento dos frutos são melhores. Acima de uns 32 °C ou abaixo de 13 °C as flores caem e os frutos se formam mal."},"umidade_pct":{"min":60,"max":70,"motivo":"Nessa faixa o pólen é viável e a planta transpira bem. Acima de 80% aumentam doenças como a requeima e o mofo; umidade muito baixa seca o pólen e prejudica a frutificação."}},{"id":"morango","nome":"Morango","nome_cientifico":"Fragaria × ananassa","luz_pct":{"min":70,"max":90,"motivo":"Precisa de bastante luz para formar flores e açúcar nos frutos. Em calor extremo, um leve sombreamento evita queimaduras nas folhas e nos frutos."},"temperatura_c":{"min":15,"max":25,"motivo":"É a faixa de melhor floração e de frutos mais doces. Acima de uns 30 °C a produção cai e os frutos ficam pequenos; frio forte paralisa o crescimento."},"umidade_pct":{"min":60,"max":75,"motivo":"Umidade moderada mantém as folhas saudáveis. O excesso, principalmente com folhas molhadas, favorece o mofo cinzento (Botrytis) nos frutos."}}];

const $=i=>document.getElementById(i);
const norm=s=>s.normalize('NFD').replace(/[\u0300-\u036f]/g,'').toLowerCase().trim();
const M=[
 {k:'luz_pct',n:'Luminosidade',u:'%',mx:100,mg:10,v:'luz'},
 {k:'temperatura_c',n:'Temperatura',u:'°C',mx:45,mg:2,v:'temperatura'},
 {k:'umidade_pct',n:'Umidade',u:'%',mx:100,mg:5,v:'umidade'}];
const DICA={
 luz_pct:['Aumente a iluminação ou retire o sombreamento.','Use tela de sombreamento ou afaste a planta da luz.'],
 temperatura_c:['Aqueça ou feche a estufa para reter calor.','Ventile ou sombreie a estufa para baixar a temperatura.'],
 umidade_pct:['Irrigue ou nebulize para aumentar a umidade.','Aumente a ventilação para reduzir a umidade.']};
const TXT={ok:'Dentro do ideal',at:'Quase no ideal',aj:'Fora do ideal'};
const COR={ok:'#2e9e5b',at:'#b98300',aj:'#c4393b'};

const mapa={};
PLANTAS.forEach(p=>mapa[norm(p.nome)]=p);
let planta=null,dados=null,off=false;

function avaliar(v,mn,mx,mg){
 if(v>=mn&&v<=mx)return{s:'ok',d:0};
 const d=v<mn?-1:1,df=d<0?mn-v:v-mx;
 return{s:df<=mg?'at':'aj',d:d};
}

function render(){
 let html='',pior=0,aj=[];
 M.forEach(m=>{
  const v=dados?dados[m.v]:null;
  const f=planta?planta[m.k]:null;
  const a=(f&&v!=null)?avaliar(v,f.min,f.max,m.mg):null;
  const pos=x=>Math.max(0,Math.min(100,x/m.mx*100));
  if(a){
   pior=Math.max(pior,a.s==='aj'?2:a.s==='at'?1:0);
   if(a.s!=='ok')aj.push('<li><b>'+m.n+':</b> '+v+' '+m.u+' está '+(a.d<0?'abaixo':'acima')+
    ' do ideal ('+f.min+' a '+f.max+' '+m.u+'). '+DICA[m.k][a.d<0?0:1]+'</li>');
  }
  html+='<div class="card '+(a?a.s:'')+'"><h2>'+m.n+'</h2><div class="v">'+(v==null?'--':v)+
   '<small> '+m.u+'</small></div>';
  if(f){
   html+='<div class="ideal">Ideal: '+f.min+' a '+f.max+' '+m.u+'</div><div class="bar">'+
    '<div class="zona" style="left:'+pos(f.min)+'%;width:'+(pos(f.max)-pos(f.min))+'%"></div>'+
    (v==null?'':'<div class="mk" style="left:'+pos(v)+'%"></div>')+'</div>';
  }else html+='<div class="ideal">Escolha uma planta</div>';
  if(a)html+='<div class="est" style="color:'+COR[a.s]+'">'+TXT[a.s]+'</div>';
  html+='</div>';
 });
 $('cards').innerHTML=html;

 const r=$('res');
 if(off){r.className='';r.textContent='Sem conexão com o ESP32'}
 else if(!planta){r.className='';r.textContent='Escolha uma planta para comparar'}
 else if(!dados){r.className='';r.textContent='Aguardando dados dos sensores'}
 else if(!dados.lido){r.className='';r.textContent='Aguardando leitura do DHT11'}
 else{
  r.className=['ok','at','aj'][pior];
  r.textContent=['Estufa adequada para '+planta.nome,
   'Quase ideal: pequenos ajustes recomendados','A estufa precisa de ajustes'][pior];
 }

 let e='';
 if(planta){
  e='<h3>Por que esses valores para '+planta.nome+'?</h3>';
  M.forEach(m=>{const f=planta[m.k];
   e+='<p><b>'+m.n+' ('+f.min+' a '+f.max+' '+m.u+')</b><br>'+f.motivo+'</p>'});
 }
 $('exp').innerHTML=e;

 let j='';
 if(planta&&dados&&dados.lido&&!off){
  j=aj.length?'<h3>Ajustes sugeridos</h3><ul>'+aj.join('')+'</ul>'
   :'<h3>Ajustes</h3><p>Nenhum ajuste necessário no momento.</p>';
 }
 $('aj').innerHTML=j;
}

function escolher(nome){
 planta=mapa[norm(nome)]||null;
 document.querySelectorAll('.chip').forEach(c=>
  c.classList.toggle('on',!!planta&&c.textContent===planta.nome));
 $('np').textContent=(!planta&&nome.trim())?'Planta não cadastrada. Escolha uma da lista.':'';
 try{localStorage.setItem('planta',planta?planta.nome:'')}catch(e){}
 render();
}

PLANTAS.forEach(p=>{
 const o=document.createElement('option');o.value=p.nome;$('lista').appendChild(o);
 const b=document.createElement('button');b.className='chip';b.textContent=p.nome;
 b.onclick=()=>{$('pl').value=p.nome;escolher(p.nome)};
 $('chips').appendChild(b);
});
$('pl').oninput=e=>escolher(e.target.value);

async function att(){
 try{dados=await (await fetch('/dados')).json();off=false}catch(e){off=true}
 render();
}

try{const s=localStorage.getItem('planta');if(s){$('pl').value=s;escolher(s)}}catch(e){}
render();att();setInterval(att,2000);
</script>
</body>
</html>
)rawliteral";

// ---------- Objetos ----------
WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

// ---------- Variaveis ----------
float temp = 0, hum = 0;
bool lido = false; 
int ldrBruto = 0;
float luz = 0; 

unsigned long tLeitura = 0;

// ---------- LDR ----------
int lerLdrBruto() {
  long soma = 0;
  for (int i = 0; i < 16; i++) {
    soma += analogRead(LDR_PIN);
    delay(2);
  }
  return soma / 16;
}

float luzPercentual(int bruto) {
  float p = (bruto - LDR_ESCURO) * 100.0 / (LDR_CLARO - LDR_ESCURO);
  return constrain(p, 0, 100);
}

// ---------- Rotas ----------
void paginaInicial() {
  server.send_P(200, "text/html; charset=utf-8", PAGINA);
}

void dados() {
  String json = "{";
  json += "\"temperatura\":" + (lido ? String(temp, 1) : String("null")) + ",";
  json += "\"umidade\":" + (lido ? String(hum, 1) : String("null")) + ",";
  json += "\"luz\":" + String(luz, 0) + ",";
  json += "\"ldrBruto\":" + String(ldrBruto) + ",";
  json += "\"lido\":" + String(lido ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// ---------- Setup ----------
void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LOW);

  dht.begin();

  WiFi.softAP(ssid, senha);

  server.on("/", paginaInicial);
  server.on("/dados", dados);
  server.begin();
}

// ---------- Loop ----------
void loop() {
  server.handleClient();

  // Leitura dos sensores a cada 2 s
  if (millis() - tLeitura >= 2000) {
    tLeitura = millis();

    // Pisca o LED Verde para indicar que o circuito esta ativo e lendo
    digitalWrite(LED_STATUS, HIGH);

    // LDR
    ldrBruto = lerLdrBruto();
    luz = luzPercentual(ldrBruto);

    // DHT11
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temp = t;
      hum = h;
      lido = true;
    }

    // Apaga o LED Verde apos a leitura
    digitalWrite(LED_STATUS, LOW);
  }
}
