import json, math
C = json.load(open("curves.json"))

VALUES   = [0.62, 0.48, 0.35, 0.55, 0.70]
LABELS   = ["AMOUNT", "LENGTH", "TONE", "DRIVE", "DUCK"]
READOUTS = ["62%", "1.8 s", "128 Hz", "55%", "70%"]

def knob(v, r=27):
    circ  = 2 * math.pi * r
    sweep = 0.75 * circ
    th    = math.radians(135 + v * 270)
    # detent ticks around the travel
    ticks = []
    for i in range(11):
        a  = math.radians(135 + (i / 10) * 270)
        r0, r1 = r + 6, r + (10 if i in (0, 5, 10) else 8)
        ticks.append('<line x1="%.2f" y1="%.2f" x2="%.2f" y2="%.2f" stroke="%s" stroke-width="%s"/>'
                     % (r0*math.cos(a), r0*math.sin(a), r1*math.cos(a), r1*math.sin(a),
                        "#8e8b83" if i in (0,5,10) else "#adaaa2", "1.4" if i in (0,5,10) else "1"))
    return (
      '<svg width="96" height="96" viewBox="-48 -48 96 96" style="overflow:visible;display:block">'
      '<g>%s</g>'
      '<circle r="%g" fill="none" stroke="#0f0f11" stroke-width="2.5" stroke-linecap="round" stroke-dasharray="%.2f %.2f" transform="rotate(135)"/>'
      '<circle r="%g" fill="none" stroke="#ff7a18" stroke-width="2.5" stroke-linecap="round" stroke-dasharray="%.2f %.2f" transform="rotate(135)"/>'
      '<circle r="%g" fill="rgba(0,0,0,.20)"/>'
      '<circle r="%g" fill="url(#cap)" stroke="#141416" stroke-width="1"/>'
      '<line x1="%.2f" y1="%.2f" x2="%.2f" y2="%.2f" stroke="#f4f4f2" stroke-width="2.6" stroke-linecap="round"/>'
      '</svg>'
    ) % ("".join(ticks), r, sweep, circ, r, v*sweep, circ, r-4.5, r-6,
         (r-16)*math.cos(th), (r-16)*math.sin(th), (r-8)*math.cos(th), (r-8)*math.sin(th))

DEFS = ('<svg width="0" height="0" style="position:absolute">'
        '<defs><radialGradient id="cap" cx="34%" cy="26%" r="82%">'
        '<stop offset="0%" stop-color="#4a4a4f"/><stop offset="55%" stop-color="#2a2a2d"/>'
        '<stop offset="100%" stop-color="#151517"/></radialGradient></defs></svg>')

knobs = ""
for v, lab, ro in zip(VALUES, LABELS, READOUTS):
    knobs += (
      '<div style="display:flex;flex-direction:column;align-items:center;gap:7px">%s'
      '<div style="font:500 9px/1 \'IBM Plex Mono\',monospace;letter-spacing:.2em;color:#6d6a63">%s</div>'
      '<div style="font:600 13px/1 \'IBM Plex Mono\',monospace;color:#26262a">%s</div></div>'
    ) % (knob(v), lab, ro)

def cross(pos):
    return ('<svg width="9" height="9" viewBox="0 0 9 9" style="position:absolute;%s;opacity:.32">'
            '<path d="M4.5 0v9M0 4.5h9" stroke="#5f5c55" stroke-width="1"/></svg>') % pos

def led(txt, size=26):
    return ('<div style="font:600 %dpx/1 \'IBM Plex Mono\',monospace;letter-spacing:.05em;color:#ff7a18;'
            'text-shadow:0 0 14px rgba(255,122,24,.6),0 0 3px rgba(255,122,24,.9)">%s</div>') % (size, txt)

BODY = '''<div style="width:900px;height:520px;background:linear-gradient(178deg,#e2e0da 0%,#d3d0c9 52%,#c6c3bb 100%);font-family:'Archivo',system-ui,sans-serif;display:flex;flex-direction:column;overflow:hidden;position:relative;box-shadow:inset 0 1px 0 rgba(255,255,255,.65)">
  <div style="position:absolute;inset:0;opacity:.55;pointer-events:none;background-image:repeating-linear-gradient(90deg,rgba(0,0,0,.026) 0 1px,transparent 1px 3px)"></div>
  __DEFS__
  __X1____X2____X3____X4__

  <div style="display:flex;align-items:center;justify-content:space-between;padding:15px 24px;border-bottom:1px solid #b5b2aa;box-shadow:0 1px 0 rgba(255,255,255,.5);position:relative">
    <div style="display:flex;align-items:center;gap:13px">
      <div style="font:700 19px/1 'Archivo',sans-serif;letter-spacing:.01em;color:#1c1c1e">RUMBLE</div>
      <div style="width:1px;height:15px;background:#b5b2aa"></div>
      <div style="font:400 9px/1 'IBM Plex Mono',monospace;letter-spacing:.19em;color:#827f77">KICK TAIL GENERATOR</div>
    </div>
    <div style="display:flex;align-items:center;gap:11px">
      <div style="display:flex;align-items:center;gap:7px;padding:6px 13px;border-radius:3px;background:linear-gradient(180deg,#cbc8c0,#bcb9b1);border:1px solid #a8a59d;box-shadow:inset 0 1px 0 rgba(255,255,255,.45)">
        <span style="font:500 9px/1 'IBM Plex Mono',monospace;letter-spacing:.13em;color:#4b4943">WAREHOUSE 01</span>
        <svg width="8" height="5" viewBox="0 0 8 5"><path d="M1 1l3 3 3-3" stroke="#6d6a63" stroke-width="1.3" fill="none" stroke-linecap="round"/></svg>
      </div>
      <div style="width:12px;height:12px;border-radius:50%;background:#ff7a18;box-shadow:0 0 11px rgba(255,122,24,.85),inset 0 -1px 2px rgba(0,0,0,.3)"></div>
    </div>
  </div>

  <div style="display:flex;gap:16px;padding:18px 24px 0;position:relative">

    <div style="flex-grow:1;height:198px;border-radius:4px;background:#0e0e10;border:1px solid #96938b;box-shadow:inset 0 2px 12px rgba(0,0,0,.75),0 1px 0 rgba(255,255,255,.4);position:relative;overflow:hidden">
      <svg viewBox="0 0 800 200" preserveAspectRatio="none" style="position:absolute;inset:0;width:100%;height:100%">
        <defs><linearGradient id="fill" x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stop-color="#ff7a18" stop-opacity=".30"/>
          <stop offset="100%" stop-color="#ff7a18" stop-opacity=".015"/>
        </linearGradient></defs>
        <g stroke="#1c1c20" stroke-width="1">
          <line x1="0" y1="50" x2="800" y2="50"/><line x1="0" y1="100" x2="800" y2="100"/><line x1="0" y1="150" x2="800" y2="150"/>
        </g>
        <g stroke="#26262b" stroke-width="1">
          <line x1="200" y1="0" x2="200" y2="200"/><line x1="400" y1="0" x2="400" y2="200"/><line x1="600" y1="0" x2="600" y2="200"/>
        </g>
        <polyline points="__GHOST__" fill="none" stroke="#4a3f31" stroke-width="1.1" stroke-dasharray="3 4"/>
        <polygon points="__AREA__" fill="url(#fill)"/>
        <polyline points="__RUMBLE__" fill="none" stroke="#ff7a18" stroke-width="2" stroke-linejoin="round"/>
        <polyline points="__KICK__" fill="none" stroke="#e9e7e1" stroke-width="1.5" stroke-linejoin="round" opacity=".82"/>
      </svg>
      <div style="position:absolute;left:11px;top:9px;font:400 8px/1 'IBM Plex Mono',monospace;letter-spacing:.19em;color:#6a675f">RUMBLE ENVELOPE &#183; 1 BAR</div>
      <div style="position:absolute;right:11px;top:9px;display:flex;gap:13px;font:400 8px/1 'IBM Plex Mono',monospace;letter-spacing:.11em">
        <span style="color:#ff7a18">&#9473; OUT</span><span style="color:#6a5c48">&#9476; RAW TAIL</span><span style="color:#8e8b85">&#9473; KICK</span>
      </div>
      <div style="position:absolute;left:11px;bottom:8px;display:flex;gap:1px">
        <span style="font:400 8px/1 'IBM Plex Mono',monospace;color:#4f4c46">1&#8226;2&#8226;3&#8226;4</span>
      </div>
    </div>

    <div style="width:146px;display:flex;flex-direction:column;gap:10px">
      <div style="border-radius:4px;background:#0e0e10;border:1px solid #96938b;box-shadow:inset 0 2px 9px rgba(0,0,0,.75),0 1px 0 rgba(255,255,255,.4);padding:11px 13px 12px">
        <div style="font:400 8px/1 'IBM Plex Mono',monospace;letter-spacing:.2em;color:#6a675f;margin-bottom:7px">GAIN RED.</div>
        <div style="display:flex;align-items:baseline;gap:4px">__LED1__<span style="font:400 9px/1 'IBM Plex Mono',monospace;color:#8a5a2c">dB</span></div>
        <div style="margin-top:9px;height:4px;border-radius:2px;background:#1e1e22;overflow:hidden">
          <div style="width:64%;height:100%;background:#ff7a18;box-shadow:0 0 7px rgba(255,122,24,.75)"></div>
        </div>
      </div>
      <div style="border-radius:4px;background:#0e0e10;border:1px solid #96938b;box-shadow:inset 0 2px 9px rgba(0,0,0,.75),0 1px 0 rgba(255,255,255,.4);padding:11px 13px 12px">
        <div style="font:400 8px/1 'IBM Plex Mono',monospace;letter-spacing:.2em;color:#6a675f;margin-bottom:7px">KICK DETECTED</div>
        <div style="display:flex;align-items:baseline;gap:4px">__LED2__<span style="font:400 9px/1 'IBM Plex Mono',monospace;color:#8a5a2c">Hz</span></div>
        <div style="margin-top:9px;display:flex;gap:3px">__DOTS__</div>
      </div>
    </div>
  </div>

  <div style="display:flex;justify-content:space-between;padding:16px 42px 0;position:relative">__KNOBS__</div>

  <div style="margin-top:auto;display:flex;justify-content:space-between;align-items:center;padding:11px 24px;border-top:1px solid #b5b2aa;box-shadow:inset 0 1px 0 rgba(255,255,255,.5);font:400 8px/1 'IBM Plex Mono',monospace;letter-spacing:.15em;color:#827f77;position:relative">
    <span>MONO &lt; 120 HZ &#183; SUB CUT 25 HZ &#183; DUCK ATK 2 MS</span>
    <span>IR &#183; CONCRETE HALL B</span>
  </div>
</div>'''

dots = "".join('<div style="width:5px;height:5px;border-radius:50%%;background:%s;%s"></div>'
               % ("#ff7a18" if i < 4 else "#2a2a2e",
                  "box-shadow:0 0 6px rgba(255,122,24,.7)" if i < 4 else "")
               for i in range(12))

body = (BODY.replace("__DEFS__", DEFS)
        .replace("__X1__", cross("left:9px;top:9px")).replace("__X2__", cross("right:9px;top:9px"))
        .replace("__X3__", cross("left:9px;bottom:9px")).replace("__X4__", cross("right:9px;bottom:9px"))
        .replace("__GHOST__", C["ghost_line"]).replace("__AREA__", C["rumble_area"])
        .replace("__RUMBLE__", C["rumble_line"]).replace("__KICK__", C["kick_line"])
        .replace("__LED1__", led("-6.4")).replace("__LED2__", led("52"))
        .replace("__DOTS__", dots).replace("__KNOBS__", knobs))

HTML = '''<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <script src="./support.js"></script>
</head>
<body>
<x-dc>
<helmet>
  <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Archivo:wght@400;600;700&family=IBM+Plex+Mono:wght@400;500;600&display=swap">
  <style>
    body { margin: 0; }
    a { color: #c2560c; } a:hover { color: #ff7a18; }
  </style>
</helmet>
''' + body + '''</x-dc>
<script data-dc-script data-props='{"$preview":{"width":900,"height":520}}'>
class Component extends DCLogic {}
</script>
</body>
</html>
'''
open("Main.dc.html", "w").write(HTML)
print("Main.dc.html written,", len(HTML), "bytes")
