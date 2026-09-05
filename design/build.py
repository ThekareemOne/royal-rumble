import json, math

C = json.load(open("curves.json"))

def knob(cx, cy, value, r=26, track="#2a2a2e", fill="#e0662a", ind="#fff", sw=3):
    circ = 2 * math.pi * r
    sweep = 0.75 * circ
    th = math.radians(135 + value * 270)
    x1, y1 = 13 * math.cos(th), 13 * math.sin(th)
    x2, y2 = (r - 4) * math.cos(th), (r - 4) * math.sin(th)
    return (
        '<g transform="translate(%g,%g)">'
        '<circle r="%g" fill="none" stroke="%s" stroke-width="%g" stroke-linecap="round" '
        'stroke-dasharray="%.2f %.2f" transform="rotate(135)"/>'
        '<circle r="%g" fill="none" stroke="%s" stroke-width="%g" stroke-linecap="round" '
        'stroke-dasharray="%.2f %.2f" transform="rotate(135)"/>'
        '<line x1="%.2f" y1="%.2f" x2="%.2f" y2="%.2f" stroke="%s" stroke-width="2.5" stroke-linecap="round"/>'
        '</g>'
    ) % (cx, cy, r, track, sw, sweep, circ, r, fill, sw, value * sweep, circ,
         x1, y1, x2, y2, ind)

def knobrow(values, labels, readouts, y, cxs, **kw):
    out = []
    for v, lab, ro, cx in zip(values, labels, readouts, cxs):
        out.append(knob(cx, y, v, **kw))
    return "".join(out)

VALUES   = [0.62, 0.48, 0.35, 0.55, 0.70]
LABELS   = ["AMOUNT", "LENGTH", "TONE", "DRIVE", "DUCK"]
READOUTS = ["62%", "1.8 s", "128 Hz", "55%", "70%"]
CXS      = [110, 285, 460, 635, 810]

HEAD = '''<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <script src="./support.js"></script>
</head>
<body>
<x-dc>
<helmet>
  <link rel="stylesheet" href="__FONTS__">
  <style>
    body { margin: 0; }
    a { color: __LINK__; } a:hover { color: __LINKH__; }
  </style>
</helmet>
'''

TAIL = '''</x-dc>
<script data-dc-script data-props='{"$preview":{"width":900,"height":520}}'>
class Component extends DCLogic {}
</script>
</body>
</html>
'''

def page(fonts, link, linkh, body):
    return HEAD.replace("__FONTS__", fonts).replace("__LINK__", link).replace("__LINKH__", linkh) + body + TAIL

# ---------------------------------------------------------------- direction A
FONTS_A = "https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;700&family=IBM+Plex+Mono:wght@400;500&display=swap"

kn_a = knobrow(VALUES, LABELS, READOUTS, 0, [0]*5)
knobs_a = ""
for v, lab, ro, cx in zip(VALUES, LABELS, READOUTS, CXS):
    knobs_a += (
        '<div style="display:flex;flex-direction:column;align-items:center;gap:10px">'
        '<svg width="72" height="72" viewBox="-36 -36 72 72" style="overflow:visible">%s</svg>'
        '<div style="font:500 11px/1 \'IBM Plex Mono\',monospace;letter-spacing:.14em;color:#6b6b73">%s</div>'
        '<div style="font:500 15px/1 \'Space Grotesk\',sans-serif;color:#e8e4dd">%s</div>'
        '</div>'
    ) % (knob(0, 0, v, track="#26262b", fill="#e0662a"), lab.lower(), ro)

body_a = '''<div style="width:900px;height:520px;background:radial-gradient(120% 90% at 50% -10%, #1c1a19 0%, #0d0c0c 55%, #070707 100%);font-family:'Space Grotesk',system-ui,sans-serif;display:flex;flex-direction:column;overflow:hidden">

  <div style="display:flex;align-items:center;justify-content:space-between;padding:20px 28px 0">
    <div style="display:flex;align-items:baseline;gap:12px">
      <div style="font:700 21px/1 'Space Grotesk',sans-serif;letter-spacing:-.02em;color:#f2ede6">rumble</div>
      <div style="font:400 10px/1 'IBM Plex Mono',monospace;letter-spacing:.18em;color:#5a5a62">V0.1</div>
    </div>
    <div style="display:flex;align-items:center;gap:22px">
      <div style="font:400 11px/1 'IBM Plex Mono',monospace;letter-spacing:.1em;color:#6b6b73">warehouse&nbsp;01</div>
      <div style="width:34px;height:18px;border-radius:9px;background:#e0662a;position:relative;box-shadow:0 0 16px rgba(224,102,42,.45)">
        <div style="position:absolute;right:2px;top:2px;width:14px;height:14px;border-radius:50%;background:#0d0c0c"></div>
      </div>
    </div>
  </div>

  <div style="margin:18px 28px 0;flex-grow:1;border-radius:10px;background:linear-gradient(180deg,#100f0f 0%,#0a0909 100%);border:1px solid #1e1c1b;position:relative;overflow:hidden">
    <svg viewBox="0 0 800 200" preserveAspectRatio="none" style="position:absolute;inset:0;width:100%;height:100%">
      <defs>
        <linearGradient id="rg" x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stop-color="#e0662a" stop-opacity=".42"/>
          <stop offset="100%" stop-color="#e0662a" stop-opacity=".02"/>
        </linearGradient>
      </defs>
      <g stroke="#1a1817" stroke-width="1">
        <line x1="0" y1="50" x2="800" y2="50"/><line x1="0" y1="100" x2="800" y2="100"/><line x1="0" y1="150" x2="800" y2="150"/>
      </g>
      <polyline points="__GHOST__" fill="none" stroke="#3a3430" stroke-width="1.2" stroke-dasharray="3 4"/>
      <polygon points="__AREA__" fill="url(#rg)"/>
      <polyline points="__RUMBLE__" fill="none" stroke="#e0662a" stroke-width="2.2" stroke-linejoin="round"/>
      <polyline points="__KICK__" fill="none" stroke="#f2ede6" stroke-width="1.6" stroke-linejoin="round" opacity=".85"/>
    </svg>
    <div style="position:absolute;left:14px;top:12px;font:400 10px/1 'IBM Plex Mono',monospace;letter-spacing:.16em;color:#55504b">RUMBLE ENVELOPE</div>
    <div style="position:absolute;right:14px;top:12px;display:flex;gap:16px;font:400 10px/1 'IBM Plex Mono',monospace;letter-spacing:.1em">
      <span style="color:#e0662a">&#9473;&nbsp;ducked</span>
      <span style="color:#55504b">&#9476;&nbsp;raw&nbsp;tail</span>
      <span style="color:#8d8880">&#9473;&nbsp;kick</span>
    </div>
  </div>

  <div style="display:flex;justify-content:space-between;padding:26px 44px 0">__KNOBS__</div>

  <div style="display:flex;justify-content:space-between;align-items:center;padding:20px 28px 18px;font:400 10px/1 'IBM Plex Mono',monospace;letter-spacing:.12em;color:#4d4d55">
    <span>mono &lt; 120 hz &nbsp;&#183;&nbsp; sub cut 25 hz</span>
    <span>ir: concrete hall b</span>
  </div>
</div>'''

body_a = (body_a.replace("__GHOST__", C["ghost_line"]).replace("__AREA__", C["rumble_area"])
          .replace("__RUMBLE__", C["rumble_line"]).replace("__KICK__", C["kick_line"])
          .replace("__KNOBS__", knobs_a))
open("Main.dc.html", "w").write(page(FONTS_A, "#e0662a", "#f08040", body_a))
print("Main.dc.html", len(body_a))
