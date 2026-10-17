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
<title>CodeBlockSwitchT4</title>
<style>
:root{color-scheme:dark;--bg:#0b1018;--surface:#121b28;--surface2:#182436;--line:#26364b;--text:#edf4fc;--muted:#92a3b8;--accent:#63c5ff;--green:#34c98b;--red:#ff6574;--amber:#f6ba54;--radius:18px}
*{box-sizing:border-box}body{margin:0;min-height:100vh;font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;color:var(--text);background:radial-gradient(ellipse at 50% -20%,#1c3550 0,transparent 48%),var(--bg);line-height:1.45}
header{position:sticky;top:0;z-index:5;max-width:920px;margin:0 auto;padding:20px 22px 14px;font-size:21px;font-weight:750;letter-spacing:-.025em;display:flex;justify-content:space-between;align-items:center;background:linear-gradient(180deg,rgba(11,16,24,.98),rgba(11,16,24,.88));backdrop-filter:blur(14px)}
body>div,body>button,body>h3{max-width:920px;margin-left:auto!important;margin-right:auto!important}a,.back{color:var(--accent);text-decoration:none;font-size:14px;font-weight:650}a:hover,.back:hover{color:#b5e6ff}
#st{font-size:12px;font-weight:700;letter-spacing:.01em;padding:6px 11px;border-radius:999px;background:#293649;color:#c7d3e2;white-space:nowrap}#st.ok{background:rgba(52,201,139,.16);color:#75e3b2}#st.err{background:rgba(255,101,116,.16);color:#ff9aa4}#st.run{background:rgba(246,186,84,.16);color:#ffd27b}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px;padding:10px 20px 20px;max-width:920px;margin:0 auto}.card{background:linear-gradient(145deg,rgba(24,36,54,.96),rgba(18,27,40,.96));border:1px solid var(--line);border-radius:var(--radius);padding:20px;text-align:left;box-shadow:0 8px 28px rgba(0,0,0,.16);transition:transform .18s,border-color .18s}.card:hover{border-color:#3e5b7d}.lbl{font-size:12px;color:var(--muted);font-weight:650;letter-spacing:.07em;text-transform:uppercase}.val{font-variant-numeric:tabular-nums;font-size:clamp(27px,5vw,38px);font-weight:760;letter-spacing:-.04em;margin:8px 0 0}.unit{font-size:13px;color:var(--muted)}
.nav-list{display:flex;flex-direction:column;gap:10px;max-width:880px;margin:0 auto;padding:0 20px 24px}.nav-card{display:flex;align-items:center;gap:14px;width:100%;padding:14px 16px;text-align:left;color:var(--text);background:linear-gradient(110deg,var(--surface2),var(--surface));border:1px solid var(--line);border-radius:16px}.nav-card:hover{border-color:#426587;transform:translateY(-1px)}.nav-icon{display:grid;place-items:center;flex:none;width:42px;height:42px;border-radius:13px;background:rgba(99,197,255,.12);color:var(--accent);font-size:19px;font-weight:800}.nav-copy{display:flex;flex-direction:column;gap:2px;flex:1}.nav-copy strong{font-size:14px}.nav-copy small{font-size:12px;color:var(--muted)}.nav-arrow{font-size:21px;color:var(--accent);padding-left:8px}button{font:inherit;cursor:pointer;transition:filter .15s,transform .15s,border-color .15s}button:hover:not(:disabled){filter:brightness(1.12)}button:active:not(:disabled){transform:translateY(1px)}button:focus-visible,input:focus-visible,a:focus-visible{outline:2px solid var(--accent);outline-offset:3px}button:disabled{opacity:.42;cursor:wait}
.sel{display:flex;gap:10px;padding:0 20px 12px;align-items:center;max-width:920px;margin:0 auto}.sel button{background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:22px;width:52px;height:50px}.sel input{flex:1;min-width:0;background:var(--surface);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:21px;height:50px;text-align:center;font-weight:700}.grid .card{padding:18px}.grid .card[style*="cursor: pointer"]{cursor:pointer}.raw{font-size:11px;color:#718198;margin-top:8px}.soll{font-size:12px;color:#79dda9;margin-top:7px}#msg{max-width:880px;margin:0 auto;padding:8px 20px 14px;color:var(--muted);font-size:14px}
.act{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:8px 20px 14px;max-width:920px;margin:0 auto}.act button{border:1px solid transparent;border-radius:14px;padding:15px;font-size:15px;font-weight:700;color:#fff}.rd{background:#1765a0}.cl{background:#9e303b}#msg:has(+ .list){background:var(--surface);border:1px solid var(--line);border-radius:14px;margin:0 auto 16px;padding:13px 16px;width:calc(100% - 40px)}.list{padding:0 20px 24px;display:flex;flex-direction:column;gap:12px;max-width:920px;margin:0 auto}.f{background:var(--surface);border:1px solid var(--line);border-left:4px solid var(--red);border-radius:16px;padding:18px;box-shadow:0 8px 24px rgba(0,0,0,.14)}.f.sp{border-left-color:var(--amber)}.top{display:flex;justify-content:space-between;align-items:center;gap:12px}.code{font-size:23px;font-weight:800;letter-spacing:.03em}.tag{font-size:11px;font-weight:750;padding:5px 10px;border-radius:999px;background:rgba(255,101,116,.15);color:#ff9aa4}.sp .tag{background:rgba(246,186,84,.15);color:#ffd27b}.txt{color:#c2cede;margin-top:10px;font-size:14px}.obd{font-size:12px;color:var(--accent);margin-left:9px;font-weight:700}.ttl{font-size:16px;font-weight:700;margin-top:10px}.sec{margin-top:12px;font-size:13px;line-height:1.55;color:#c5d1df}.sec b{display:block;color:var(--muted);font-size:10px;text-transform:uppercase;letter-spacing:.09em;margin-bottom:3px}.unk{margin-top:10px;font-size:13px;color:var(--amber)}.empty{text-align:center;color:#82e0ae;background:rgba(52,201,139,.1);border:1px solid rgba(52,201,139,.22);padding:20px;border-radius:16px}
.back{display:block;background:none;border:0;padding:2px 20px 10px;max-width:920px}.back:hover{background:none}.row{display:flex;align-items:center;gap:8px;background:var(--surface);border:1px solid var(--line);border-radius:14px;padding:10px 12px;margin:0 20px 9px;max-width:880px}.row span{flex:1;min-width:0;font-size:14px;font-weight:560;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.row button{flex:none;background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:10px;width:36px;height:36px;font-size:15px}.row button.del{background:rgba(255,101,116,.16);border-color:rgba(255,101,116,.2);color:#ff9aa4}h3{font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.12em;margin-top:22px!important;margin-bottom:9px!important;padding:0 20px}.hint{color:#8191a6;font-size:12px;margin:0 20px 10px;max-width:880px}.save{display:block;width:calc(100% - 40px);max-width:880px;margin:20px auto 30px;background:linear-gradient(135deg,#188a62,#14724f);color:white;border:0;border-radius:14px;padding:15px;font-size:15px;font-weight:750;box-shadow:0 8px 22px rgba(20,114,79,.2)}
@media(min-width:700px){.grid{grid-template-columns:repeat(4,minmax(0,1fr));padding-top:18px}.grid .card{padding:22px}.act{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:480px){.nav-list{padding-left:14px;padding-right:14px}.nav-card{padding:12px}.nav-icon{width:38px;height:38px}header{padding:17px 18px 12px;font-size:19px}.grid{gap:10px;padding:8px 14px 16px}.grid .card{padding:15px;border-radius:15px}.val{font-size:29px}.sel{padding-left:14px;padding-right:14px}.act{padding-left:14px;padding-right:14px;gap:9px}.act button{padding:14px 8px;font-size:14px}.list{padding-left:14px;padding-right:14px}.row{margin-left:14px;margin-right:14px}.hint,h3{margin-left:14px!important;margin-right:14px!important;padding-left:0;padding-right:0}#msg:has(+ .list){width:calc(100% - 28px)}.save{width:calc(100% - 28px)}}
</style>
</head>
<body>
<header>CodeBlockSwitchT4<span id="st">...</span></header>
<div class="grid">
  <div class="card"><div class="lbl">Drehzahl</div><div class="val" id="rpm">-</div><div class="unit">U/min</div></div>
  <div class="card"><div class="lbl">Ladedruck</div><div class="val" id="boost">-</div><div class="unit">mbar</div></div>
  <div class="card"><div class="lbl">Einspritzmenge</div><div class="val" id="inj">-</div><div class="unit">mg/Hub</div></div>
  <div class="card"><div class="lbl">Spannung</div><div class="val" id="volt">-</div><div class="unit">V</div></div>
</div>
<nav class="nav-list" aria-label="Weitere Ansichten">
  <button class="nav-card" onclick="location.href='/mwb'"><span class="nav-icon">▤</span><span class="nav-copy"><strong>Messwerteblöcke</strong><small>Einzelne Motorwerte im Detail ansehen</small></span><span class="nav-arrow">→</span></button>
  <button class="nav-card" onclick="location.href='/faults'"><span class="nav-icon">!</span><span class="nav-copy"><strong>Fehlerspeicher</strong><small>Fehlercodes lesen und verwalten</small></span><span class="nav-arrow">→</span></button>
  <button class="nav-card" onclick="location.href='/oled'"><span class="nav-icon">▣</span><span class="nav-copy"><strong>OLED-Anzeige</strong><small>Anzeige und Reihenfolge einstellen</small></span><span class="nav-arrow">→</span></button>
</nav>
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
:root{color-scheme:dark;--bg:#0b1018;--surface:#121b28;--surface2:#182436;--line:#26364b;--text:#edf4fc;--muted:#92a3b8;--accent:#63c5ff;--green:#34c98b;--red:#ff6574;--amber:#f6ba54;--radius:18px}
*{box-sizing:border-box}body{margin:0;min-height:100vh;font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;color:var(--text);background:radial-gradient(ellipse at 50% -20%,#1c3550 0,transparent 48%),var(--bg);line-height:1.45}
header{position:sticky;top:0;z-index:5;max-width:920px;margin:0 auto;padding:20px 22px 14px;font-size:21px;font-weight:750;letter-spacing:-.025em;display:flex;justify-content:space-between;align-items:center;background:linear-gradient(180deg,rgba(11,16,24,.98),rgba(11,16,24,.88));backdrop-filter:blur(14px)}
body>div,body>button,body>h3{max-width:920px;margin-left:auto!important;margin-right:auto!important}a,.back{color:var(--accent);text-decoration:none;font-size:14px;font-weight:650}a:hover,.back:hover{color:#b5e6ff}
#st{font-size:12px;font-weight:700;letter-spacing:.01em;padding:6px 11px;border-radius:999px;background:#293649;color:#c7d3e2;white-space:nowrap}#st.ok{background:rgba(52,201,139,.16);color:#75e3b2}#st.err{background:rgba(255,101,116,.16);color:#ff9aa4}#st.run{background:rgba(246,186,84,.16);color:#ffd27b}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px;padding:10px 20px 20px;max-width:920px;margin:0 auto}.card{background:linear-gradient(145deg,rgba(24,36,54,.96),rgba(18,27,40,.96));border:1px solid var(--line);border-radius:var(--radius);padding:20px;text-align:left;box-shadow:0 8px 28px rgba(0,0,0,.16);transition:transform .18s,border-color .18s}.card:hover{border-color:#3e5b7d}.lbl{font-size:12px;color:var(--muted);font-weight:650;letter-spacing:.07em;text-transform:uppercase}.val{font-variant-numeric:tabular-nums;font-size:clamp(27px,5vw,38px);font-weight:760;letter-spacing:-.04em;margin:8px 0 0}.unit{font-size:13px;color:var(--muted)}
.nav-list{display:flex;flex-direction:column;gap:10px;max-width:880px;margin:0 auto;padding:0 20px 24px}.nav-card{display:flex;align-items:center;gap:14px;width:100%;padding:14px 16px;text-align:left;color:var(--text);background:linear-gradient(110deg,var(--surface2),var(--surface));border:1px solid var(--line);border-radius:16px}.nav-card:hover{border-color:#426587;transform:translateY(-1px)}.nav-icon{display:grid;place-items:center;flex:none;width:42px;height:42px;border-radius:13px;background:rgba(99,197,255,.12);color:var(--accent);font-size:19px;font-weight:800}.nav-copy{display:flex;flex-direction:column;gap:2px;flex:1}.nav-copy strong{font-size:14px}.nav-copy small{font-size:12px;color:var(--muted)}.nav-arrow{font-size:21px;color:var(--accent);padding-left:8px}button{font:inherit;cursor:pointer;transition:filter .15s,transform .15s,border-color .15s}button:hover:not(:disabled){filter:brightness(1.12)}button:active:not(:disabled){transform:translateY(1px)}button:focus-visible,input:focus-visible,a:focus-visible{outline:2px solid var(--accent);outline-offset:3px}button:disabled{opacity:.42;cursor:wait}
.sel{display:flex;gap:10px;padding:0 20px 12px;align-items:center;max-width:920px;margin:0 auto}.sel button{background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:22px;width:52px;height:50px}.sel input{flex:1;min-width:0;background:var(--surface);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:21px;height:50px;text-align:center;font-weight:700}.grid .card{padding:18px}.grid .card[style*="cursor: pointer"]{cursor:pointer}.raw{font-size:11px;color:#718198;margin-top:8px}.soll{font-size:12px;color:#79dda9;margin-top:7px}#msg{max-width:880px;margin:0 auto;padding:8px 20px 14px;color:var(--muted);font-size:14px}
.act{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:8px 20px 14px;max-width:920px;margin:0 auto}.act button{border:1px solid transparent;border-radius:14px;padding:15px;font-size:15px;font-weight:700;color:#fff}.rd{background:#1765a0}.cl{background:#9e303b}#msg:has(+ .list){background:var(--surface);border:1px solid var(--line);border-radius:14px;margin:0 auto 16px;padding:13px 16px;width:calc(100% - 40px)}.list{padding:0 20px 24px;display:flex;flex-direction:column;gap:12px;max-width:920px;margin:0 auto}.f{background:var(--surface);border:1px solid var(--line);border-left:4px solid var(--red);border-radius:16px;padding:18px;box-shadow:0 8px 24px rgba(0,0,0,.14)}.f.sp{border-left-color:var(--amber)}.top{display:flex;justify-content:space-between;align-items:center;gap:12px}.code{font-size:23px;font-weight:800;letter-spacing:.03em}.tag{font-size:11px;font-weight:750;padding:5px 10px;border-radius:999px;background:rgba(255,101,116,.15);color:#ff9aa4}.sp .tag{background:rgba(246,186,84,.15);color:#ffd27b}.txt{color:#c2cede;margin-top:10px;font-size:14px}.obd{font-size:12px;color:var(--accent);margin-left:9px;font-weight:700}.ttl{font-size:16px;font-weight:700;margin-top:10px}.sec{margin-top:12px;font-size:13px;line-height:1.55;color:#c5d1df}.sec b{display:block;color:var(--muted);font-size:10px;text-transform:uppercase;letter-spacing:.09em;margin-bottom:3px}.unk{margin-top:10px;font-size:13px;color:var(--amber)}.empty{text-align:center;color:#82e0ae;background:rgba(52,201,139,.1);border:1px solid rgba(52,201,139,.22);padding:20px;border-radius:16px}
.back{display:block;background:none;border:0;padding:2px 20px 10px;max-width:920px}.back:hover{background:none}.row{display:flex;align-items:center;gap:8px;background:var(--surface);border:1px solid var(--line);border-radius:14px;padding:10px 12px;margin:0 20px 9px;max-width:880px}.row span{flex:1;min-width:0;font-size:14px;font-weight:560;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.row button{flex:none;background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:10px;width:36px;height:36px;font-size:15px}.row button.del{background:rgba(255,101,116,.16);border-color:rgba(255,101,116,.2);color:#ff9aa4}h3{font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.12em;margin-top:22px!important;margin-bottom:9px!important;padding:0 20px}.hint{color:#8191a6;font-size:12px;margin:0 20px 10px;max-width:880px}.save{display:block;width:calc(100% - 40px);max-width:880px;margin:20px auto 30px;background:linear-gradient(135deg,#188a62,#14724f);color:white;border:0;border-radius:14px;padding:15px;font-size:15px;font-weight:750;box-shadow:0 8px 22px rgba(20,114,79,.2)}
@media(min-width:700px){.grid{grid-template-columns:repeat(4,minmax(0,1fr));padding-top:18px}.grid .card{padding:22px}.act{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:480px){.nav-list{padding-left:14px;padding-right:14px}.nav-card{padding:12px}.nav-icon{width:38px;height:38px}header{padding:17px 18px 12px;font-size:19px}.grid{gap:10px;padding:8px 14px 16px}.grid .card{padding:15px;border-radius:15px}.val{font-size:29px}.sel{padding-left:14px;padding-right:14px}.act{padding-left:14px;padding-right:14px;gap:9px}.act button{padding:14px 8px;font-size:14px}.list{padding-left:14px;padding-right:14px}.row{margin-left:14px;margin-right:14px}.hint,h3{margin-left:14px!important;margin-right:14px!important;padding-left:0;padding-right:0}#msg:has(+ .list){width:calc(100% - 28px)}.save{width:calc(100% - 28px)}}
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
:root{color-scheme:dark;--bg:#0b1018;--surface:#121b28;--surface2:#182436;--line:#26364b;--text:#edf4fc;--muted:#92a3b8;--accent:#63c5ff;--green:#34c98b;--red:#ff6574;--amber:#f6ba54;--radius:18px}
*{box-sizing:border-box}body{margin:0;min-height:100vh;font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;color:var(--text);background:radial-gradient(ellipse at 50% -20%,#1c3550 0,transparent 48%),var(--bg);line-height:1.45}
header{position:sticky;top:0;z-index:5;max-width:920px;margin:0 auto;padding:20px 22px 14px;font-size:21px;font-weight:750;letter-spacing:-.025em;display:flex;justify-content:space-between;align-items:center;background:linear-gradient(180deg,rgba(11,16,24,.98),rgba(11,16,24,.88));backdrop-filter:blur(14px)}
body>div,body>button,body>h3{max-width:920px;margin-left:auto!important;margin-right:auto!important}a,.back{color:var(--accent);text-decoration:none;font-size:14px;font-weight:650}a:hover,.back:hover{color:#b5e6ff}
#st{font-size:12px;font-weight:700;letter-spacing:.01em;padding:6px 11px;border-radius:999px;background:#293649;color:#c7d3e2;white-space:nowrap}#st.ok{background:rgba(52,201,139,.16);color:#75e3b2}#st.err{background:rgba(255,101,116,.16);color:#ff9aa4}#st.run{background:rgba(246,186,84,.16);color:#ffd27b}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px;padding:10px 20px 20px;max-width:920px;margin:0 auto}.card{background:linear-gradient(145deg,rgba(24,36,54,.96),rgba(18,27,40,.96));border:1px solid var(--line);border-radius:var(--radius);padding:20px;text-align:left;box-shadow:0 8px 28px rgba(0,0,0,.16);transition:transform .18s,border-color .18s}.card:hover{border-color:#3e5b7d}.lbl{font-size:12px;color:var(--muted);font-weight:650;letter-spacing:.07em;text-transform:uppercase}.val{font-variant-numeric:tabular-nums;font-size:clamp(27px,5vw,38px);font-weight:760;letter-spacing:-.04em;margin:8px 0 0}.unit{font-size:13px;color:var(--muted)}
.nav-list{display:flex;flex-direction:column;gap:10px;max-width:880px;margin:0 auto;padding:0 20px 24px}.nav-card{display:flex;align-items:center;gap:14px;width:100%;padding:14px 16px;text-align:left;color:var(--text);background:linear-gradient(110deg,var(--surface2),var(--surface));border:1px solid var(--line);border-radius:16px}.nav-card:hover{border-color:#426587;transform:translateY(-1px)}.nav-icon{display:grid;place-items:center;flex:none;width:42px;height:42px;border-radius:13px;background:rgba(99,197,255,.12);color:var(--accent);font-size:19px;font-weight:800}.nav-copy{display:flex;flex-direction:column;gap:2px;flex:1}.nav-copy strong{font-size:14px}.nav-copy small{font-size:12px;color:var(--muted)}.nav-arrow{font-size:21px;color:var(--accent);padding-left:8px}button{font:inherit;cursor:pointer;transition:filter .15s,transform .15s,border-color .15s}button:hover:not(:disabled){filter:brightness(1.12)}button:active:not(:disabled){transform:translateY(1px)}button:focus-visible,input:focus-visible,a:focus-visible{outline:2px solid var(--accent);outline-offset:3px}button:disabled{opacity:.42;cursor:wait}
.sel{display:flex;gap:10px;padding:0 20px 12px;align-items:center;max-width:920px;margin:0 auto}.sel button{background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:22px;width:52px;height:50px}.sel input{flex:1;min-width:0;background:var(--surface);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:21px;height:50px;text-align:center;font-weight:700}.grid .card{padding:18px}.grid .card[style*="cursor: pointer"]{cursor:pointer}.raw{font-size:11px;color:#718198;margin-top:8px}.soll{font-size:12px;color:#79dda9;margin-top:7px}#msg{max-width:880px;margin:0 auto;padding:8px 20px 14px;color:var(--muted);font-size:14px}
.act{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:8px 20px 14px;max-width:920px;margin:0 auto}.act button{border:1px solid transparent;border-radius:14px;padding:15px;font-size:15px;font-weight:700;color:#fff}.rd{background:#1765a0}.cl{background:#9e303b}#msg:has(+ .list){background:var(--surface);border:1px solid var(--line);border-radius:14px;margin:0 auto 16px;padding:13px 16px;width:calc(100% - 40px)}.list{padding:0 20px 24px;display:flex;flex-direction:column;gap:12px;max-width:920px;margin:0 auto}.f{background:var(--surface);border:1px solid var(--line);border-left:4px solid var(--red);border-radius:16px;padding:18px;box-shadow:0 8px 24px rgba(0,0,0,.14)}.f.sp{border-left-color:var(--amber)}.top{display:flex;justify-content:space-between;align-items:center;gap:12px}.code{font-size:23px;font-weight:800;letter-spacing:.03em}.tag{font-size:11px;font-weight:750;padding:5px 10px;border-radius:999px;background:rgba(255,101,116,.15);color:#ff9aa4}.sp .tag{background:rgba(246,186,84,.15);color:#ffd27b}.txt{color:#c2cede;margin-top:10px;font-size:14px}.obd{font-size:12px;color:var(--accent);margin-left:9px;font-weight:700}.ttl{font-size:16px;font-weight:700;margin-top:10px}.sec{margin-top:12px;font-size:13px;line-height:1.55;color:#c5d1df}.sec b{display:block;color:var(--muted);font-size:10px;text-transform:uppercase;letter-spacing:.09em;margin-bottom:3px}.unk{margin-top:10px;font-size:13px;color:var(--amber)}.empty{text-align:center;color:#82e0ae;background:rgba(52,201,139,.1);border:1px solid rgba(52,201,139,.22);padding:20px;border-radius:16px}
.back{display:block;background:none;border:0;padding:2px 20px 10px;max-width:920px}.back:hover{background:none}.row{display:flex;align-items:center;gap:8px;background:var(--surface);border:1px solid var(--line);border-radius:14px;padding:10px 12px;margin:0 20px 9px;max-width:880px}.row span{flex:1;min-width:0;font-size:14px;font-weight:560;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.row button{flex:none;background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:10px;width:36px;height:36px;font-size:15px}.row button.del{background:rgba(255,101,116,.16);border-color:rgba(255,101,116,.2);color:#ff9aa4}h3{font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.12em;margin-top:22px!important;margin-bottom:9px!important;padding:0 20px}.hint{color:#8191a6;font-size:12px;margin:0 20px 10px;max-width:880px}.save{display:block;width:calc(100% - 40px);max-width:880px;margin:20px auto 30px;background:linear-gradient(135deg,#188a62,#14724f);color:white;border:0;border-radius:14px;padding:15px;font-size:15px;font-weight:750;box-shadow:0 8px 22px rgba(20,114,79,.2)}
@media(min-width:700px){.grid{grid-template-columns:repeat(4,minmax(0,1fr));padding-top:18px}.grid .card{padding:22px}.act{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:480px){.nav-list{padding-left:14px;padding-right:14px}.nav-card{padding:12px}.nav-icon{width:38px;height:38px}header{padding:17px 18px 12px;font-size:19px}.grid{gap:10px;padding:8px 14px 16px}.grid .card{padding:15px;border-radius:15px}.val{font-size:29px}.sel{padding-left:14px;padding-right:14px}.act{padding-left:14px;padding-right:14px;gap:9px}.act button{padding:14px 8px;font-size:14px}.list{padding-left:14px;padding-right:14px}.row{margin-left:14px;margin-right:14px}.hint,h3{margin-left:14px!important;margin-right:14px!important;padding-left:0;padding-right:0}#msg:has(+ .list){width:calc(100% - 28px)}.save{width:calc(100% - 28px)}}
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
:root{color-scheme:dark;--bg:#0b1018;--surface:#121b28;--surface2:#182436;--line:#26364b;--text:#edf4fc;--muted:#92a3b8;--accent:#63c5ff;--green:#34c98b;--red:#ff6574;--amber:#f6ba54;--radius:18px}
*{box-sizing:border-box}body{margin:0;min-height:100vh;font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;color:var(--text);background:radial-gradient(ellipse at 50% -20%,#1c3550 0,transparent 48%),var(--bg);line-height:1.45}
header{position:sticky;top:0;z-index:5;max-width:920px;margin:0 auto;padding:20px 22px 14px;font-size:21px;font-weight:750;letter-spacing:-.025em;display:flex;justify-content:space-between;align-items:center;background:linear-gradient(180deg,rgba(11,16,24,.98),rgba(11,16,24,.88));backdrop-filter:blur(14px)}
body>div,body>button,body>h3{max-width:920px;margin-left:auto!important;margin-right:auto!important}a,.back{color:var(--accent);text-decoration:none;font-size:14px;font-weight:650}a:hover,.back:hover{color:#b5e6ff}
#st{font-size:12px;font-weight:700;letter-spacing:.01em;padding:6px 11px;border-radius:999px;background:#293649;color:#c7d3e2;white-space:nowrap}#st.ok{background:rgba(52,201,139,.16);color:#75e3b2}#st.err{background:rgba(255,101,116,.16);color:#ff9aa4}#st.run{background:rgba(246,186,84,.16);color:#ffd27b}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px;padding:10px 20px 20px;max-width:920px;margin:0 auto}.card{background:linear-gradient(145deg,rgba(24,36,54,.96),rgba(18,27,40,.96));border:1px solid var(--line);border-radius:var(--radius);padding:20px;text-align:left;box-shadow:0 8px 28px rgba(0,0,0,.16);transition:transform .18s,border-color .18s}.card:hover{border-color:#3e5b7d}.lbl{font-size:12px;color:var(--muted);font-weight:650;letter-spacing:.07em;text-transform:uppercase}.val{font-variant-numeric:tabular-nums;font-size:clamp(27px,5vw,38px);font-weight:760;letter-spacing:-.04em;margin:8px 0 0}.unit{font-size:13px;color:var(--muted)}
.nav-list{display:flex;flex-direction:column;gap:10px;max-width:880px;margin:0 auto;padding:0 20px 24px}.nav-card{display:flex;align-items:center;gap:14px;width:100%;padding:14px 16px;text-align:left;color:var(--text);background:linear-gradient(110deg,var(--surface2),var(--surface));border:1px solid var(--line);border-radius:16px}.nav-card:hover{border-color:#426587;transform:translateY(-1px)}.nav-icon{display:grid;place-items:center;flex:none;width:42px;height:42px;border-radius:13px;background:rgba(99,197,255,.12);color:var(--accent);font-size:19px;font-weight:800}.nav-copy{display:flex;flex-direction:column;gap:2px;flex:1}.nav-copy strong{font-size:14px}.nav-copy small{font-size:12px;color:var(--muted)}.nav-arrow{font-size:21px;color:var(--accent);padding-left:8px}button{font:inherit;cursor:pointer;transition:filter .15s,transform .15s,border-color .15s}button:hover:not(:disabled){filter:brightness(1.12)}button:active:not(:disabled){transform:translateY(1px)}button:focus-visible,input:focus-visible,a:focus-visible{outline:2px solid var(--accent);outline-offset:3px}button:disabled{opacity:.42;cursor:wait}
.sel{display:flex;gap:10px;padding:0 20px 12px;align-items:center;max-width:920px;margin:0 auto}.sel button{background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:22px;width:52px;height:50px}.sel input{flex:1;min-width:0;background:var(--surface);color:var(--text);border:1px solid var(--line);border-radius:13px;font-size:21px;height:50px;text-align:center;font-weight:700}.grid .card{padding:18px}.grid .card[style*="cursor: pointer"]{cursor:pointer}.raw{font-size:11px;color:#718198;margin-top:8px}.soll{font-size:12px;color:#79dda9;margin-top:7px}#msg{max-width:880px;margin:0 auto;padding:8px 20px 14px;color:var(--muted);font-size:14px}
.act{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:8px 20px 14px;max-width:920px;margin:0 auto}.act button{border:1px solid transparent;border-radius:14px;padding:15px;font-size:15px;font-weight:700;color:#fff}.rd{background:#1765a0}.cl{background:#9e303b}#msg:has(+ .list){background:var(--surface);border:1px solid var(--line);border-radius:14px;margin:0 auto 16px;padding:13px 16px;width:calc(100% - 40px)}.list{padding:0 20px 24px;display:flex;flex-direction:column;gap:12px;max-width:920px;margin:0 auto}.f{background:var(--surface);border:1px solid var(--line);border-left:4px solid var(--red);border-radius:16px;padding:18px;box-shadow:0 8px 24px rgba(0,0,0,.14)}.f.sp{border-left-color:var(--amber)}.top{display:flex;justify-content:space-between;align-items:center;gap:12px}.code{font-size:23px;font-weight:800;letter-spacing:.03em}.tag{font-size:11px;font-weight:750;padding:5px 10px;border-radius:999px;background:rgba(255,101,116,.15);color:#ff9aa4}.sp .tag{background:rgba(246,186,84,.15);color:#ffd27b}.txt{color:#c2cede;margin-top:10px;font-size:14px}.obd{font-size:12px;color:var(--accent);margin-left:9px;font-weight:700}.ttl{font-size:16px;font-weight:700;margin-top:10px}.sec{margin-top:12px;font-size:13px;line-height:1.55;color:#c5d1df}.sec b{display:block;color:var(--muted);font-size:10px;text-transform:uppercase;letter-spacing:.09em;margin-bottom:3px}.unk{margin-top:10px;font-size:13px;color:var(--amber)}.empty{text-align:center;color:#82e0ae;background:rgba(52,201,139,.1);border:1px solid rgba(52,201,139,.22);padding:20px;border-radius:16px}
.back{display:block;background:none;border:0;padding:2px 20px 10px;max-width:920px}.back:hover{background:none}.row{display:flex;align-items:center;gap:8px;background:var(--surface);border:1px solid var(--line);border-radius:14px;padding:10px 12px;margin:0 20px 9px;max-width:880px}.row span{flex:1;min-width:0;font-size:14px;font-weight:560;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.row button{flex:none;background:var(--surface2);color:var(--text);border:1px solid var(--line);border-radius:10px;width:36px;height:36px;font-size:15px}.row button.del{background:rgba(255,101,116,.16);border-color:rgba(255,101,116,.2);color:#ff9aa4}h3{font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.12em;margin-top:22px!important;margin-bottom:9px!important;padding:0 20px}.hint{color:#8191a6;font-size:12px;margin:0 20px 10px;max-width:880px}.save{display:block;width:calc(100% - 40px);max-width:880px;margin:20px auto 30px;background:linear-gradient(135deg,#188a62,#14724f);color:white;border:0;border-radius:14px;padding:15px;font-size:15px;font-weight:750;box-shadow:0 8px 22px rgba(20,114,79,.2)}
@media(min-width:700px){.grid{grid-template-columns:repeat(4,minmax(0,1fr));padding-top:18px}.grid .card{padding:22px}.act{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:480px){.nav-list{padding-left:14px;padding-right:14px}.nav-card{padding:12px}.nav-icon{width:38px;height:38px}header{padding:17px 18px 12px;font-size:19px}.grid{gap:10px;padding:8px 14px 16px}.grid .card{padding:15px;border-radius:15px}.val{font-size:29px}.sel{padding-left:14px;padding-right:14px}.act{padding-left:14px;padding-right:14px;gap:9px}.act button{padding:14px 8px;font-size:14px}.list{padding-left:14px;padding-right:14px}.row{margin-left:14px;margin-right:14px}.hint,h3{margin-left:14px!important;margin-right:14px!important;padding-left:0;padding-right:0}#msg:has(+ .list){width:calc(100% - 28px)}.save{width:calc(100% - 28px)}}
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