#pragma once
#include <Arduino.h>

// =====================================================
// DASHBOARD
// =====================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>TDI Dashboard</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
#st.ok{background:#1b7f3a}
#st.err{background:#a12a2a}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:0 12px 12px}
.card{background:#1e1e1e;border-radius:12px;padding:16px;text-align:center}
.lbl{font-size:13px;color:#999}
.val{font-size:34px;font-weight:700;margin:6px 0 2px}
.unit{font-size:13px;color:#999}
</style>
</head>
<body>
<header>TDI Dashboard<span id="st">...</span></header>
<div class="grid">
  <div class="card"><div class="lbl">Drehzahl</div><div class="val" id="rpm">-</div><div class="unit">U/min</div></div>
  <div class="card"><div class="lbl">Ladedruck</div><div class="val" id="boost">-</div><div class="unit">mbar</div></div>
  <div class="card"><div class="lbl">Einspritzmenge</div><div class="val" id="inj">-</div><div class="unit">mg/Hub</div></div>
  <div class="card"><div class="lbl">Spannung</div><div class="val" id="volt">-</div><div class="unit">V</div></div>
</div>
<div style="padding:0 12px 16px;display:grid;grid-template-columns:1fr 1fr;gap:12px">
  <button onclick="location.href='/mwb'" style="background:#1e1e1e;color:#6cf;border:0;border-radius:11px;padding:14px;font-size:15px">Messwerteblöcke →</button>
  <button onclick="location.href='/faults'" style="background:#1e1e1e;color:#6cf;border:0;border-radius:11px;padding:14px;font-size:15px">Fehlerspeicher →</button>
  <button onclick="location.href='/oled'" style="grid-column:1/3;background:#1e1e1e;color:#6cf;border:0;border-radius:11px;padding:14px;font-size:15px">OLED-Anzeige →</button>
</div>
<script>
const st = document.getElementById('st');

async function upd() {
  try {
    const r = await fetch('/api/live', { cache: 'no-store' });
    const d = await r.json();

    for (const k of ['rpm', 'boost', 'inj']) {
      document.getElementById(k).textContent = d.connected ? d[k] : '-';
    }
    document.getElementById('volt').textContent = d.volt;

    st.textContent = d.connected ? 'MSG verbunden' : 'MSG getrennt';
    st.className = d.connected ? 'ok' : 'err';
  } catch (e) {
    st.textContent = 'ESP offline';
    st.className = 'err';
  }

  // Erst nach Antwort neu anfragen -> kein Request-Stau
  setTimeout(upd, 500);
}

upd();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// MESSWERTEBLÖCKE
// =====================================================
const char MWB_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Messwerteblock</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
a{color:#6cf;text-decoration:none;font-size:14px}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
#st.ok{background:#1b7f3a}
#st.err{background:#a12a2a}
.sel{display:flex;gap:8px;padding:0 12px 12px;align-items:center}
.sel button{background:#2a2a2a;color:#eee;border:0;border-radius:10px;font-size:22px;width:52px;height:48px}
.sel input{flex:1;min-width:0;background:#1e1e1e;color:#eee;border:1px solid #333;border-radius:10px;font-size:22px;height:48px;text-align:center}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:0 12px 12px}
.card{background:#1e1e1e;border-radius:12px;padding:16px;text-align:center}
.lbl{font-size:13px;color:#999}
.val{font-size:30px;font-weight:700;margin:6px 0 2px;word-break:break-all}
.unit{font-size:13px;color:#999}
.raw{font-size:11px;color:#666;margin-top:6px}
.soll{font-size:12px;color:#8c8;margin-top:6px}
#msg{padding:0 16px 12px;color:#999;font-size:14px}
</style>
</head>
<body>
<header><span>Messwerteblock</span><span id="st">...</span></header>
<div style="padding:0 16px 12px"><a href="/">&larr; Dashboard</a></div>
<div class="sel">
  <button id="prev">&minus;</button>
  <input id="grp" type="number" min="1" max="255" inputmode="numeric">
  <button id="next">+</button>
</div>
<div id="msg"></div>
<div class="grid" id="grid"></div>
<script>
const st = document.getElementById('st');
const grp = document.getElementById('grp');
const grid = document.getElementById('grid');
const msg = document.getElementById('msg');

let g = parseInt(localStorage.getItem('mwbGroup')) || 1;
grp.value = g;

function setG(v) {
  v = Math.max(1, Math.min(255, parseInt(v) || 1));
  g = v;
  grp.value = v;
  localStorage.setItem('mwbGroup', v);
  grid.innerHTML = '';
  msg.textContent = 'Lade Block ' + v + '...';
}

document.getElementById('prev').onclick = () => setG(g - 1);
document.getElementById('next').onclick = () => setG(g + 1);
grp.onchange = () => setG(grp.value);

function card(i, f) {
  const c = document.createElement('div');
  c.className = 'card';
  const mk = (cls, txt) => {
    const e = document.createElement('div');
    e.className = cls;
    e.textContent = txt;
    c.appendChild(e);
  };
  const named = f.n && f.n !== '-';
  mk('lbl', named ? f.n : 'Feld ' + (i + 1));
  mk('val', f.v);
  mk('unit', f.u || '\u00a0');
  mk('raw', 'Feld ' + (i + 1) + '  K=' + f.k + '  a=' + f.a + '  b=' + f.b);
  c.style.cursor = 'pointer';
  c.onclick = () => pick(i, f);
  return c;
}

async function pick(i, f) {
  const title = f.n && f.n !== '-'
                  ? f.n
                  : 'MWB ' + g + ' Feld ' + (i + 1);

  const unit = (f.u || '').substring(0, 9);

  try {
    const r = await fetch(
      '/api/oled/custom?g=' + g +
      '&f=' + (i + 1) +
      '&title=' + encodeURIComponent(title) +
      '&unit=' + encodeURIComponent(unit),
      { method: 'POST' }
    );

    const d = await r.json();

    alert(
      d.ok
        ? '★ ' + title + ' wurde zum OLED hinzugefügt.'
        : d.msg
    );
  } catch (e) {
    alert('ESP nicht erreichbar');
  }
}

async function upd() {
  try {
    const r = await fetch('/api/mwb?group=' + g, { cache: 'no-store' });
    const d = await r.json();

    st.textContent = d.connected ? 'MSG verbunden' : 'MSG getrennt';
    st.className = d.connected ? 'ok' : 'err';

    if (d.group === g && d.dataGroup === g && d.fields.length > 0) {
      grid.innerHTML = '';
      d.fields.forEach((f, i) => grid.appendChild(card(i, f)));
      msg.textContent = 'Block ' + g +
        (d.title ? ' – ' + d.title : '') +
        (d.cond ? ' (' + d.cond + ')' : '');
    } else if (d.error && d.group === g) {
      msg.textContent = 'Block ' + g + ' konnte nicht gelesen werden.';
    } else if (!d.connected) {
      msg.textContent = 'Warte auf Verbindung zum MSG...';
    }
  } catch (e) {
    st.textContent = 'ESP offline';
    st.className = 'err';
  }
  setTimeout(upd, 500);
}

msg.textContent = 'Lade Block ' + g + '...';
upd();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// FEHLERSPEICHER
// =====================================================
const char FAULT_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fehlerspeicher</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
a{color:#6cf;text-decoration:none;font-size:14px}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
#st.ok{background:#1b7f3a}
#st.err{background:#a12a2a}
#st.run{background:#9a6500}
.act{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:0 12px 12px}
button{border:0;border-radius:11px;padding:14px;font-size:16px;font-weight:600;color:#fff}
button:disabled{opacity:.4}
.rd{background:#1b6ca8}
.cl{background:#a12a2a}
#msg{margin:0 12px 12px;padding:12px;background:#1e1e1e;border-radius:11px;color:#bbb;font-size:14px}
.list{padding:0 12px 18px;display:flex;flex-direction:column;gap:10px}
.f{background:#1e1e1e;border-radius:12px;padding:14px;border-left:5px solid #d13b3b}
.f.sp{border-left-color:#e3a11a}
.top{display:flex;justify-content:space-between;align-items:center}
.code{font-size:24px;font-weight:700;letter-spacing:1px}
.tag{font-size:12px;padding:4px 9px;border-radius:12px;background:#8e2525}
.sp .tag{background:#8a620d}
.txt{color:#bbb;margin-top:8px;font-size:14px}
.raw{color:#666;margin-top:5px;font-size:11px}
.empty{text-align:center;color:#8bd29c;background:#17351f;padding:18px;border-radius:12px}
.obd{font-size:13px;color:#6cf;margin-left:8px;font-weight:600}
.ttl{font-size:16px;font-weight:600;margin-top:8px;color:#fff}
.sec{margin-top:10px;font-size:13px;line-height:1.4}
.sec b{display:block;color:#999;font-size:11px;text-transform:uppercase;letter-spacing:.5px;margin-bottom:2px}
.unk{margin-top:8px;font-size:13px;color:#e3a11a}
</style>
</head>
<body>
<header><span>Fehlerspeicher</span><span id="st">...</span></header>
<div style="padding:0 16px 12px"><a href="/">&larr; Dashboard</a></div>
<div class="act">
  <button class="rd" id="rb">Fehler lesen</button>
  <button class="cl" id="cb">Fehler löschen</button>
</div>
<div id="msg">Noch nicht gelesen</div>
<div class="list" id="list"></div>
<script>
const st=document.getElementById('st');
const msg=document.getElementById('msg');
const list=document.getElementById('list');
const rb=document.getElementById('rb');
const cb=document.getElementById('cb');

function btn(dis){rb.disabled=dis;cb.disabled=dis;}

async function cmd(a){
  if(a==='clear' && !confirm('Fehlerspeicher wirklich löschen?')) return;
  btn(true);
  msg.textContent = a==='read' ? 'Lese Fehlerspeicher...' : 'Lösche Fehlerspeicher...';
  try{
    await fetch('/api/faults/'+a,{method:'POST',cache:'no-store'});
  }catch(e){
    msg.textContent='Anforderung fehlgeschlagen';
    btn(false);
  }
}

rb.onclick=()=>cmd('read');
cb.onclick=()=>cmd('clear');

function card(f){
  const c=document.createElement('div');
  c.className='f'+(f.sporadic?' sp':'');

  const top=document.createElement('div');
  top.className='top';

  const left=document.createElement('div');
  const code=document.createElement('span');
  code.className='code';
  code.textContent=f.code;
  left.appendChild(code);

  if(f.known){
    const obd=document.createElement('span');
    obd.className='obd';
    obd.textContent=f.obd;
    left.appendChild(obd);
  }

  const tag=document.createElement('div');
  tag.className='tag';
  tag.textContent=f.sporadic?'sporadisch':'statisch';

  top.appendChild(left);
  top.appendChild(tag);
  c.appendChild(top);

  const add=(cls,txt)=>{
    const e=document.createElement('div');
    e.className=cls;
    e.textContent=txt;
    c.appendChild(e);
    return e;
  };

  const sec=(label,txt)=>{
    const e=document.createElement('div');
    e.className='sec';
    const b=document.createElement('b');
    b.textContent=label;
    e.appendChild(b);
    e.appendChild(document.createTextNode(txt));
    c.appendChild(e);
  };

  if(f.known){
    add('ttl',f.title);
    if(f.text) add('txt',f.text);
    sec('Ursache',f.cause);
    sec('Auswirkung',f.effect);
  }else{
    if(f.text) add('txt',f.text);
    add('unk','Fehlercode nicht in der Tabelle hinterlegt');
  }

  add('raw','Fehlerort 0x'+f.hex+' | Fehlerart 0x'+f.typeHex);
  return c;
}

async function upd(){
  try{
    const r=await fetch('/api/faults',{cache:'no-store'});
    const d=await r.json();

    msg.textContent=d.status;

    if(d.running){
      st.textContent='läuft...'; st.className='run'; btn(true);
    }else{
      st.textContent=d.error?'Fehler':(d.valid?'Bereit':'Nicht gelesen');
      st.className=d.error?'err':(d.valid?'ok':'');
      btn(false);
    }

    list.innerHTML='';
    if(d.valid && d.faults.length===0){
      const e=document.createElement('div');
      e.className='empty';
      e.textContent='Kein Fehler erkannt';
      list.appendChild(e);
    }else{
      d.faults.forEach(f=>list.appendChild(card(f)));
    }
  }catch(e){
    st.textContent='ESP offline'; st.className='err';
  }
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

const char OLED_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>OLED-Anzeige</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
.back{background:none;border:0;color:#6cf;font-size:14px;padding:0 16px 4px}
h3{font-size:13px;color:#999;text-transform:uppercase;letter-spacing:.5px;margin:16px 16px 8px}
.row{display:flex;align-items:center;gap:6px;background:#1e1e1e;border-radius:10px;padding:8px 10px;margin:0 12px 8px}
.row span{flex:1;min-width:0;font-size:15px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.row button{flex:none;background:#2a2a2a;color:#eee;border:0;border-radius:8px;width:36px;height:36px;font-size:16px}
.row button.del{background:#5a1f1f}
.hint{color:#777;font-size:12px;margin:0 16px 8px}
.save{display:block;width:calc(100% - 24px);margin:16px 12px 24px;background:#1b7f3a;color:#fff;border:0;border-radius:11px;padding:14px;font-size:16px;font-weight:600}
</style>
</head>
<body>
<header><span>OLED-Anzeige</span><span id="st">...</span></header>
<button class="back" onclick="location.href='/'">&larr; Dashboard</button>

<h3>Reihenfolge (Taster)</h3>
<div id="act"></div>
<div class="hint">Erster Eintrag = Startbildschirm. Danach folgt immer: Codeblock.</div>

<h3>Nicht angezeigt</h3>
<div id="off"></div>

<h3>Eigene Messwerte</h3>
<div id="cus"></div>
<div class="hint">Hinzufügen: auf der Seite Messwerteblöcke eine Karte antippen. Löschen: 🗑</div>

<button class="save" id="sv">Reihenfolge speichern</button>

<script>
const st = document.getElementById('st');
const BASE = {0:'Spannung',1:'Drehzahl',2:'Ladedruck',3:'Verbrauch',4:'Durchschnitt',5:'Einspritzmenge'};
let list = [], custom = [], units = [];

function name(id) {
  if (id >= 10) {
    const c = custom[id - 10];

    if (!c || !c.g) return '?';

    const title = c.title || ('MWB ' + c.g + ' Feld ' + c.f);
    const unit = c.unit || '';

    return '★ ' + title + (unit ? ' [' + unit + ']' : '');
  }

  return BASE[id] + ' [' + (units[id] || '') + ']';
}

function avail() {
  const a = [0,1,2,3,4,5];
  custom.forEach((c, i) => { if (c.g) a.push(10 + i); });
  return a;
}

function row(txt, btns) {
  const r = document.createElement('div');
  r.className = 'row';
  const s = document.createElement('span');
  s.textContent = txt;
  r.appendChild(s);
  btns.forEach(([t, f, cls]) => {
    const b = document.createElement('button');
    b.textContent = t;
    if (cls) b.className = cls;
    b.onclick = f;
    r.appendChild(b);
  });
  return r;
}

function changed() {
  st.textContent = 'Nicht gespeichert';
  draw();
}

function move(i, d) {
  const j = i + d;
  if (j < 0 || j >= list.length) return;
  [list[i], list[j]] = [list[j], list[i]];
  changed();
}

function hide(i) {
  if (list.length > 1) { list.splice(i, 1); changed(); }
}

function draw() {
  const act = document.getElementById('act');
  const off = document.getElementById('off');
  const cus = document.getElementById('cus');
  act.innerHTML = ''; off.innerHTML = ''; cus.innerHTML = '';

  list.forEach((id, i) => act.appendChild(row((i + 1) + '. ' + name(id), [
    ['✎', () => edit(id)],
    ['▲', () => move(i, -1)],
    ['▼', () => move(i, 1)],
    ['✕', () => hide(i)]
  ])));

  avail().filter(id => !list.includes(id)).forEach(id =>
    off.appendChild(row(name(id), [
      ['✎', () => edit(id)],
      ['+', () => { list.push(id); changed(); }]
    ])));

  let any = false;
	custom.forEach((c, i) => {
	  if (!c.g) return;

	  any = true;

	  const title = c.title || ('MWB ' + c.g + ' Feld ' + c.f);
	  const unit = c.unit ? ' [' + c.unit + ']' : '';

	  cus.appendChild(
		row(
		  '★ ' + title + unit +
		  '  (MWB ' + c.g + ' Feld ' + c.f + ')',
		  [
			['✎', () => edit(10 + i)],
			['🗑', () => del(i), 'del']
		  ]
		)
	  );
	});
	
  if (!any) {
    const h = document.createElement('div');
    h.className = 'hint';
    h.textContent = 'Keine eigenen Messwerte angelegt.';
    cus.appendChild(h);
  }
}

async function load() {
  try {
    const r = await fetch('/api/oled', { cache: 'no-store' });
    const d = await r.json();
    list = d.list;
    custom = d.custom;
    units = d.units || [];
    st.textContent = 'Geladen';
    draw();
  } catch (e) {
    st.textContent = 'ESP offline';
  }
}

async function edit(id) {
  const cur = id >= 10
              ? (custom[id - 10].unit || '')
              : (units[id] || '');
  const n = prompt('Einheit / Bezeichnung auf dem OLED\n' +
                   '(max. 9 Zeichen, ohne Umlaute, leer = Standard):', cur);
  if (n === null) return;

  try {
    const r = await fetch('/api/oled/unit?id=' + id + '&n=' + encodeURIComponent(n),
                          { method: 'POST' });
    const d = await r.json();
    if (!d.ok) { st.textContent = 'Fehler'; return; }

	if (id >= 10) {
	  custom[id - 10].unit = d.n;
	} else {
	  units[id] = d.n;
	}


    st.textContent = 'Einheit gespeichert';
    draw();
  } catch (e) {
    st.textContent = 'ESP offline';
  }
}

// Eigenen Messwert endgültig löschen (ohne Neuladen der Seite)
async function del(i) {
  const c = custom[i];
	if (!confirm(
	  'Eigenen Messwert "' +
	  (c.title || ('MWB ' + c.g + ' Feld ' + c.f)) +
	  '" endgültig löschen?'
	)) return;

  try {
    const r = await fetch('/api/oled/delete?slot=' + i, { method: 'POST' });
    const d = await r.json();
    if (!d.ok) { st.textContent = 'Fehler'; return; }

	custom[i] = {
	  g: 0,
	  f: 0,
	  title: '',
	  unit: ''
	};
    list = list.filter(id => id !== 10 + i);
    if (list.length === 0) list = [0];

    st.textContent = 'Gelöscht';
    draw();
  } catch (e) {
    st.textContent = 'ESP offline';
  }
}

document.getElementById('sv').onclick = async () => {
  try {
    const r = await fetch('/api/oled/save?list=' + list.join(','), { method: 'POST' });
    st.textContent = r.ok ? 'Gespeichert' : 'Fehler';
  } catch (e) {
    st.textContent = 'ESP offline';
  }
};

load();
</script>
</body>
</html>
)rawliteral";