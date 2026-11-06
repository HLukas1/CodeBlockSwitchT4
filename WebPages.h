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
.cbcard{display:flex;align-items:center;gap:14px;margin-top:10px}
.cbcard>div{flex:1;min-width:0}
.cbcard .lbl{padding:0;display:flex;align-items:center;gap:7px}
.cbcard .badge{position:static}
.cbcard .btn{flex:none;width:auto;padding:12px 14px;font-size:14px}
.modal{position:fixed;inset:0;z-index:20;display:grid;place-items:center;padding:24px;background:rgba(0,0,0,.72)}
.modal[hidden]{display:none}
.dlg{width:100%;max-width:360px;background:var(--surface2);border:1px solid var(--line2);border-radius:var(--r);padding:22px;text-align:center}
.dlg h3{margin:0;font-size:19px;font-weight:720;letter-spacing:-.01em}
.dlg p{margin:10px 0 18px;font-size:14px;color:#c3cedb}
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
<link rel="stylesheet" href="/app.css?v=4">
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
<div class="card cbcard" id="cb">
  <div>
    <div class="lbl">Codeblock<span class="badge" id="cbBadge" hidden>OLED</span></div>
    <div class="val dim" id="cbVal">-</div>
    <div class="unit" id="cbPs">&nbsp;</div>
  </div>
  <button class="btn pri" id="cbBtn" hidden></button>
</div>
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

$('cbBtn').onclick=async()=>{
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
  if(!cbWait){$('cbErr').hidden=!d.cbErr;$('cbErr').textContent=d.cbErr;}
  if(t0>=ackDone){
    $('cycle').hidden=!d.cbCycle;
    if(d.cbCycle)$('cycleP').textContent=(known?PS[coding]:'Codeblock '+coding)+' wurde geschrieben. Zündung aus- und wieder einschalten, damit die Änderung wirksam wird.';
  }
}

function build(items){
  grid.innerHTML='';
  items.forEach(it=>{
    const c=document.createElement('div');
    c.className='card';c.id='c'+it.id;
    c.innerHTML='<div class="lbl"></div><div class="val">-</div><div class="unit"></div><span class="badge" hidden>OLED</span>';
    c.querySelector('.lbl').textContent=it.id>=10?'★ '+it.n:it.n;
    grid.appendChild(c);
  });
}

async function upd(){
  try{
    const t0=Date.now();
    const d=await (await fetch('/api/live',{cache:'no-store'})).json();
    showCb(d,t0);
    const s=d.items.map(i=>i.id+i.n).join('|');
    if(s!==sig){sig=s;build(d.items);}

    d.items.forEach(it=>{
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
<header><div class="hd"><div><h1>Fehlerspeicher</h1><div class="sub">Motorsteuergerät (Adresse 01)</div></div><span class="pill" id="st">…</span></div></header>
<main>
<div class="btns">
  <button class="btn pri" id="rb">Fehler lesen</button>
  <button class="btn dan" id="cb">Fehler löschen</button>
</div>
<div class="note" id="msg">Noch nicht gelesen</div>
<h2>Einträge</h2>
<div id="list"></div>
</main>
)rawliteral" NAV_HTML R"rawliteral(
<script>
const st=document.getElementById('st'),msg=document.getElementById('msg'),list=document.getElementById('list'),rb=document.getElementById('rb'),cb=document.getElementById('cb');

function btn(dis){rb.disabled=dis;cb.disabled=dis;}

async function cmd(a){
  if(a==='clear'&&!confirm('Fehlerspeicher wirklich löschen?'))return;
  btn(true);
  msg.textContent=a==='read'?'Lese Fehlerspeicher …':'Lösche Fehlerspeicher …';
  try{await fetch('/api/faults/'+a,{method:'POST',cache:'no-store'});}
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

async function upd(){
  try{
    const d=await (await fetch('/api/faults',{cache:'no-store'})).json();
    msg.textContent=d.status;
    if(d.running){st.textContent='läuft …';st.className='pill run';btn(true);}
    else{
      st.textContent=d.error?'Fehler':(d.valid?'Bereit':'Nicht gelesen');
      st.className='pill'+(d.error?' err':(d.valid?' ok':''));
      btn(false);
    }
    list.innerHTML='';
    if(d.valid&&d.faults.length===0){
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
<p class="hint">Erster Eintrag = Startbildschirm. Danach folgt immer der Codeblock. Diese Werte zeigt auch die Live-Seite.</p>

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
const BASE={0:'Spannung',2:'Ladedruck',3:'Verbrauch',4:'Durchschnitt'};
let list=[],custom=[],units=[];

function cname(c){return c.title||('MWB '+c.g+' Feld '+c.f);}
function name(id){
  if(id>=10){const c=custom[id-10];if(!c||!c.g)return '?';return '★ '+cname(c)+(c.unit?' ['+c.unit+']':'');}
  return (BASE[id]||'?')+' ['+(units[id]||'')+']';
}
function avail(){const a=[0,2,3,4];custom.forEach((c,i)=>{if(c.g)a.push(10+i);});return a;}

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

function draw(){
  const act=document.getElementById('act'),off=document.getElementById('off'),cus=document.getElementById('cus');
  act.innerHTML='';off.innerHTML='';cus.innerHTML='';
  list.forEach((id,i)=>act.appendChild(row((i+1)+'. '+name(id),[
    ['✎',()=>edit(id),'','Einheit ändern'],['▲',()=>move(i,-1),'','Nach oben'],['▼',()=>move(i,1),'','Nach unten'],['✕',()=>hide(i),'','Ausblenden']])));
  const o=avail().filter(id=>!list.includes(id));
  o.forEach(id=>off.appendChild(row(name(id),[['✎',()=>edit(id),'','Einheit ändern'],['+',()=>{list.push(id);changed();},'add','Anzeigen']])));
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

<div class="btns">
  <button class="btn" id="rs">Standardwerte</button>
  <button class="btn pri" id="sv">Speichern</button>
</div>
<div class="note" id="msg" hidden></div>
</main>
)rawliteral" NAV_HTML R"rawliteral(
<script>
const st=document.getElementById('st'),msg=document.getElementById('msg');
const lp=document.getElementById('lp'),dp=document.getElementById('dp'),vo=document.getElementById('vo');
const fmt=(v,d)=>Number(v).toFixed(d).replace('.',',');
let def=null,dirty=false;

document.querySelectorAll('.info').forEach(b=>b.onclick=()=>{
  const d=document.getElementById(b.getAttribute('aria-controls'));
  const o=!d.classList.contains('open');
  d.classList.toggle('open',o);b.setAttribute('aria-expanded',o);
});

function mark(){dirty=true;st.textContent='Nicht gespeichert';st.className='pill run';msg.hidden=true;}
[lp,dp,vo].forEach(i=>i.oninput=mark);

document.querySelectorAll('.ctl .ib').forEach(b=>b.onclick=()=>{
  const i=document.getElementById(b.dataset.t);
  const s=parseFloat(b.dataset.s),dec=b.dataset.t==='vo'?2:1;
  let v=(parseFloat(i.value)||0)+s;
  v=Math.min(parseFloat(i.max),Math.max(parseFloat(i.min),v));
  i.value=v.toFixed(dec);mark();
});

function fill(d){lp.value=(d.lp/1000).toFixed(1);dp.value=(d.dp/1000).toFixed(1);vo.value=Number(d.vo).toFixed(2);}

async function load(){
  try{
    const d=await (await fetch('/api/settings',{cache:'no-store'})).json();
    def=d.def;
    document.getElementById('df-lp').textContent=fmt(def.lp/1000,1)+' s';
    document.getElementById('df-dp').textContent=fmt(def.dp/1000,1)+' s';
    document.getElementById('df-vo').textContent=fmt(def.vo,2)+' V';
    document.getElementById('volt').textContent=fmt(d.volt,2)+' V';
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
  const q='lp='+Math.round(parseFloat(lp.value)*1000)+'&dp='+Math.round(parseFloat(dp.value)*1000)+'&vo='+parseFloat(vo.value).toFixed(2);
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
