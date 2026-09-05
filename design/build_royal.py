import json, math
C = json.load(open("curves.json"))

# The eight controls as actually implemented, in their four functional groups.
KNOBS = [
    ("SPACE",   "50",     "u", 0), ("LENGTH",  "1.8 s",  "p", 0),
    ("HPF",     "30 Hz",  "u", 1), ("TONE",    "128 Hz", "s", 1), ("DRIVE", "55%", "s", 1),
    ("DUCK",    "70%",    "p", 2), ("RELEASE", "90 ms",  "s", 2),
    ("AMOUNT",  "62%",    "p", 3),
]
VALS   = {"SPACE":0.50,"LENGTH":0.48,"HPF":0.18,"TONE":0.35,"DRIVE":0.55,
          "DUCK":0.70,"RELEASE":0.30,"AMOUNT":0.62}
GROUPS = ["TAIL", "FILTER", "DUCK", "OUTPUT"]
DIA    = {"p":116, "s":86, "u":66}

def knob(v, d, track, fill, cap, ind, ring):
    r = d/2 - 9
    circ, sweep = 2*math.pi*r, 0.75*2*math.pi*r
    th = math.radians(135 + v*270)
    ticks = "".join(
        '<line x1="%.2f" y1="%.2f" x2="%.2f" y2="%.2f" stroke="%s" stroke-width="%s"/>' % (
            (r+5)*math.cos(a), (r+5)*math.sin(a),
            (r+(8 if i%5==0 else 6.5))*math.cos(a), (r+(8 if i%5==0 else 6.5))*math.sin(a),
            ring, "1.3" if i%5==0 else "0.9")
        for i,a in ((i, math.radians(135+i/10*270)) for i in range(11)))
    return ('<svg width="%d" height="%d" viewBox="%g %g %d %d" style="overflow:visible;display:block">'
            '%s'
            '<circle r="%g" fill="none" stroke="%s" stroke-width="2.4" stroke-linecap="round" stroke-dasharray="%.2f %.2f" transform="rotate(135)"/>'
            '<circle r="%g" fill="none" stroke="%s" stroke-width="2.4" stroke-linecap="round" stroke-dasharray="%.2f %.2f" transform="rotate(135)"/>'
            '<circle r="%g" fill="%s" stroke="%s" stroke-width="1"/>'
            '<line x1="%.2f" y1="%.2f" x2="%.2f" y2="%.2f" stroke="%s" stroke-width="2.3" stroke-linecap="round"/>'
            '</svg>') % (
        d, d, -d/2, -d/2, d, d, ticks,
        r, track, sweep, circ, r, fill, v*sweep, circ,
        r-5, cap, ring,
        (r-14)*math.cos(th), (r-14)*math.sin(th), (r-8)*math.cos(th), (r-8)*math.sin(th), ind)

def knobrow(T):
    out, last = [], -1
    for name, val, tier, grp in KNOBS:
        if grp != last:
            if last >= 0: out.append('<div style="width:18px"></div>')
            last = grp
        d = DIA[tier]
        cap_sz = {"p":13.5,"s":12,"u":11}[tier]
        val_sz = {"p":15.5,"s":13.5,"u":12}[tier]
        cap_col = {"p":T["capP"],"s":T["capS"],"u":T["capU"]}[tier]
        out.append(
          '<div style="display:flex;flex-direction:column;align-items:center;width:%dpx">'
          '<div style="height:%dpx;display:flex;align-items:center">%s</div>'
          '<div style="font:600 %gpx/1 %s;letter-spacing:.16em;color:%s;margin-top:%dpx;white-space:nowrap">%s</div>'
          '<div style="font:500 %gpx/1 %s;color:%s;margin-top:5px;white-space:nowrap">%s</div>'
          '</div>' % (d+10, DIA["p"], knob(VALS[name], d, T["track"], T["fill"], T["cap"], T["ind"], T["ring"]),
                      cap_sz, T["labFont"], cap_col, 8, name, val_sz, T["numFont"], T["valCol"], val))
    return "".join(out)

def grouphdr(T):
    out, widths = [], []
    for g in range(4):
        w = sum(DIA[t]+10 for _,_,t,gg in KNOBS if gg==g)
        widths.append(w)
    for g,w in enumerate(widths):
        out.append('<div style="width:%dpx;display:flex;align-items:center;gap:9px">'
                   '<span style="font:600 10.5px/1 %s;letter-spacing:.2em;color:%s;white-space:nowrap">%s</span>'
                   '<span style="flex-grow:1;height:1px;background:%s"></span></div>' % (
                   w, T["labFont"], T["hdrCol"], GROUPS[g], T["hdrRule"]))
        if g < 3: out.append('<div style="width:18px"></div>')
    return "".join(out)

def scope(T, h=176):
    return ('<svg viewBox="0 0 800 200" preserveAspectRatio="none" style="position:absolute;inset:0;width:100%%;height:100%%">'
      '<defs><linearGradient id="%s" x1="0" y1="0" x2="0" y2="1">'
      '<stop offset="0%%" stop-color="%s" stop-opacity=".34"/>'
      '<stop offset="100%%" stop-color="%s" stop-opacity=".02"/></linearGradient></defs>'
      '<g stroke="%s" stroke-width="1"><line x1="0" y1="50" x2="800" y2="50"/><line x1="0" y1="100" x2="800" y2="100"/><line x1="0" y1="150" x2="800" y2="150"/></g>'
      '<g stroke="%s" stroke-width="1"><line x1="200" y1="0" x2="200" y2="200"/><line x1="400" y1="0" x2="400" y2="200"/><line x1="600" y1="0" x2="600" y2="200"/></g>'
      '<polyline points="%s" fill="none" stroke="%s" stroke-width="1.1" stroke-dasharray="3 4"/>'
      '<polygon points="%s" fill="url(#%s)"/>'
      '<polyline points="%s" fill="none" stroke="%s" stroke-width="2" stroke-linejoin="round"/>'
      '<polyline points="%s" fill="none" stroke="%s" stroke-width="1.4" opacity=".8"/>'
      '</svg>') % (T["gid"], T["fill"], T["fill"], T["grid1"], T["grid2"],
                   C["ghost_line"], T["ghost"], C["rumble_area"], T["gid"],
                   C["rumble_line"], T["fill"], C["kick_line"], T["kickCol"])

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
  <style>body{margin:0}a{color:__LINK__}a:hover{color:__LINKH__}</style>
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
    return HEAD.replace("__FONTS__",fonts).replace("__LINK__",link).replace("__LINKH__",linkh)+body+TAIL

def shell(T, ornament="", plate_extra=""):
    """Common chassis: header, scope + meters + solo, grouped knob row, footer."""
    return '''<div style="width:900px;height:520px;background:%(bg)s;font-family:%(labFont)s;display:flex;flex-direction:column;overflow:hidden;position:relative">
  %(texture)s%(ornament)s%(plate_extra)s

  <div style="display:flex;align-items:center;justify-content:space-between;padding:13px 18px;border-bottom:1px solid %(rule)s;position:relative">
    <div style="display:flex;align-items:baseline;gap:14px">
      <div style="font:%(markWeight)s %(markSize)dpx/1 %(markFont)s;letter-spacing:%(markTrack)s;color:%(markCol)s;%(markExtra)s">ROYAL RUMBLE</div>
      <div style="width:1px;height:14px;background:%(rule)s"></div>
      <div style="font:600 11.5px/1 %(labFont)s;letter-spacing:.18em;color:%(hdrCol)s">KICK TAIL GENERATOR</div>
    </div>
    <div style="display:flex;align-items:center;gap:12px">
      <div style="font:500 11px/1 %(numFont)s;color:%(hdrCol)s">48.0 kHz</div>
      %(crest)s
    </div>
  </div>

  <div style="display:flex;gap:14px;padding:12px 18px 0;position:relative">
    <div style="flex-grow:1;height:214px;border-radius:%(scopeRad)s;background:%(scopeBg)s;border:1px solid %(scopeEdge)s;box-shadow:%(scopeShadow)s;position:relative;overflow:hidden">
      %(scope)s
      <div style="position:absolute;left:11px;top:8px;font:600 10px/1 %(labFont)s;letter-spacing:.18em;color:%(scopeLab)s">RUMBLE ENVELOPE &#183; 2 S</div>
      <div style="position:absolute;right:11px;top:8px;display:flex;gap:12px;font:500 8px/1 %(numFont)s;letter-spacing:.08em">
        <span style="color:%(fill)s">OUT</span><span style="color:%(ghost)s">RAW</span><span style="color:%(kickCol)s">KICK</span>
      </div>
    </div>
    <div style="width:162px;display:flex;flex-direction:column;gap:7px">
      <div style="border-radius:%(scopeRad)s;background:%(scopeBg)s;border:1px solid %(scopeEdge)s;box-shadow:%(scopeShadow)s;padding:9px 12px">
        <div style="font:600 10.5px/1 %(labFont)s;letter-spacing:.18em;color:%(scopeLab)s;margin-bottom:6px">GAIN RED.</div>
        <div style="font:700 27px/1 %(numFont)s;color:%(fill)s;text-shadow:0 0 12px %(glow)s">-6.4<span style="font-size:10.5px;color:%(scopeLab)s;margin-left:4px">dB</span></div>
        <div style="margin-top:7px;height:3px;border-radius:2px;background:%(barBg)s;overflow:hidden"><div style="width:27%%;height:100%%;background:%(fill)s"></div></div>
      </div>
      <div style="border-radius:%(scopeRad)s;background:%(scopeBg)s;border:1px solid %(scopeEdge)s;box-shadow:%(scopeShadow)s;padding:9px 12px">
        <div style="font:600 10.5px/1 %(labFont)s;letter-spacing:.18em;color:%(scopeLab)s;margin-bottom:6px">KICK DETECTED</div>
        <div style="font:700 27px/1 %(numFont)s;color:%(fill)s;text-shadow:0 0 12px %(glow)s">50<span style="font-size:10.5px;color:%(scopeLab)s;margin-left:4px">Hz</span></div>
      </div>
      <div style="height:29px;border-radius:%(scopeRad)s;background:%(soloBg)s;border:1px solid %(soloEdge)s;display:grid;place-items:center;font:700 11px/1 %(labFont)s;letter-spacing:.2em;color:%(soloCol)s">SOLO</div>
    </div>
  </div>

  <div style="display:flex;justify-content:center;padding:13px 18px 0;position:relative">%(hdrs)s</div>
  <div style="display:flex;justify-content:center;padding:3px 18px 0;position:relative">%(knobs)s</div>

  <div style="margin-top:auto;display:flex;justify-content:space-between;align-items:center;padding:8px 18px;border-top:1px solid %(rule)s;font:500 11px/1 %(labFont)s;letter-spacing:.12em;color:%(hdrCol)s;position:relative">
    <span>MONO RUMBLE PATH &#183; DUCK ATTACK 1 MS</span><span>IR &#183; SYNTHESISED</span>
  </div>
</div>''' % dict(T, ornament=ornament, plate_extra=plate_extra,
                 scope=scope(T), hdrs=grouphdr(T), knobs=knobrow(T))

# ---------------------------------------------------------------- A: Championship
A = dict(
  gid="ga", bg="linear-gradient(178deg,#141110 0%,#0b0908 60%,#080606 100%)",
  texture='<div style="position:absolute;inset:0;opacity:.5;pointer-events:none;background-image:radial-gradient(circle at 22% 18%,rgba(255,255,255,.045) 0 1px,transparent 1px),radial-gradient(circle at 68% 72%,rgba(255,255,255,.035) 0 1px,transparent 1px);background-size:7px 7px,11px 11px"></div>',
  rule="#3a2c14", hdrCol="#8a7444", hdrRule="#3a2c14",
  markFont="'Cinzel',serif", markWeight="700", markSize=23, markTrack=".07em",
  markCol="#f2d894",
  markExtra="text-shadow:0 1px 0 #6b5017,0 0 22px rgba(226,187,92,.35);",
  labFont="'Barlow Condensed',sans-serif", numFont="'JetBrains Mono',monospace",
  crest='<svg width="22" height="22" viewBox="0 0 24 24"><path d="M3 9l4 3 5-7 5 7 4-3-2 10H5z" fill="none" stroke="#e2bb5c" stroke-width="1.6" stroke-linejoin="round"/><circle cx="12" cy="19.5" r="1" fill="#e2bb5c"/></svg>',
  scopeRad="3px", scopeBg="#080605", scopeEdge="#6b5017",
  scopeShadow="inset 0 2px 14px rgba(0,0,0,.9),0 0 0 1px rgba(226,187,92,.14)",
  scopeLab="#7a6534", fill="#e2bb5c", glow="rgba(226,187,92,.45)",
  grid1="#1a1409", grid2="#241b0d", ghost="#5a4820", kickCol="#cfc3a4",
  barBg="#221a0c", track="#241b0d", cap="#15110c", ind="#f7e9c4", ring="#6b5017",
  capP="#e2bb5c", capS="#a08a54", capU="#6f5f3a", valCol="#c9b071")
A["soloBg"] = "#1a1207"; A["soloEdge"] = "#6b5017"; A["soloCol"] = "#a08a54"

ORN_A = ('<div style="position:absolute;inset:9px;border:1px solid rgba(226,187,92,.22);pointer-events:none"></div>'
         '<div style="position:absolute;inset:13px;border:1px solid rgba(226,187,92,.10);pointer-events:none"></div>'
         + "".join('<svg width="17" height="17" viewBox="0 0 17 17" style="position:absolute;%s;pointer-events:none">'
                   '<path d="M1 8.5a7.5 7.5 0 0 1 7.5-7.5" fill="none" stroke="rgba(226,187,92,.5)" stroke-width="1.3"/>'
                   '<circle cx="4" cy="4" r="1.2" fill="rgba(226,187,92,.6)"/></svg>' % s
                   for s in ["left:14px;top:14px","right:14px;top:14px;transform:scaleX(-1)",
                             "left:14px;bottom:14px;transform:scaleY(-1)",
                             "right:14px;bottom:14px;transform:scale(-1)"]))

FONTS_A = "https://fonts.googleapis.com/css2?family=Cinzel:wght@600;700&family=Barlow+Condensed:wght@500;600;700&family=JetBrains+Mono:wght@500;700&display=swap"

# ---------------------------------------------------------------- B: Regalia
B = dict(A)
B.update(
  gid="gb",
  bg="linear-gradient(175deg,#12161f 0%,#0c0f16 55%,#080a0f 100%)",
  texture='<div style="position:absolute;inset:0;opacity:.4;pointer-events:none;background-image:repeating-linear-gradient(135deg,rgba(255,255,255,.022) 0 1px,transparent 1px 5px)"></div>',
  rule="#243044", hdrCol="#6d7d96", hdrRule="#243044",
  markFont="'Cormorant Garamond',serif", markWeight="600", markSize=27, markTrack=".05em",
  markCol="#e8d9b0", markExtra="font-style:italic;",
  labFont="'Archivo',sans-serif",
  crest='<svg width="20" height="20" viewBox="0 0 24 24"><path d="M12 2l2.6 6.3 6.4.5-4.9 4.2 1.5 6.5L12 16l-5.6 3.5 1.5-6.5L3 8.8l6.4-.5z" fill="none" stroke="#c9a961" stroke-width="1.4" stroke-linejoin="round"/></svg>',
  scopeBg="#070a0f", scopeEdge="#2e3d55", scopeShadow="inset 0 2px 12px rgba(0,0,0,.85)",
  scopeLab="#5a6a82", fill="#c9a961", glow="rgba(201,169,97,.4)",
  grid1="#131a26", grid2="#1b2432", ghost="#4a4432", kickCol="#b8c4d6",
  barBg="#18202e", track="#1b2432", cap="#0e1219", ind="#f0e6cc", ring="#3a4a63",
  capP="#d8c48a", capS="#8b9bb3", capU="#5a6a82", valCol="#a8b6c9",
  soloBg="#111722", soloEdge="#3a4a63", soloCol="#8b9bb3")
ORN_B = ('<div style="position:absolute;inset:10px;border:1px solid rgba(201,169,97,.20);pointer-events:none"></div>'
         + "".join('<div style="position:absolute;%s;width:26px;height:26px;border-%s:1px solid rgba(201,169,97,.45);border-%s:1px solid rgba(201,169,97,.45);pointer-events:none"></div>' % s
                   for s in [("left:16px;top:16px","left","top"),("right:16px;top:16px","right","top"),
                             ("left:16px;bottom:16px","left","bottom"),("right:16px;bottom:16px","right","bottom")]))
FONTS_B = "https://fonts.googleapis.com/css2?family=Cormorant+Garamond:ital,wght@1,600;1,700&family=Archivo:wght@500;600;700&family=JetBrains+Mono:wght@500;700&display=swap"

# ---------------------------------------------------------------- C: Arena
Cc = dict(A)
Cc.update(
  gid="gc",
  bg="radial-gradient(130% 100% at 50% -15%, #5e1218 0%, #2a0709 45%, #140304 100%)",
  texture='<div style="position:absolute;inset:0;pointer-events:none;background:radial-gradient(80% 55% at 50% 0%,rgba(255,214,140,.16) 0%,transparent 70%)"></div>',
  rule="#5a2020", hdrCol="#c08a72", hdrRule="#5a2020",
  markFont="'Anton',sans-serif", markWeight="400", markSize=26, markTrack=".04em",
  markCol="#ffd98a", markExtra="text-shadow:0 0 26px rgba(255,190,90,.5);",
  labFont="'Barlow Condensed',sans-serif",
  crest='<div style="width:11px;height:11px;border-radius:50%;background:#ffbe5a;box-shadow:0 0 14px rgba(255,190,90,.9)"></div>',
  scopeBg="#150405", scopeEdge="#7a2a26", scopeShadow="inset 0 2px 16px rgba(0,0,0,.9)",
  scopeLab="#a86a58", fill="#ffbe5a", glow="rgba(255,190,90,.55)",
  grid1="#260a0b", grid2="#331012", ghost="#7a4a2e", kickCol="#ffe6c4",
  barBg="#2a0c0d", track="#331012", cap="#1c0708", ind="#fff0d4", ring="#8a3a30",
  capP="#ffd98a", capS="#d99a70", capU="#a86a58", valCol="#e8b48c",
  soloBg="#280a0b", soloEdge="#8a3a30", soloCol="#d99a70")
ORN_C = '<div style="position:absolute;inset:0;pointer-events:none;box-shadow:inset 0 0 120px rgba(0,0,0,.6)"></div>'
FONTS_C = "https://fonts.googleapis.com/css2?family=Anton&family=Barlow+Condensed:wght@500;600;700&family=JetBrains+Mono:wght@500;700&display=swap"


# ---------------------------------------------------------------- Light chassis (shipping)
L = dict(A)
L.update(
  gid="gl",
  bg="linear-gradient(178deg,#e2e0da 0%,#d3d0c9 52%,#c6c3bb 100%)",
  texture='<div style="position:absolute;inset:0;opacity:.55;pointer-events:none;background-image:repeating-linear-gradient(90deg,rgba(0,0,0,.026) 0 1px,transparent 1px 3px)"></div>',
  rule="#b5b2aa", hdrCol="#827f77", hdrRule="#b5b2aa",
  markFont="'Chakra Petch',sans-serif", markWeight="700", markSize=25, markTrack=".01em",
  markCol="#1c1c1e", markExtra="",
  labFont="'Chakra Petch',sans-serif", numFont="'JetBrains Mono',monospace",
  crest='<div style="width:13px;height:13px;border-radius:50%;background:#ff7a18;box-shadow:0 0 11px rgba(255,122,24,.85)"></div>',
  scopeRad="4px", scopeBg="#0e0e10", scopeEdge="#96938b",
  scopeShadow="inset 0 2px 12px rgba(0,0,0,.75),0 1px 0 rgba(255,255,255,.4)",
  scopeLab="#6a675f", fill="#ff7a18", glow="rgba(255,122,24,.5)",
  grid1="#1c1c20", grid2="#26262b", ghost="#4a3f31", kickCol="#e9e7e1",
  barBg="#1e1e22", track="#0f0f11", cap="#2a2a2d", ind="#f4f4f2", ring="#adaaa2",
  capP="#3c3a35", capS="#6d6a63", capU="#8a877f", valCol="#26262a",
  soloBg="#1a1a1d", soloEdge="#96938b", soloCol="#8a8780")
ORN_L = "".join('<svg width="10" height="10" viewBox="0 0 10 10" style="position:absolute;%s;opacity:.32;pointer-events:none">'
                '<path d="M5 0v10M0 5h10" stroke="#5f5c55" stroke-width="1"/></svg>' % s
                for s in ["left:8px;top:8px","right:8px;top:8px","left:8px;bottom:8px","right:8px;bottom:8px"])
FONTS_L = "https://fonts.googleapis.com/css2?family=Chakra+Petch:wght@500;600;700&family=JetBrains+Mono:wght@500;700&display=swap"
if __name__ == "__main__":
    open("Main.dc.html","w").write(page(FONTS_L,"#c2560c","#ff7a18", shell(L, ORN_L)))
    print("Main.dc.html (light chassis)")
