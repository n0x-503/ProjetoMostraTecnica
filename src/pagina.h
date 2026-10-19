// pagina.h - HTML, CSS e JavaScript do site da estufa
// Fica separado do .ino porque o Arduino IDE insere linhas '#line' dentro de
// textos longos no .ino, o que quebra o JavaScript da pagina.
#pragma once
#include <Arduino.h>

const char PAGINA[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="theme-color" content="#3a6b68">
<title>Estufa Inteligente</title>
<style>
*{box-sizing:border-box}
body{margin:0;padding:22px 16px 40px;background:#3a6b68;color:#201a3a;
font-family:Roboto,system-ui,-apple-system,"Segoe UI",Arial,sans-serif}
.wrap{max-width:520px;margin:0 auto}
.topo{margin:0 0 18px;text-align:center;color:#d6ebe8;font-size:15px;
letter-spacing:2px;text-transform:uppercase}

/* ---------- Cards ---------- */
.card{background:#fff;border-radius:24px;border-top:12px solid #201a3a;
padding:18px 20px 22px;margin-bottom:18px;text-align:center;
box-shadow:0 6px 14px rgba(0,0,0,.25);transition:background .4s}
.card:empty{display:none}
.card h2{margin:0 0 8px;font-size:19px}
.card h3{margin:0 0 14px;font-size:21px}
.val{font-size:38px;font-weight:bold;color:#3a6b68;margin-bottom:6px}
.st{font-size:15px;color:#444}

/* acima do ideal */
.card.ac{background:#ffe8e8;border-top-color:#c62828}
.card.ac .val,.card.ac .st,.card.ac h3{color:#c62828}
.card.ac .st{font-weight:bold}

/* abaixo do ideal */
.card.ab{background:#fff6d6;border:4px solid #f2b705;border-top-width:12px}
.card.ab .val,.card.ab .st{color:#8a6200}
.card.ab .st{font-weight:bold}

/* estufa adequada */
.card.okv{background:#e8f6ec;border-top-color:#2e9e5b}
.card.okv h3{color:#1e6b3e;margin:0}

/* ---------- Seletor ---------- */
.tit{margin:30px 0 14px;text-align:center;color:#fff;font-size:30px}
select{display:block;width:calc(100% - 8px);margin:0 4px 26px;padding:16px 50px 16px 18px;
font-family:inherit;font-size:22px;font-weight:bold;color:#201a3a;
border:5px solid #111;border-radius:26px;box-shadow:0 0 0 4px #fff;
-webkit-appearance:none;appearance:none;
background:#fff url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath d='M3 8l9 9 9-9' fill='none' stroke='%23201a3a' stroke-width='2.5'/%3E%3C/svg%3E") no-repeat right 16px center/24px}

/* ---------- Recomendado ---------- */
.lin{display:flex;justify-content:space-between;align-items:center;
padding:14px 4px;border-bottom:1px solid #ddd;font-size:17px;text-align:left}
.lin:last-child{border-bottom:0}
.lin b{color:#3a6b68;white-space:nowrap;margin-left:10px}
.vz{margin:0;color:#555;font-size:18px;line-height:1.4}

.card ul{margin:0;padding-left:20px;text-align:left;line-height:1.5;font-size:15px}
#exp p{text-align:left;font-size:15px;line-height:1.45;margin:0 0 12px}
#exp p:last-child{margin-bottom:0}

#con{text-align:center;color:#a9cfcb;font-size:17px;margin:10px 0 0}
</style>
</head>
<body>
<div class="wrap">
<p class="topo">Estufa Inteligente</p>

<div id="cards"></div>

<h2 class="tit">Escolha o vegetal:</h2>
<select id="sel"><option value="">Selecione...</option></select>

<div class="card" id="rec"></div>
<div class="card" id="aj"></div>
<div class="card" id="exp"></div>

<p id="con">Aguardando JavaScript...</p>
</div>

<script>
window.onerror=function(m,u,l){var e=document.getElementById('con');
e.style.color='#ffb4b4';e.textContent='Erro JS: '+m+' (linha '+l+')'};
document.getElementById('con').textContent='Script iniciado...';
</script>
<script>
const PLANTAS=[{"id":"rucula","nome":"Rúcula","nome_cientifico":"Eruca vesicaria subsp. sativa","luz_pct":{"min":50,"max":80,"motivo":"Folhosa de ciclo curto: precisa de boa luz para fotossíntese e folhas vigorosas, mas tolera meia-sombra. Luz excessiva junto com calor acelera o florescimento."},"temperatura_c":{"min":15,"max":25,"motivo":"Planta de clima ameno. Acima de cerca de 25 a 28 °C ela emite a haste floral (pendoamento), e as folhas ficam mais amargas e picantes."},"umidade_pct":{"min":50,"max":70,"motivo":"Umidade moderada mantém as folhas firmes sem favorecer fungos. Umidade muito alta aumenta o risco de míldio e podridões."},"solo_pct":{"min":60,"max":80,"motivo":"Tem raízes rasas e cresce rápido, então precisa de solo sempre levemente úmido. A falta de água estressa a planta, deixa as folhas mais amargas e acelera o florescimento; solo encharcado sufoca as raízes."}},{"id":"alface","nome":"Alface","nome_cientifico":"Lactuca sativa","luz_pct":{"min":50,"max":70,"motivo":"Cresce bem com luz moderada. Luz excessiva com calor estressa a planta e estimula o pendoamento; pouca luz deixa as folhas alongadas e fracas (estiolamento)."},"temperatura_c":{"min":15,"max":22,"motivo":"Faixa em que a alface forma folhas macias e doces. Acima de uns 25 °C a planta emite o pendão floral e produz látex, deixando as folhas amargas."},"umidade_pct":{"min":60,"max":80,"motivo":"Umidade mais alta reduz a perda de água das folhas, que são finas e delicadas. Acima de 80% crescem doenças fúngicas como míldio e podridão."},"solo_pct":{"min":65,"max":85,"motivo":"As folhas são quase só água e as raízes são rasas, por isso o solo deve ficar constantemente úmido. Regas irregulares causam folhas amargas e queima nas bordas; encharcamento favorece podridão."}},{"id":"manjericao","nome":"Manjericão","nome_cientifico":"Ocimum basilicum","luz_pct":{"min":70,"max":100,"motivo":"Precisa de sol pleno (6 a 8 horas por dia). A luz intensa aumenta a produção dos óleos essenciais responsáveis pelo aroma e pelo sabor."},"temperatura_c":{"min":20,"max":30,"motivo":"Espécie tropical e sensível ao frio: abaixo de uns 10 °C as folhas escurecem e a planta pode morrer. O crescimento é melhor em clima quente."},"umidade_pct":{"min":40,"max":60,"motivo":"Prefere o ar mais seco que as folhosas. Umidade alta com pouca ventilação favorece fungos como míldio e mofo."},"solo_pct":{"min":40,"max":60,"motivo":"Gosta de solo úmido, mas bem drenado, e tolera secar um pouco entre as regas. Encharcamento causa podridão de raiz e fungos; a falta de água faz a planta murchar rapidamente."}},{"id":"tomate","nome":"Tomate","nome_cientifico":"Solanum lycopersicum","luz_pct":{"min":80,"max":100,"motivo":"Exige muita luz: ela determina a fotossíntese, o número de flores e o acúmulo de açúcares nos frutos. Pouca luz gera plantas alongadas e poucos frutos."},"temperatura_c":{"min":20,"max":28,"motivo":"Entre cerca de 20 e 28 °C a polinização e o pegamento dos frutos são melhores. Acima de uns 32 °C ou abaixo de 13 °C as flores caem e os frutos se formam mal."},"umidade_pct":{"min":60,"max":70,"motivo":"Nessa faixa o pólen é viável e a planta transpira bem. Acima de 80% aumentam doenças como a requeima e o mofo; umidade muito baixa seca o pólen e prejudica a frutificação."},"solo_pct":{"min":55,"max":75,"motivo":"O mais importante é a regularidade. Alternar solo seco e encharcado causa rachadura dos frutos e a podridão apical (falta de cálcio por absorção irregular). Excesso de água também favorece doenças de raiz."}},{"id":"morango","nome":"Morango","nome_cientifico":"Fragaria × ananassa","luz_pct":{"min":70,"max":90,"motivo":"Precisa de bastante luz para formar flores e açúcar nos frutos. Em calor extremo, um leve sombreamento evita queimaduras nas folhas e nos frutos."},"temperatura_c":{"min":15,"max":25,"motivo":"É a faixa de melhor floração e de frutos mais doces. Acima de uns 30 °C a produção cai e os frutos ficam pequenos; frio forte paralisa o crescimento."},"umidade_pct":{"min":60,"max":75,"motivo":"Umidade moderada mantém as folhas saudáveis. O excesso, principalmente com folhas molhadas, favorece o mofo cinzento (Botrytis) nos frutos."},"solo_pct":{"min":60,"max":80,"motivo":"As raízes são superficiais e sensíveis à seca, então o solo deve ficar úmido, mas sempre drenado. Solo encharcado causa podridão de raiz e favorece mofo nos frutos."}}];

const $=i=>document.getElementById(i);
const M=[
 {k:'temperatura_c',n:'Temperatura',u:'°C',v:'temperatura'},
 {k:'umidade_pct',n:'Umidade do ar',u:'%',v:'umidade'},
 {k:'luz_pct',n:'Luminosidade',u:'%',v:'luz'},
 {k:'solo_pct',n:'Umidade do solo',u:'%',v:'solo'}];
const DICA={
 temperatura_c:['Aqueça ou feche a estufa para reter calor.','Ventile ou sombreie a estufa para baixar a temperatura.'],
 umidade_pct:['Nebulize ou feche a estufa para aumentar a umidade do ar.','Aumente a ventilação para reduzir a umidade do ar.'],
 luz_pct:['Aumente a iluminação ou retire o sombreamento.','Use tela de sombreamento ou afaste a planta da luz.'],
 solo_pct:['Regue a planta: o solo está seco.','Reduza a irrigação e confira a drenagem: o solo está encharcado.']};

let planta=null,dados=null,off=false;

function render(){
 let cards='',aj=[];

 // Cards dos sensores
 M.forEach(m=>{
  const v=dados?dados[m.v]:null;
  const f=planta?planta[m.k]:null;
  let cl='',txt='';
  if(f&&v!=null){
   if(v<f.min){cl=' ab';txt='Abaixo do ideal (mín. '+f.min+' '+m.u+')';aj.push([m,0])}
   else if(v>f.max){cl=' ac';txt='Acima do ideal (máx. '+f.max+' '+m.u+')';aj.push([m,1])}
   else txt='Dentro do ideal';
  }
  cards+='<div class="card'+cl+'"><h2>'+m.n+'</h2><div class="val">'+
   (v==null?'--':v+' '+m.u)+'</div>'+(txt?'<div class="st">'+txt+'</div>':'')+'</div>';
 });
 $('cards').innerHTML=cards;

 // Valores recomendados
 let r='';
 if(planta){
  r='<h3>Recomendado para '+planta.nome+'</h3>';
  M.forEach(m=>{const f=planta[m.k];
   r+='<div class="lin"><span>'+m.n+'</span><b>'+f.min+' a '+f.max+' '+m.u+'</b></div>'});
 }else r='<p class="vz">Escolha uma verdura ou legume para ver os valores recomendados.</p>';
 $('rec').innerHTML=r;

 // Resultado geral
 const a=$('aj');
 if(planta&&dados&&dados.lido&&!off){
  if(aj.length){
   a.className='card ac';
   a.innerHTML='<h3>A estufa precisa de ajustes</h3><ul>'+
    aj.map(x=>'<li><b>'+x[0].n+':</b> '+DICA[x[0].k][x[1]]+'</li>').join('')+'</ul>';
  }else{
   a.className='card okv';
   a.innerHTML='<h3>Estufa adequada para '+planta.nome+'</h3>';
  }
 }else{a.className='card';a.innerHTML=''}

 // Explicacao tecnica
 let e='';
 if(planta){
  e='<h3>Por que esses valores?</h3>';
  M.forEach(m=>{e+='<p><b>'+m.n+'</b><br>'+planta[m.k].motivo+'</p>'});
 }
 $('exp').innerHTML=e;

 $('con').textContent=off?'Sem conexão com o ESP32':(dados?'Conectado ao ESP32'+(dados.led&&dados.led!=='off'?' | LED '+dados.led:''):'Conectando ao ESP32...');
}

// Lista de plantas
PLANTAS.forEach(p=>{
 const o=document.createElement('option');
 o.value=p.id;o.textContent=p.nome;
 $('sel').appendChild(o);
});

let ultimaTroca=0;
function enviarPlanta(){
 let q='id='+(planta?planta.id:'');
 if(planta){
  [['t','temperatura_c'],['u','umidade_pct'],['l','luz_pct'],['s','solo_pct']].forEach(x=>{
   q+='&'+x[0]+'min='+planta[x[1]].min+'&'+x[0]+'max='+planta[x[1]].max});
 }
 fetch('/planta?'+q).then(()=>att()).catch(()=>{});
}
$('sel').onchange=()=>{
 planta=PLANTAS.find(p=>p.id===$('sel').value)||null;
 ultimaTroca=Date.now();
 render();
 enviarPlanta();
};

async function att(){
 try{const c=new AbortController();setTimeout(()=>c.abort(),4000);
  dados=await (await fetch('/dados',{signal:c.signal})).json();off=false}catch(e){off=true}
 // A planta escolhida fica guardada no ESP32: sincroniza a tela com ela
 if(dados&&typeof dados.planta==='string'&&Date.now()-ultimaTroca>4000){
  const p=PLANTAS.find(x=>x.id===dados.planta)||null;
  if((p?p.id:'')!==(planta?planta.id:'')){planta=p;$('sel').value=p?p.id:''}
 }
 render();
}

render();att();setInterval(att,2000);
</script>
<p id="fim" style="text-align:center;color:#a9cfcb;font-size:12px">fim da pagina</p>
</body>
</html>
)rawliteral";
