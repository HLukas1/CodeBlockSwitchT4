#pragma once
#include <Arduino.h>

// =====================================================
// GEMEINSAMES STYLESHEET (/app.css)
// =====================================================
const char APP_CSS[] PROGMEM = R"rawliteral(
:root{color-scheme:dark;--bg:#080c12;--surface:#111821;--surface2:#18212d;--line:#232f3e;--line2:#324257;--text:#eef3f9;--muted:#8a99ad;--faint:#5f6e82;--accent:#4fb8ff;--accent-bg:rgba(79,184,255,.12);--green:#3ccf91;--green-bg:rgba(60,207,145,.13);--red:#ff6b78;--red-bg:rgba(255,107,120,.13);--amber:#f5b94f;--amber-bg:rgba(245,185,79,.13);--r:16px}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html{background:var(--bg)}
body{margin:0;min-height:100vh;font-family:ui-sans-serif,system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;color:var(--text);background:var(--bg);line-height:1.45;padding-bottom:calc(84px + env(safe-area-inset-bottom))}
main{max-width:860px;margin:0 auto;padding:0 16px 24px}
header{position:sticky;top:0;z-index:5;background:rgba(8,12,18,.86);backdrop-filter:blur(16px);-webkit-backdrop-filter:blur(16px);border-bottom:1px solid rgba(35,47,62,.6)}
.hd{max-width:860px;margin:0 auto;padding:14px 16px;display:flex;align-items:center;justify-content:space-between;gap:12px}
h1{margin:0;font-size:19px;font-weight:720;letter-spacing:-.02em}
.sub{font-size:12px;color:var(--muted);margin-top:1px}
.pill{display:inline-flex;align-items:center;gap:7px;font-size:12px;font-weight:650;padding:6px 11px;border-radius:999px;background:var(--surface2);color:var(--muted);white-space:nowrap}
.pill::before{content:"";width:7px;height:7px;border-radius:50%;background:currentColor}
.pill.ok{background:var(--green-bg);color:var(--green)}
.pill.err{background:var(--red-bg);color:var(--red)}
.pill.run{background:var(--amber-bg);color:var(--amber)}
h2{font-size:11px;font-weight:700;color:var(--muted);text-transform:uppercase;letter-spacing:.11em;margin:24px 2px 10px}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px;margin-top:16px}
@media(min-width:640px){.grid{grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}}
.card{position:relative;background:var(--surface);border:1px solid var(--line);border-radius:var(--r);padding:16px;min-width:0}
.card.on{border-color:rgba(79,184,255,.55);box-shadow:inset 0 0 0 1px rgba(79,184,255,.25)}
.card.pick{cursor:pointer;transition:border-color .15s,transform .15s}
.card.pick:hover{border-color:var(--line2)}
.card.pick:active{transform:scale(.98)}
.lbl{font-size:12px;font-weight:600;color:var(--muted);overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.card.on .lbl{padding-right:44px}
.val{font-size:clamp(28px,8vw,38px);font-weight:720;letter-spacing:-.03em;font-variant-numeric:tabular-nums;margin-top:6px;line-height:1.1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.val.dim{color:var(--faint)}
.unit{font-size:13px;color:var(--muted);margin-top:2px;min-height:19px}
.badge{position:absolute;top:13px;right:13px;font-size:10px;font-weight:750;letter-spacing:.05em;padding:3px 7px;border-radius:6px;background:var(--accent-bg);color:var(--accent)}
.raw{font-size:11px;color:var(--faint);margin-top:8px;font-variant-numeric:tabular-nums}
.note{background:var(--surface);border:1px solid var(--line);border-radius:12px;padding:12px 14px;color:var(--muted);font-size:14px;margin-top:16px}
.note.warn{background:var(--amber-bg);border-color:rgba(245,185,79,.25);color:var(--amber)}
.hint{font-size:12px;color:var(--faint);margin:8px 2px 0}
button{font:inherit;color:inherit;cursor:pointer;border:0;background:none}
button:disabled{opacity:.45;cursor:wait}
:focus-visible{outline:2px solid var(--accent);outline-offset:2px}
.btn{display:flex;align-items:center;justify-content:center;gap:8px;width:100%;padding:14px;border-radius:12px;font-size:15px;font-weight:680;background:var(--surface2);border:1px solid var(--line)}
.btn.pri{background:var(--accent);border-color:var(--accent);color:#04121f}
.btn.dan{background:var(--red-bg);border-color:rgba(255,107,120,.3);color:var(--red)}
.btns{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:16px}
.row{display:flex;align-items:center;gap:8px;background:var(--surface);border:1px solid var(--line);border-radius:12px;padding:8px 8px 8px 14px;margin-bottom:8px}
.row span{flex:1;min-width:0;font-size:14px;font-weight:550;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.row small{color:var(--faint);font-weight:500}
.ib{flex:none;display:grid;place-items:center;width:36px;height:36px;border-radius:9px;background:var(--surface2);border:1px solid var(--line);font-size:14px}
.ib.del{background:var(--red-bg);border-color:rgba(255,107,120,.25);color:var(--red)}
.ib.add{background:var(--green-bg);border-color:rgba(60,207,145,.25);color:var(--green)}
.step{display:flex;gap:8px;align-items:center;margin-top:16px}
.step input{flex:1;min-width:0;height:50px;text-align:center;font:inherit;font-size:20px;font-weight:700;color:var(--text);background:var(--surface);border:1px solid var(--line);border-radius:12px}
.step .ib{width:50px;height:50px;font-size:22px;border-radius:12px}
.f{background:var(--surface);border:1px solid var(--line);border-left:3px solid var(--red);border-radius:12px;padding:14px 16px;margin-bottom:10px}
.f.sp{border-left-color:var(--amber)}
.top{display:flex;justify-content:space-between;align-items:center;gap:10px}
.code{font-size:21px;font-weight:760;letter-spacing:.02em;font-variant-numeric:tabular-nums}
.obd{font-size:12px;color:var(--accent);margin-left:8px;font-weight:700}
.tag{font-size:11px;font-weight:700;padding:4px 9px;border-radius:999px;background:var(--red-bg);color:var(--red)}
.sp .tag{background:var(--amber-bg);color:var(--amber)}
.ttl{font-size:15px;font-weight:650;margin-top:8px}
.txt{color:#c3cedb;margin-top:6px;font-size:14px}
.sec{margin-top:10px;font-size:13px;color:#c3cedb}
.sec b{display:block;color:var(--muted);font-size:10px;text-transform:uppercase;letter-spacing:.09em;margin-bottom:2px}
.unk{margin-top:8px;font-size:13px;color:var(--amber)}
.empty{text-align:center;color:var(--green);background:var(--green-bg);border-radius:12px;padding:18px;font-weight:600}
.set{background:var(--surface);border:1px solid var(--line);border-radius:var(--r);padding:16px;margin-bottom:10px}
.set-h{display:flex;align-items:center;gap:10px}
.set-h label{flex:1;font-size:15px;font-weight:620}
.info{flex:none;width:26px;height:26px;border-radius:50%;border:1px solid var(--line2);color:var(--accent);font-size:13px;font-weight:750;font-style:italic;font-family:Georgia,serif}
.info[aria-expanded=true]{background:var(--accent-bg);border-color:var(--accent)}
.desc{display:none;font-size:13px;color:#b9c5d3;background:var(--surface2);border-radius:10px;padding:10px 12px;margin-top:10px}
.desc.open{display:block}
.desc b{color:var(--text)}
.ctl{display:flex;align-items:center;gap:8px;margin-top:12px}
.ctl input{flex:1;min-width:0;height:46px;text-align:center;font:inherit;font-size:18px;font-weight:700;color:var(--text);background:var(--bg);border:1px solid var(--line);border-radius:10px}
.ctl .ib{width:46px;height:46px;font-size:20px;border-radius:10px}
.ctl .u{width:22px;color:var(--muted);font-size:14px}
.meta{font-size:12px;color:var(--faint);margin-top:8px}
.meta b{color:var(--muted);font-weight:600}
.cbcard{display:flex;align-items:center;gap:14px;grid-column:1/-1}
.cbcard>div{flex:1;min-width:0}
.cbcard .lbl{padding:0;display:flex;align-items:center;gap:7px}
.cbcard .badge{position:static}
.cbcard .btn{flex:none;width:auto;padding:12px 14px;font-size:14px}
.modal{position:fixed;inset:0;z-index:20;display:grid;place-items:center;padding:24px;background:rgba(0,0,0,.72)}
.modal[hidden]{display:none}
.dlg{width:100%;max-width:360px;background:var(--surface2);border:1px solid var(--line2);border-radius:var(--r);padding:22px;text-align:center}
.dlg h3{margin:0;font-size:19px;font-weight:720;letter-spacing:-.01em}
.dlg p{margin:10px 0 18px;font-size:14px;color:#c3cedb}
.grp{margin-top:18px}
.grp-h{display:flex;justify-content:space-between;align-items:center;gap:10px;margin:0 2px 8px}
.grp-h b{font-size:15px}
.kv{display:flex;justify-content:space-between;gap:12px;padding:10px 0;border-top:1px solid var(--line);font-size:14px}
.kv:first-child{border-top:0}
.kv b{font-weight:650;white-space:nowrap}
.kv b.dim{color:var(--faint);font-weight:500}
.ecu{display:block;width:100%;height:48px;margin-top:16px;padding:0 14px;font:inherit;font-size:15px;font-weight:620;color:var(--text);background:var(--surface);border:1px solid var(--line);border-radius:12px}
nav.tabs{position:fixed;left:0;right:0;bottom:0;z-index:10;background:rgba(12,17,24,.94);backdrop-filter:blur(16px);-webkit-backdrop-filter:blur(16px);border-top:1px solid var(--line);padding-bottom:env(safe-area-inset-bottom)}
nav.tabs div{max-width:560px;margin:0 auto;display:grid;grid-template-columns:repeat(5,1fr)}
nav.tabs a{display:flex;flex-direction:column;align-items:center;gap:3px;padding:9px 2px 10px;color:var(--faint);text-decoration:none;font-size:10.5px;font-weight:620}
nav.tabs a.act{color:var(--accent)}
nav.tabs svg{width:22px;height:22px;fill:none;stroke:currentColor;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round}
)rawliteral";

// App-Infos für "Zum Startbildschirm hinzufügen" (/manifest.json)
const char APP_MANIFEST[] PROGMEM = R"rawliteral({"name":"CodeBlockSwitchT4","short_name":"T4","start_url":"/","scope":"/","display":"standalone","background_color":"#080c12","theme_color":"#080c12","lang":"de","icons":[{"src":"/icon.png","sizes":"192x192","type":"image/png","purpose":"any maskable"}]})rawliteral";

// Untere Navigation, in jede Seite eingebaut (aktiver Tab per JS)
#define NAV_HTML R"rawliteral(
<nav class="tabs" aria-label="Navigation"><div>
<a href="/" data-p="/"><svg viewBox="0 0 24 24"><path d="M4 14a8 8 0 0 1 16 0"/><path d="M12 14l4-4"/><path d="M4 18h16"/></svg>Live</a>
<a href="/mwb" data-p="/mwb"><svg viewBox="0 0 24 24"><rect x="4" y="4" width="7" height="7" rx="1.5"/><rect x="13" y="4" width="7" height="7" rx="1.5"/><rect x="4" y="13" width="7" height="7" rx="1.5"/><rect x="13" y="13" width="7" height="7" rx="1.5"/></svg>Messwerte</a>
<a href="/faults" data-p="/faults"><svg viewBox="0 0 24 24"><path d="M12 4l9 16H3z"/><path d="M12 10v4"/><path d="M12 17v.5"/></svg>Fehler</a>
<a href="/oled" data-p="/oled"><svg viewBox="0 0 24 24"><rect x="5" y="3" width="14" height="18" rx="2.5"/><rect x="8" y="6" width="8" height="6" rx="1"/><circle cx="12" cy="16.5" r="1.5"/></svg>OLED</a>
<a href="/settings" data-p="/settings"><svg viewBox="0 0 24 24"><path d="M4 7h10M18 7h2M4 17h4M12 17h8"/><circle cx="16" cy="7" r="2"/><circle cx="10" cy="17" r="2"/></svg>Einstellungen</a>
</div></nav>
<script>document.querySelectorAll('nav.tabs a').forEach(a=>{if(a.dataset.p===location.pathname)a.className='act'})</script>
)rawliteral"

// Steuergeräte-Auswahl für den Fehlerspeicher (laut Autoscan,
// alle über K-Line mit KW1281). Adresse in Hex wie in VCDS.
#define ECU_JS R"rawliteral(
<script>
const ECUS=[['01','Motorelektronik'],['03','Bremse'],['08','Climatronic'],['15','Airbag'],['17','Kombiinstrument'],['19','Gateway']];
let ecu='01';
try{const s=localStorage.getItem('ecu');if(ECUS.some(e=>e[0]===s))ecu=s;}catch(e){}
function ecuName(a){const e=ECUS.find(x=>x[0]===a);return e?e[1]:'Adresse '+a;}
function ecuSelect(onChange){
  const sel=document.getElementById('ecu');
  ECUS.forEach(([a,n])=>{const o=document.createElement('option');o.value=a;o.textContent=a+' – '+n;sel.appendChild(o);});
  sel.value=ecu;
  sel.onchange=()=>{ecu=sel.value;try{localStorage.setItem('ecu',ecu);}catch(e){}onChange();};
}
</script>
)rawliteral"

#define HEAD_HTML(title) R"rawliteral(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#080c12">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black">
<meta name="apple-mobile-web-app-title" content="T4">
<link rel="icon" type="image/png" sizes="192x192" href="/icon.png">
<link rel="apple-touch-icon" href="/icon.png">
<link rel="manifest" href="/manifest.json">
<link rel="stylesheet" href="/app.css?v=8">
<title>)rawliteral" title R"rawliteral(</title>
</head>
)rawliteral"

// =====================================================
// DASHBOARD
// =====================================================
const char INDEX_HTML[] PROGMEM = HEAD_HTML("T4 Live") R"rawliteral(
<body>
<header><div class="hd"><div><h1>Live</h1><div class="sub">Werte aus der OLED-Auswahl</div></div><span class="pill" id="st">…</span></div></header>
<main>
<div class="note warn" id="diag" hidden>Diagnose ist aus – die K-Line ist frei. Zum Einschalten am Gerät im Spannungsbildschirm lange drücken.</div>
<div class="grid" id="grid"></div>
<div class="note warn" id="cbErr" hidden></div>
<p class="hint">Welche Werte hier erscheinen, legst du unter <b>OLED</b> fest. <span style="color:var(--accent)">OLED</span> markiert den Wert, der gerade am Gerät angezeigt wird.</p>
</main>
<div class="modal" id="cycle" role="alertdialog" aria-modal="true" aria-labelledby="cycleT" hidden>
  <div class="dlg">
    <h3 id="cycleT">Bitte Zündungswechsel durchführen</h3>
    <p id="cycleP"></p>
    <button class="btn pri" id="cycleOk">Verstanden</button>
  </div>
</div>
)rawliteral" NAV_HTML R"rawliteral(
<script>
const st=document.getElementById('st'),grid=document.getElementById('grid'),diag=document.getElementById('diag');
const $=id=>document.getElementById(id);
const PS={2:'102 PS',3:'138 PS'};
let sig='',coding=0,cbWait=false,cbPosted=0,ackDone=0;

const cbClick=async()=>{
  const to=coding===2?3:2;
  if(!confirm('Auf '+PS[to]+' umschalten?'))return;
  cbWait=true;cbPosted=Infinity;
  $('cbErr').hidden=true;
  $('cbBtn').disabled=true;$('cbBtn').textContent='Schreibe …';
  try{
    const r=await fetch('/api/codeblock?to='+to,{method:'POST'});
    const d=await r.json();
    if(!d.ok){cbWait=false;$('cbErr').hidden=false;$('cbErr').textContent=d.msg;}
  }catch(e){cbWait=false;$('cbErr').hidden=false;$('cbErr').textContent='ESP nicht erreichbar';}
  cbPosted=Date.now();
};

$('cycleOk').onclick=()=>{
  ackDone=Infinity;$('cycle').hidden=true;
  fetch('/api/codeblock/ack',{method:'POST'}).catch(()=>{}).finally(()=>{ackDone=Date.now();});
};

function showCb(d,t0){
  // Erst Antworten auswerten, die nach dem Schreibauftrag angefragt wurden
  if(cbWait&&t0>=cbPosted)cbWait=false;
  coding=d.coding;
  const known=coding===2||coding===3;
  const shown=!!$('cb');
  if(shown){
    const v=$('cbVal');
    v.textContent=known?PS[coding]:(coding?coding:'-');
    v.className='val'+(coding?'':' dim');
    $('cbPs').hidden=known;
    $('cbPs').textContent=coding?'Unbekannte Codierung':'Noch nicht gelesen';
    $('cb').classList.toggle('on',d.cb);
    $('cbBadge').hidden=!d.cb;
    const b=$('cbBtn');
    b.hidden=!known||!d.diag;
    b.disabled=cbWait;
    if(cbWait)b.textContent='Schreibe …';
    else if(known)b.textContent=PS[coding===2?3:2];
  }
  if(!cbWait){$('cbErr').hidden=!shown||!d.cbErr;$('cbErr').textContent=d.cbErr;}
  if(t0>=ackDone){
    $('cycle').hidden=!d.cbCycle;
    if(d.cbCycle)$('cycleP').textContent=(known?PS[coding]:'Codeblock '+coding)+' wurde geschrieben. Zündung aus- und wieder einschalten, damit die Änderung wirksam wird.';
  }
}

function build(items){
  grid.innerHTML='';
  items.forEach(it=>{
    const c=document.createElement('div');
    if(it.id===6){
      c.className='card cbcard';c.id='cb';
      c.innerHTML='<div><div class="lbl">Codeblock<span class="badge" id="cbBadge" hidden>OLED</span></div><div class="val dim" id="cbVal">-</div><div class="unit" id="cbPs">&nbsp;</div></div><button class="btn pri" id="cbBtn" hidden></button>';
      c.querySelector('#cbBtn').onclick=cbClick;
    }else{
      c.className='card';c.id='c'+it.id;
      c.innerHTML='<div class="lbl"></div><div class="val">-</div><div class="unit"></div><span class="badge" hidden>OLED</span>';
      c.querySelector('.lbl').textContent=it.id>=10?'★ '+it.n:it.n;
    }
    grid.appendChild(c);
  });
}

async function upd(){
  try{
    const t0=Date.now();
    const d=await (await fetch('/api/live',{cache:'no-store'})).json();
    const s=d.items.map(i=>i.id+i.n).join('|');
    if(s!==sig){sig=s;build(d.items);}
    showCb(d,t0);

    d.items.forEach(it=>{
      if(it.id===6)return;
      const c=document.getElementById('c'+it.id);
      const ok=!it.k||d.connected;
      const v=c.querySelector('.val');
      v.textContent=ok?it.v:'-';
      v.className='val'+(ok&&it.v!=='-'?'':' dim');
      c.querySelector('.unit').textContent=it.u||' ';
      const on=!d.cb&&it.id===d.cur;
      c.classList.toggle('on',on);
      c.querySelector('.badge').hidden=!on;
    });

    diag.hidden=d.diag;
    if(!d.diag){st.textContent='Diagnose aus';st.className='pill run';}
    else{st.textContent=d.connected?'MSG verbunden':'MSG getrennt';st.className='pill '+(d.connected?'ok':'err');}
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
  setTimeout(upd,500);
}
upd();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// MESSWERTEBLÖCKE
// =====================================================
const char MWB_HTML[] PROGMEM = HEAD_HTML("Messwerteblöcke") R"rawliteral(
<body>
<header><div class="hd"><div><h1>Messwerteblöcke</h1><div class="sub">Karte antippen = als ★ aufs OLED</div></div><span class="pill" id="st">…</span></div></header>
<main>
<div class="step">
  <button class="ib" id="prev" aria-label="Vorheriger Block">&minus;</button>
  <input id="grp" type="number" min="1" max="255" inputmode="numeric" aria-label="Blocknummer">
  <button class="ib" id="next" aria-label="Nächster Block">+</button>
</div>
<div class="note" id="msg"></div>
<div class="grid" id="grid"></div>
</main>
)rawliteral" NAV_HTML R"rawliteral(
<script>
const st=document.getElementById('st'),grp=document.getElementById('grp'),grid=document.getElementById('grid'),msg=document.getElementById('msg');
let g=1;
try{g=parseInt(localStorage.getItem('mwbGroup'))||1;}catch(e){}
grp.value=g;

function setG(v){
  v=Math.max(1,Math.min(255,parseInt(v)||1));
  g=v;grp.value=v;
  try{localStorage.setItem('mwbGroup',v);}catch(e){}
  grid.innerHTML='';
  msg.textContent='Lade Block '+v+' …';
}
document.getElementById('prev').onclick=()=>setG(g-1);
document.getElementById('next').onclick=()=>setG(g+1);
grp.onchange=()=>setG(grp.value);

function card(i,f){
  const c=document.createElement('div');
  c.className='card pick';
  const mk=(cls,txt)=>{const e=document.createElement('div');e.className=cls;e.textContent=txt;c.appendChild(e);};
  const named=f.n&&f.n!=='-';
  mk('lbl',named?f.n:'Feld '+(i+1));
  mk('val',f.v);
  mk('unit',f.u||' ');
  mk('raw','Feld '+(i+1)+' · K='+f.k+' a='+f.a+' b='+f.b);
  c.onclick=()=>pick(i,f);
  return c;
}

async function pick(i,f){
  const title=f.n&&f.n!=='-'?f.n:'MWB '+g+' Feld '+(i+1);
  const unit=(f.u||'').substring(0,9);
  if(!confirm('„'+title+'“ als eigenen Messwert aufs OLED legen?'))return;
  try{
    const d=await (await fetch('/api/oled/custom?g='+g+'&f='+(i+1)+'&title='+encodeURIComponent(title)+'&unit='+encodeURIComponent(unit),{method:'POST'})).json();
    alert(d.ok?'★ '+title+' wurde zum OLED hinzugefügt.':d.msg);
  }catch(e){alert('ESP nicht erreichbar');}
}

async function upd(){
  try{
    const d=await (await fetch('/api/mwb?group='+g,{cache:'no-store'})).json();
    st.textContent=d.connected?'MSG verbunden':'MSG getrennt';
    st.className='pill '+(d.connected?'ok':'err');
    if(d.group===g&&d.dataGroup===g&&d.fields.length>0){
      grid.innerHTML='';
      d.fields.forEach((f,i)=>grid.appendChild(card(i,f)));
      msg.textContent='Block '+g+(d.title?' – '+d.title:'')+(d.cond?' ('+d.cond+')':'');
    }else if(d.error&&d.group===g){
      msg.textContent='Block '+g+' konnte nicht gelesen werden.';
    }else if(!d.connected){
      msg.textContent='Warte auf Verbindung zum MSG …';
    }
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
  setTimeout(upd,500);
}
msg.textContent='Lade Block '+g+' …';
upd();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// FEHLERSPEICHER
// =====================================================
const char FAULT_HTML[] PROGMEM = HEAD_HTML("Fehlerspeicher") R"rawliteral(
<body>
<header><div class="hd"><div><h1>Fehlerspeicher</h1><div class="sub" id="sub">Motorelektronik (Adresse 01)</div></div><span class="pill" id="st">…</span></div></header>
<main>
<select id="ecu" class="ecu" aria-label="Steuergerät"></select>
<div class="btns">
  <button class="btn pri" id="rb">Fehler lesen</button>
  <button class="btn dan" id="cb">Fehler löschen</button>
</div>
<div class="note" id="msg">Noch nicht gelesen</div>
<button class="btn" id="dl" style="margin-top:10px" hidden>Bericht herunterladen (HTML)</button>
<h2>Einträge</h2>
<div id="list"></div>
</main>
)rawliteral" NAV_HTML ECU_JS R"rawliteral(
<script>
const st=document.getElementById('st'),msg=document.getElementById('msg'),list=document.getElementById('list'),rb=document.getElementById('rb'),cb=document.getElementById('cb');
const sub=document.getElementById('sub'),dl=document.getElementById('dl');
const ALL='ALL';
const sleep=ms=>new Promise(r=>setTimeout(r,ms));
let scanning=false,results=[],scanTime=null;
// Letztes Ergebnis bei einem einzelnen Steuergerät (für den Bericht)
let single=null,singleTime=null,wasRunning=false;

ecuSelect(()=>{
  list.innerHTML='';
  if(ecu===ALL)renderAll();
  else{msg.textContent='Noch nicht gelesen';dl.hidden=true;}
});
// Zusätzlicher Eintrag: alle Steuergeräte nacheinander auslesen
(function(){
  const sel=document.getElementById('ecu');
  const o=document.createElement('option');o.value=ALL;o.textContent='Alle Steuergeräte';sel.appendChild(o);
  try{if(localStorage.getItem('ecu')===ALL){ecu=ALL;sel.value=ALL;}}catch(e){}
})();

function btn(dis){rb.disabled=dis;cb.disabled=dis||ecu===ALL;}

async function cmd(a){
  if(ecu===ALL){if(a==='read')scanAll();return;}
  if(a==='clear'&&!confirm('Fehlerspeicher von '+ecuName(ecu)+' wirklich löschen?'))return;
  btn(true);
  msg.textContent=a==='read'?'Lese Fehlerspeicher …':'Lösche Fehlerspeicher …';
  try{await fetch('/api/faults/'+a+'?addr='+ecu,{method:'POST',cache:'no-store'});}
  catch(e){msg.textContent='Anforderung fehlgeschlagen';btn(false);}
}
rb.onclick=()=>cmd('read');
cb.onclick=()=>cmd('clear');

function card(f){
  const c=document.createElement('div');
  c.className='f'+(f.sporadic?' sp':'');
  const top=document.createElement('div');top.className='top';
  const left=document.createElement('div');
  const code=document.createElement('span');code.className='code';code.textContent=f.code;left.appendChild(code);
  if(f.known){const o=document.createElement('span');o.className='obd';o.textContent=f.obd;left.appendChild(o);}
  const tag=document.createElement('div');tag.className='tag';tag.textContent=f.sporadic?'sporadisch':'statisch';
  top.appendChild(left);top.appendChild(tag);c.appendChild(top);
  const add=(cls,txt)=>{const e=document.createElement('div');e.className=cls;e.textContent=txt;c.appendChild(e);};
  const sec=(label,txt)=>{const e=document.createElement('div');e.className='sec';const b=document.createElement('b');b.textContent=label;e.appendChild(b);e.appendChild(document.createTextNode(txt));c.appendChild(e);};
  if(f.known){add('ttl',f.title);if(f.text)add('txt',f.text);sec('Ursache',f.cause);sec('Auswirkung',f.effect);}
  else{if(f.text)add('txt',f.text);add('unk','Fehlercode nicht in der Tabelle hinterlegt');}
  add('raw','Fehlerort 0x'+f.hex+' · Fehlerart 0x'+f.typeHex);
  return c;
}

// ---------- Alle Steuergeräte ----------

// Ein Steuergerät lesen und auf das Ergebnis warten
async function readOne(a){
  for(let t=0;t<20;t++){
    try{const r=await fetch('/api/faults/read?addr='+a,{method:'POST',cache:'no-store'});if(r.status!==409)break;}catch(e){}
    await sleep(700);
  }
  for(let i=0;i<150;i++){
    await sleep(600);
    try{
      const d=await (await fetch('/api/faults?addr='+a,{cache:'no-store'})).json();
      if(!d.running&&d.addr===a)return d;
    }catch(e){}
  }
  return {addr:a,valid:false,error:true,status:'Keine Antwort (Zeitüberschreitung)',faults:[],ident:''};
}

function summary(items){
  const total=items.reduce((s,r)=>s+(r.d.valid?r.d.faults.length:0),0);
  const bad=items.filter(r=>!r.d.valid).length;
  if(items.length===1)return bad?'nicht erreichbar':total+' Fehler';
  return total+' Fehler in '+items.length+' Steuergeräten'+(bad?', '+bad+' nicht erreichbar':'');
}

function renderAll(){
  list.innerHTML='';
  sub.textContent='Alle Steuergeräte ('+ECUS.length+')';
  if(!results.length){
    if(!scanning)msg.textContent='Liest alle '+ECUS.length+' Steuergeräte nacheinander. Das erste Mal kann es einige Minuten dauern.';
    dl.hidden=true;return;
  }
  results.forEach(r=>{
    const g=document.createElement('div');g.className='grp';
    const h=document.createElement('div');h.className='grp-h';
    const b=document.createElement('b');b.textContent=r.a+' – '+r.n;h.appendChild(b);
    const p=document.createElement('span');
    const n=r.d.valid?r.d.faults.length:-1;
    p.className='pill '+(n<0?'err':(n?'run':'ok'));
    p.textContent=n<0?'nicht erreichbar':(n?n+' Fehler':'kein Fehler');
    h.appendChild(p);g.appendChild(h);
    if(r.d.ident){const i=document.createElement('div');i.className='meta';i.style.margin='-4px 2px 8px';i.textContent=r.d.ident;g.appendChild(i);}
    if(n<0){const e=document.createElement('div');e.className='note warn';e.style.marginTop='0';e.textContent=r.d.status;g.appendChild(e);}
    else r.d.faults.forEach(f=>g.appendChild(card(f)));
    list.appendChild(g);
  });
  if(!scanning){msg.textContent='Fertig: '+summary(results);dl.hidden=false;}
}

async function scanAll(){
  scanning=true;results=[];scanTime=null;btn(true);dl.hidden=true;list.innerHTML='';
  st.textContent='läuft …';st.className='pill run';
  for(let i=0;i<ECUS.length;i++){
    const [a,n]=ECUS[i];
    msg.textContent='Lese '+a+' – '+n+' ('+(i+1)+'/'+ECUS.length+') …';
    const d=await readOne(a);
    results.push({a,n,d});
    renderAll();
    msg.textContent=i+1<ECUS.length?'Lese weiter …':msg.textContent;
  }
  scanning=false;scanTime=new Date();
  renderAll();
  st.textContent='Fertig';st.className='pill ok';
  btn(false);
}

// ---------- HTML-Bericht ----------

function esc(s){return String(s==null?'':s).replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));}

// Bericht als eigenständige HTML-Datei (hell, druckbar)
// items: [{a,n,d}], all: Gesamtauszug oder einzelnes Steuergerät
function report(items,t,all,icon){
  const z=n=>String(n).padStart(2,'0');
  const day=t.toLocaleDateString('de-DE',{weekday:'long',day:'numeric',month:'long',year:'numeric'});
  const time=z(t.getHours())+':'+z(t.getMinutes())+':'+z(t.getSeconds());
  const iso=t.getFullYear()+'-'+z(t.getMonth()+1)+'-'+z(t.getDate())+' '+time;
  const title=all?'Gesamtauszug':items[0].a+' – '+items[0].n;
  const fw=(items.find(r=>r.d.fw)||{d:{}}).d.fw||'';
  const total=items.reduce((s,r)=>s+(r.d.valid?r.d.faults.length:0),0);
  const bad=items.filter(r=>!r.d.valid).length;
  const clean=items.filter(r=>r.d.valid&&!r.d.faults.length).length;

  const badge=r=>!r.d.valid?'<span class="b na">nicht erreichbar</span>'
    :(r.d.faults.length?`<span class="b err">${r.d.faults.length} Fehler</span>`:'<span class="b ok">kein Fehler</span>');

  const fault=f=>`<div class="f${f.sporadic?' sp':''}">
<div class="fh"><span class="code">${esc(f.code)}</span>${f.known&&f.obd?`<span class="obd">${esc(f.obd)}</span>`:''}<span class="b ${f.sporadic?'warn':'err'}">${f.sporadic?'sporadisch':'statisch'}</span></div>
<div class="ft">${esc(f.known?f.title:'Fehlercode nicht in der Tabelle hinterlegt')}</div>
<div class="fa">${f.text?esc(f.text)+' · ':''}Fehlerart ${parseInt(f.typeHex,16)&127} (0x${esc(f.typeHex)}) · Fehlerort 0x${esc(f.hex)}</div>
${f.known?`<div class="ce"><div><span>Ursache</span>${esc(f.cause)}</div><div><span>Auswirkung</span>${esc(f.effect)}</div></div>`:''}
</div>`;

  const ecu=r=>`<section class="ecu">
<div class="eh"><span class="adr">${esc(r.a)}</span><div class="en"><b>${esc(r.n)}</b>${r.d.ident?`<code>${esc(r.d.ident)}</code>`:''}</div>${badge(r)}</div>
${!r.d.valid?`<p class="msg">${esc(r.d.status||'Keine Verbindung')}</p>`
  :(r.d.faults.length?r.d.faults.map(fault).join(''):'<p class="msg ok">Im Fehlerspeicher ist kein Fehler eingetragen.</p>')}
</section>`;

  const overview=all?`<table><thead><tr><th>Adresse</th><th>Steuergerät</th><th>Ergebnis</th></tr></thead><tbody>
${items.map(r=>`<tr><td><span class="adr s">${esc(r.a)}</span></td><td>${esc(r.n)}</td><td>${badge(r)}</td></tr>`).join('')}
</tbody></table>`:'';

  const css=`*{box-sizing:border-box}
body{margin:0;background:#f3f5f8;color:#1b2430;font:15px/1.5 -apple-system,"Segoe UI",Roboto,Helvetica,Arial,sans-serif}
.page{max-width:860px;margin:0 auto;padding:28px 18px 40px}
.top{display:flex;justify-content:space-between;align-items:flex-start;gap:18px;flex-wrap:wrap;background:#0d1520;color:#fff;border-radius:16px;padding:22px 24px}
.brand{display:flex;align-items:center;gap:14px}.brand img{width:52px;height:52px;border-radius:12px}
.app{font-size:12px;letter-spacing:.12em;text-transform:uppercase;color:#8fb8dc}
h1{margin:2px 0 0;font-size:24px;letter-spacing:-.01em}
.stamp{text-align:right;margin-left:auto}.stamp span{display:block;font-size:12px;letter-spacing:.08em;text-transform:uppercase;color:#8fb8dc}
.stamp b{display:block;font-size:16px}.stamp .tm{font-size:22px;font-variant-numeric:tabular-nums}
.tiles{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;margin:16px 0}
.tile{background:#fff;border:1px solid #dfe4ea;border-radius:12px;padding:12px 14px}
.tile span{display:block;font-size:12px;color:#5b6676}.tile b{font-size:24px;font-variant-numeric:tabular-nums}
.tile.err b{color:#c62828}.tile.ok b{color:#1a7f37}.tile.na b{color:#8a5a00}
table{width:100%;border-collapse:collapse;background:#fff;border:1px solid #dfe4ea;border-radius:12px;overflow:hidden;margin-bottom:16px}
th,td{text-align:left;padding:9px 14px;border-bottom:1px solid #edf0f3;font-size:14px}th{font-size:12px;color:#5b6676;text-transform:uppercase;letter-spacing:.06em;background:#f8f9fb}
tr:last-child td{border-bottom:0}
.ecu{background:#fff;border:1px solid #dfe4ea;border-radius:14px;padding:16px 18px;margin-bottom:12px;break-inside:avoid}
.eh{display:flex;align-items:center;gap:12px}.en{flex:1;min-width:0}.en b{display:block;font-size:17px}
.en code{font:12px ui-monospace,Menlo,Consolas,monospace;color:#5b6676}
.adr{display:inline-grid;place-items:center;min-width:42px;height:42px;border-radius:10px;background:#e8f1fb;color:#0b5cad;font:700 16px ui-monospace,Menlo,Consolas,monospace}
.adr.s{min-width:34px;height:28px;font-size:13px;border-radius:7px}
.b{display:inline-block;white-space:nowrap;font-size:12px;font-weight:700;padding:3px 10px;border-radius:999px}
.b.ok{background:#e6f4ea;color:#1a7f37}.b.err{background:#fdecec;color:#c62828}.b.warn{background:#fff4dc;color:#8a5a00}.b.na{background:#eef0f3;color:#5b6676}
.msg{margin:12px 0 0;color:#5b6676}.msg.ok{color:#1a7f37}
.f{margin-top:12px;border:1px solid #f1d3d3;border-left:4px solid #c62828;border-radius:10px;padding:12px 14px;background:#fffafa;break-inside:avoid}
.f.sp{border-color:#f0e0bb;border-left-color:#d49100;background:#fffcf4}
.fh{display:flex;align-items:center;gap:8px}.code{font:700 20px ui-monospace,Menlo,Consolas,monospace}
.obd{font:600 13px ui-monospace,Menlo,Consolas,monospace;color:#0b5cad}.fh .b{margin-left:auto}
.ft{font-weight:650;margin-top:4px}.fa{font-size:13px;color:#5b6676}
.ce{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:10px}.ce div{font-size:14px}
.ce span{display:block;font-size:11px;font-weight:700;letter-spacing:.08em;text-transform:uppercase;color:#5b6676}
footer{margin-top:22px;font-size:12px;color:#7a8594;text-align:center}
@media(max-width:600px){.tiles{grid-template-columns:repeat(2,1fr)}.ce{grid-template-columns:1fr}.stamp{text-align:left;margin-left:0}}
@media print{body{background:#fff}.page{padding:0}.top{-webkit-print-color-adjust:exact;print-color-adjust:exact}}`;

  return `<!DOCTYPE html><html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fehlerspeicher – ${esc(title)} – ${esc(iso)}</title><style>${css}</style></head><body><div class="page">
<div class="top"><div class="brand">${icon?`<img src="${icon}" alt="">`:''}<div><div class="app">CodeBlockSwitchT4 · Fehlerspeicher</div><h1>${esc(title)}</h1></div></div>
<div class="stamp"><span>Ausgelesen am</span><b>${esc(day)}</b><b class="tm">${time} Uhr</b></div></div>
<div class="tiles"><div class="tile"><span>Steuergeräte</span><b>${items.length}</b></div><div class="tile ${total?'err':'ok'}"><span>Fehler</span><b>${total}</b></div><div class="tile ok"><span>ohne Fehler</span><b>${clean}</b></div><div class="tile ${bad?'na':''}"><span>nicht erreichbar</span><b>${bad}</b></div></div>
${overview}
${items.map(ecu).join('')}
<footer>Erstellt mit CodeBlockSwitchT4${fw?' · Firmware '+esc(fw):''} · Zeitstempel ${esc(iso)}</footer>
</div></body></html>`;
}

// App-Icon als Data-URL für den Bericht (fehlt es, ohne Icon)
async function iconData(){
  try{
    const b=await (await fetch('/icon.png')).blob();
    return await new Promise(res=>{const r=new FileReader();r.onload=()=>res(r.result);r.onerror=()=>res('');r.readAsDataURL(b);});
  }catch(e){return '';}
}

dl.onclick=async()=>{
  const all=ecu===ALL;
  if(!all&&!single)return;
  const items=all?results:[single];
  const t=(all?scanTime:singleTime)||new Date(),z=n=>String(n).padStart(2,'0');
  // Dateiname: Gesamtauszug oder Adresse + Steuergerät
  const what=all?'Gesamtauszug':single.a+'_'+single.n.replace(/[^A-Za-z0-9ÄÖÜäöüß-]+/g,'_').replace(/_+$/,'');
  const name='Fehlerspeicher_'+what+'_'+t.getFullYear()+'-'+z(t.getMonth()+1)+'-'+z(t.getDate())+'_'+z(t.getHours())+'-'+z(t.getMinutes())+'.html';
  const html=report(items,t,all,await iconData());
  const u=URL.createObjectURL(new Blob([html],{type:'text/html'}));
  const a=document.createElement('a');a.href=u;a.download=name;document.body.appendChild(a);a.click();a.remove();
  setTimeout(()=>URL.revokeObjectURL(u),10000);
};

// ---------- Einzelnes Steuergerät ----------

async function upd(){
  if(ecu===ALL){
    if(!scanning&&!results.length){st.textContent='Bereit';st.className='pill';btn(false);renderAll();}
    setTimeout(upd,700);return;
  }
  try{
    const a=ecu;
    const d=await (await fetch('/api/faults?addr='+a,{cache:'no-store'})).json();
    if(a!==ecu){setTimeout(upd,50);return;}
    // Ergebnis gehört zum gewählten Steuergerät?
    const mine=d.addr===a;
    sub.textContent=ecuName(a)+' (Adresse '+a+')'+(mine&&d.ident?' · '+d.ident:'');
    msg.textContent=(mine||d.running)?d.status:'Noch nicht gelesen';
    if(d.running){st.textContent='läuft …';st.className='pill run';btn(true);}
    else{
      const err=mine&&d.error,ok=mine&&d.valid;
      st.textContent=err?'Fehler':(ok?'Bereit':'Nicht gelesen');
      st.className='pill'+(err?' err':(ok?' ok':''));
      btn(false);
    }
    // Bericht für dieses Steuergerät anbieten, sobald ein Ergebnis da ist
    if(mine&&!d.running&&(d.valid||d.error)){
      if(wasRunning||!single||single.a!==a)singleTime=new Date();
      single={a,n:ecuName(a),d};
      dl.hidden=false;
    }else dl.hidden=true;
    wasRunning=d.running;
    list.innerHTML='';
    if(!mine){}
    else if(d.valid&&d.faults.length===0){
      const e=document.createElement('div');e.className='empty';e.textContent='Kein Fehler erkannt';list.appendChild(e);
    }else{d.faults.forEach(f=>list.appendChild(card(f)));}
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
  setTimeout(upd,700);
}
upd();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// OLED-KONFIGURATION
// =====================================================
const char OLED_HTML[] PROGMEM = HEAD_HTML("OLED-Anzeige") R"rawliteral(
<body>
<header><div class="hd"><div><h1>OLED-Anzeige</h1><div class="sub">Reihenfolge, Einheiten, eigene Werte</div></div><span class="pill" id="st">…</span></div></header>
<main>
<h2>Reihenfolge (Taster)</h2>
<div id="act"></div>
<p class="hint">Erster Eintrag = Startbildschirm. Diese Werte zeigt auch die Live-Seite.</p>

<h2>Nicht angezeigt</h2>
<div id="off"></div>

<h2>Eigene Messwerte</h2>
<div id="cus"></div>
<p class="hint">Hinzufügen: unter <b>Messwerte</b> eine Karte antippen.</p>

<button class="btn pri" id="sv" style="margin-top:22px">Reihenfolge speichern</button>
</main>
)rawliteral" NAV_HTML R"rawliteral(
<script>
const st=document.getElementById('st');
const BASE={0:'Spannung',2:'Ladedruck',3:'Verbrauch',4:'Durchschnitt',6:'Codeblock'};
let list=[],custom=[],units=[];

function cname(c){return c.title||('MWB '+c.g+' Feld '+c.f);}
function name(id){
  if(id>=10){const c=custom[id-10];if(!c||!c.g)return '?';return '★ '+cname(c)+(c.unit?' ['+c.unit+']':'');}
  if(id===6)return BASE[6];
  return (BASE[id]||'?')+' ['+(units[id]||'')+']';
}
function avail(){const a=[0,2,3,4,6];custom.forEach((c,i)=>{if(c.g)a.push(10+i);});return a;}

function row(txt,btns,sub){
  const r=document.createElement('div');r.className='row';
  const s=document.createElement('span');s.textContent=txt;
  if(sub){const m=document.createElement('small');m.textContent='  '+sub;s.appendChild(m);}
  r.appendChild(s);
  btns.forEach(([t,f,cls,lbl])=>{const b=document.createElement('button');b.className='ib'+(cls?' '+cls:'');b.textContent=t;b.onclick=f;if(lbl)b.setAttribute('aria-label',lbl);r.appendChild(b);});
  return r;
}
function changed(){st.textContent='Nicht gespeichert';st.className='pill run';draw();}
function move(i,d){const j=i+d;if(j<0||j>=list.length)return;[list[i],list[j]]=[list[j],list[i]];changed();}
function hide(i){if(list.length>1){list.splice(i,1);changed();}}

// Der Codeblock hat keine einstellbare Einheit
function editBtn(id){return id===6?[]:[['✎',()=>edit(id),'','Einheit ändern']];}

function draw(){
  const act=document.getElementById('act'),off=document.getElementById('off'),cus=document.getElementById('cus');
  act.innerHTML='';off.innerHTML='';cus.innerHTML='';
  list.forEach((id,i)=>act.appendChild(row((i+1)+'. '+name(id),[
    ...editBtn(id),['▲',()=>move(i,-1),'','Nach oben'],['▼',()=>move(i,1),'','Nach unten'],['✕',()=>hide(i),'','Ausblenden']])));
  const o=avail().filter(id=>!list.includes(id));
  o.forEach(id=>off.appendChild(row(name(id),[...editBtn(id),['+',()=>{list.push(id);changed();},'add','Anzeigen']])));
  if(!o.length){const h=document.createElement('p');h.className='hint';h.textContent='Alle Bildschirme werden angezeigt.';off.appendChild(h);}
  let any=false;
  custom.forEach((c,i)=>{if(!c.g)return;any=true;
    cus.appendChild(row('★ '+cname(c)+(c.unit?' ['+c.unit+']':''),[['✎',()=>edit(10+i),'','Einheit ändern'],['🗑',()=>del(i),'del','Löschen']],'MWB '+c.g+' / '+c.f));});
  if(!any){const h=document.createElement('p');h.className='hint';h.textContent='Keine eigenen Messwerte angelegt.';cus.appendChild(h);}
}

async function load(){
  try{
    const d=await (await fetch('/api/oled',{cache:'no-store'})).json();
    list=d.list;custom=d.custom;units=d.units||[];
    st.textContent='Geladen';st.className='pill ok';draw();
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
}

async function edit(id){
  const cur=id>=10?(custom[id-10].unit||''):(units[id]||'');
  const n=prompt('Einheit / Bezeichnung auf dem OLED\n(max. 9 Zeichen, ohne Umlaute, leer = Standard):',cur);
  if(n===null)return;
  try{
    const d=await (await fetch('/api/oled/unit?id='+id+'&n='+encodeURIComponent(n),{method:'POST'})).json();
    if(!d.ok){st.textContent='Fehler';st.className='pill err';return;}
    if(id>=10)custom[id-10].unit=d.n;else units[id]=d.n;
    st.textContent='Einheit gespeichert';st.className='pill ok';draw();
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
}

async function del(i){
  if(!confirm('Eigenen Messwert „'+cname(custom[i])+'“ endgültig löschen?'))return;
  try{
    const d=await (await fetch('/api/oled/delete?slot='+i,{method:'POST'})).json();
    if(!d.ok){st.textContent='Fehler';st.className='pill err';return;}
    custom[i]={g:0,f:0,title:'',unit:''};
    list=list.filter(id=>id!==10+i);if(!list.length)list=[0];
    st.textContent='Gelöscht';st.className='pill ok';draw();
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
}

document.getElementById('sv').onclick=async()=>{
  try{
    const r=await fetch('/api/oled/save?list='+list.join(','),{method:'POST'});
    st.textContent=r.ok?'Gespeichert':'Fehler';st.className='pill '+(r.ok?'ok':'err');
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
};
load();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// EINSTELLUNGEN
// =====================================================
const char SETTINGS_HTML[] PROGMEM = HEAD_HTML("Einstellungen") R"rawliteral(
<body>
<header><div class="hd"><div><h1>Einstellungen</h1><div class="sub">Wird im Gerät gespeichert</div></div><span class="pill" id="st">…</span></div></header>
<main>
<h2>Taster</h2>

<div class="set">
  <div class="set-h"><label for="lp">Langer Druck</label><button class="info" aria-expanded="false" aria-controls="d-lp" aria-label="Info zu Langer Druck">i</button></div>
  <div class="desc" id="d-lp">So lange musst du den Knopf <b>im Codeblock-Bildschirm</b> halten, damit die Codierung zwischen 2 (102 PS) und 3 (138 PS) umgeschaltet wird. Ein Druck, der kürzer ist, zählt als kurzer Druck und schaltet zum nächsten Bildschirm.<br><br>Länger = weniger Gefahr, versehentlich umzucodieren. Bereich 1,0–10,0 s.</div>
  <div class="ctl"><button class="ib" data-t="lp" data-s="-0.5" aria-label="Weniger">&minus;</button><input id="lp" type="number" step="0.5" min="1" max="10" inputmode="decimal"><button class="ib" data-t="lp" data-s="0.5" aria-label="Mehr">+</button><span class="u">s</span></div>
  <div class="meta">Standard: <b id="df-lp">-</b></div>
</div>

<div class="set">
  <div class="set-h"><label for="dp">Diagnose aus / an</label><button class="info" aria-expanded="false" aria-controls="d-dp" aria-label="Info zu Diagnose aus / an">i</button></div>
  <div class="desc" id="d-dp">So lange musst du den Knopf <b>im Spannungsbildschirm</b> halten, um die Diagnoseverbindung zu trennen. Am OLED steht dann „Diag aus“ und die K-Line ist frei, z.&nbsp;B. für VCDS. Das bleibt auch nach einem Neustart so. Gleich lange halten schaltet die Diagnose wieder ein.<br><br>Muss mindestens 1 s länger sein als der lange Druck. Bereich 2,0–20,0 s.</div>
  <div class="ctl"><button class="ib" data-t="dp" data-s="-0.5" aria-label="Weniger">&minus;</button><input id="dp" type="number" step="0.5" min="2" max="20" inputmode="decimal"><button class="ib" data-t="dp" data-s="0.5" aria-label="Mehr">+</button><span class="u">s</span></div>
  <div class="meta">Standard: <b id="df-dp">-</b></div>
</div>

<h2>Spannung</h2>

<div class="set">
  <div class="set-h"><label for="vo">Spannungskorrektur</label><button class="info" aria-expanded="false" aria-controls="d-vo" aria-label="Info zu Spannungskorrektur">i</button></div>
  <div class="desc" id="d-vo">Wird zur gemessenen Bordspannung addiert. So gleichst du Toleranzen von Spannungsteiler und ADC aus.<br><br><b>So geht's:</b> Bordspannung mit einem Multimeter messen (z.&nbsp;B. an der Batterie) und mit dem Wert unten vergleichen. Zeigt das Gerät 0,20 V zu wenig, trägst du <b>+0,20</b> ein, bei zu viel einen negativen Wert. Bereich −3,00 bis +3,00 V.</div>
  <div class="ctl"><button class="ib" data-t="vo" data-s="-0.05" aria-label="Weniger">&minus;</button><input id="vo" type="number" step="0.05" min="-3" max="3" inputmode="decimal"><button class="ib" data-t="vo" data-s="0.05" aria-label="Mehr">+</button><span class="u">V</span></div>
  <div class="meta">Gerät misst gerade: <b id="volt">-</b> (mit gespeicherter Korrektur) · Standard: <b id="df-vo">-</b></div>
</div>

<h2>Verbrauch</h2>

<div class="set">
  <div class="set-h"><label for="sp">Geschwindigkeitsabweichung</label><button class="info" aria-expanded="false" aria-controls="d-sp" aria-label="Info zu Geschwindigkeitsabweichung">i</button></div>
  <div class="desc" id="d-sp">Das Steuergerät kennt deine Reifengröße nicht, deshalb weicht seine Geschwindigkeit oft von der echten ab. Mit diesem Wert wird sie für die Berechnung von <b>Verbrauch und Durchschnitt</b> korrigiert. Bei <b>−1,5 %</b> wird mit 1,5 % weniger Geschwindigkeit gerechnet.<br><br><b>So geht's:</b> Bei konstanter Fahrt die Geschwindigkeit vom GPS mit dem Wert in <b>Messwerte → Block 6, Feld 1</b> vergleichen. Abweichung = (GPS ÷ Steuergerät − 1) × 100. Beispiel: Steuergerät 100 km/h, GPS 98,5 km/h ergibt −1,5 %.<br><br>Der angezeigte Wert in Block 6 bleibt unverändert. Bereich −15,0 bis +15,0 %.</div>
  <div class="ctl"><button class="ib" data-t="sp" data-s="-0.1" aria-label="Weniger">&minus;</button><input id="sp" type="number" step="0.1" min="-15" max="15" inputmode="decimal"><button class="ib" data-t="sp" data-s="0.1" aria-label="Mehr">+</button><span class="u">%</span></div>
  <div class="meta">Beispiel: 100 km/h vom Steuergerät → gerechnet <b id="spEx">-</b> · Standard: <b id="df-sp">-</b></div>
</div>

<h2>Steuergeräte</h2>

<div class="set">
  <div class="set-h"><label>Gefundene Baudraten</label><button class="info" aria-expanded="false" aria-controls="d-bd" aria-label="Info zu Baudraten">i</button></div>
  <div class="desc" id="d-bd">Jedes Steuergerät spricht mit einer festen Geschwindigkeit (Baudrate). Beim ersten Fehlerspeicher-Lesen probiert das Gerät 9600, 10400, 4800, 2400 und 1200 Baud durch – das kann bis ca. 20 s dauern. Die gefundene Baudrate wird <b>dauerhaft gespeichert</b>, danach geht es sofort.<br><br>Passt eine gespeicherte Baudrate nicht mehr (z. B. nach Tausch eines Steuergeräts), wird sie automatisch verworfen und neu gesucht. Nur zur Info, hier ist nichts einzustellen.</div>
  <div id="bauds" style="margin-top:6px"></div>
</div>

<div class="btns">
  <button class="btn" id="rs">Standardwerte</button>
  <button class="btn pri" id="sv">Speichern</button>
</div>
<div class="note" id="msg" hidden></div>
</main>
)rawliteral" NAV_HTML ECU_JS R"rawliteral(
<script>
const st=document.getElementById('st'),msg=document.getElementById('msg');
const lp=document.getElementById('lp'),dp=document.getElementById('dp'),vo=document.getElementById('vo'),sp=document.getElementById('sp');
const fmt=(v,d)=>Number(v).toFixed(d).replace('.',',');
let def=null,dirty=false;

document.querySelectorAll('.info').forEach(b=>b.onclick=()=>{
  const d=document.getElementById(b.getAttribute('aria-controls'));
  const o=!d.classList.contains('open');
  d.classList.toggle('open',o);b.setAttribute('aria-expanded',o);
});

function mark(){dirty=true;st.textContent='Nicht gespeichert';st.className='pill run';msg.hidden=true;}
[lp,dp,vo,sp].forEach(i=>i.oninput=()=>{mark();spEx();});
function spEx(){const v=parseFloat(sp.value);document.getElementById('spEx').textContent=isNaN(v)?'-':fmt(100+v,1)+' km/h';}

document.querySelectorAll('.ctl .ib').forEach(b=>b.onclick=()=>{
  const i=document.getElementById(b.dataset.t);
  const s=parseFloat(b.dataset.s),dec=b.dataset.t==='vo'?2:1;
  let v=(parseFloat(i.value)||0)+s;
  v=Math.min(parseFloat(i.max),Math.max(parseFloat(i.min),v));
  i.value=v.toFixed(dec);mark();spEx();
});

function showBauds(b){
  const box=document.getElementById('bauds');box.innerHTML='';
  ECUS.forEach(([a,n])=>{
    const r=document.createElement('div');r.className='kv';
    const l=document.createElement('span');l.textContent=a+' – '+n;
    const v=document.createElement('b');v.textContent=b[a]?b[a]+' Baud':'noch nicht ermittelt';
    if(!b[a])v.className='dim';
    r.appendChild(l);r.appendChild(v);box.appendChild(r);
  });
}

function fill(d){lp.value=(d.lp/1000).toFixed(1);dp.value=(d.dp/1000).toFixed(1);vo.value=Number(d.vo).toFixed(2);sp.value=Number(d.sp).toFixed(1);spEx();}

async function load(){
  try{
    const d=await (await fetch('/api/settings',{cache:'no-store'})).json();
    def=d.def;
    document.getElementById('df-lp').textContent=fmt(def.lp/1000,1)+' s';
    document.getElementById('df-dp').textContent=fmt(def.dp/1000,1)+' s';
    document.getElementById('df-vo').textContent=fmt(def.vo,2)+' V';
    document.getElementById('df-sp').textContent=fmt(def.sp,1)+' %';
    document.getElementById('volt').textContent=fmt(d.volt,2)+' V';
    showBauds(d.bauds||{});
    if(!dirty){fill(d);st.textContent='Gespeichert';st.className='pill ok';}
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
}

// Spannung live nachführen, ohne Eingaben zu überschreiben
async function volt(){
  try{const d=await (await fetch('/api/settings',{cache:'no-store'})).json();document.getElementById('volt').textContent=fmt(d.volt,2)+' V';}catch(e){}
  setTimeout(volt,1500);
}

document.getElementById('rs').onclick=()=>{if(def){fill(def);mark();}};

document.getElementById('sv').onclick=async()=>{
  const q='lp='+Math.round(parseFloat(lp.value)*1000)+'&dp='+Math.round(parseFloat(dp.value)*1000)+'&vo='+parseFloat(vo.value).toFixed(2)+'&sp='+parseFloat(sp.value).toFixed(1);
  try{
    const r=await fetch('/api/settings/save?'+q,{method:'POST'});
    const d=await r.json();
    if(d.ok){dirty=false;st.textContent='Gespeichert';st.className='pill ok';msg.hidden=true;load();}
    else{st.textContent='Fehler';st.className='pill err';msg.hidden=false;msg.textContent=d.msg;}
  }catch(e){st.textContent='ESP offline';st.className='pill err';}
};

load().then(()=>setTimeout(volt,1500));
</script>
</body>
</html>
)rawliteral";
