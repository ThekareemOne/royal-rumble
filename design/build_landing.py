"""Landing page for GitHub Pages. The hero faceplate is generated from the same
theme dict as design/Main.dc.html, so the page and the plugin cannot drift."""
import re, pathlib, importlib.util

spec = importlib.util.spec_from_file_location("br", "build_royal.py")
br = importlib.util.module_from_spec(spec)
spec.loader.exec_module(br)          # defines shell(), L, ORN_L

REPO = "__REPO__"                    # e.g. karimhisham/royal-rumble
faceplate = br.shell(br.L, br.ORN_L)

CONTROLS = [
    ("LENGTH",  "TAIL",   "0.15 – 6 s",   "Tail duration. Regenerates the impulse response."),
    ("SPACE",   "TAIL",   "0 – 100",      "Room character, from tight and boxy to large and diffuse."),
    ("TONE",    "FILTER", "40 – 300 Hz",  "Lowpass cutoff — how dark the rumble sits."),
    ("DRIVE",   "FILTER", "0 – 100 %",    "Saturation. Adds the harmonics that keep the tail audible on speakers with no sub."),
    ("HPF",     "FILTER", "20 – 120 Hz",  "Highpass. Stops the rumble stacking under the kick's own fundamental."),
    ("DUCK",    "DUCK",   "0 – 100 %",    "Sidechain depth, keyed off the detected kick. This is what makes it groove instead of drone."),
    ("RELEASE", "DUCK",   "10 – 500 ms",  "How fast the rumble swells back after each hit."),
    ("AMOUNT",  "OUTPUT", "0 – 100 %",    "How much rumble is mixed under the dry kick."),
]

rows = "".join(
    '<tr><td class="k">%s</td><td class="g">%s</td><td class="r">%s</td><td>%s</td></tr>' % c
    for c in CONTROLS)

HTML = '''<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Royal Rumble — kick tail generator</title>
<meta name="description" content="A техно kick-rumble plugin. Detects each kick, generates a convolution reverb tail, ducks it against the kick, and mixes it underneath. VST3 and AU, macOS and Windows.">
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Chakra+Petch:wght@500;600;700&family=JetBrains+Mono:wght@400;500;700&display=swap">
<style>
:root{
  --plate-a:#e2e0da; --plate-b:#d3d0c9; --plate-c:#c6c3bb;
  --rule:#b5b2aa; --ink:#1c1c1e; --label:#6d6a63; --dim:#827f77;
  --screen:#0e0e10; --screen-edge:#96938b; --screen-lab:#6a675f;
  --accent:#ff7a18;
  --sans:'Chakra Petch',system-ui,sans-serif;
  --mono:'JetBrains Mono',ui-monospace,monospace;
}
*{box-sizing:border-box}
body{margin:0;background:linear-gradient(178deg,var(--plate-a) 0%,var(--plate-b) 46%,var(--plate-c) 100%);
     background-attachment:fixed;color:var(--ink);font-family:var(--sans);font-size:16px;line-height:1.6}
body::before{content:'';position:fixed;inset:0;pointer-events:none;z-index:0;
     background-image:repeating-linear-gradient(90deg,rgba(0,0,0,.026) 0 1px,transparent 1px 3px)}
.wrap{position:relative;z-index:1;max-width:980px;margin:0 auto;padding:0 22px}

header{padding:54px 0 30px;border-bottom:1px solid var(--rule);box-shadow:0 1px 0 rgba(255,255,255,.5)}
h1{font-size:clamp(38px,7vw,60px);font-weight:700;letter-spacing:.01em;margin:0;line-height:1}
.strap{font-weight:600;letter-spacing:.2em;font-size:12px;color:var(--dim);margin-top:12px;text-transform:uppercase}
.lede{max-width:60ch;margin:22px 0 0;font-size:17px}
.lede b{font-weight:600}

.dl{display:flex;flex-wrap:wrap;gap:12px;margin:30px 0 4px}
.btn{display:flex;flex-direction:column;gap:2px;padding:13px 22px;border-radius:4px;text-decoration:none;
     border:1px solid #a8a59d;background:linear-gradient(180deg,#cbc8c0,#bcb9b1);color:var(--ink);
     box-shadow:inset 0 1px 0 rgba(255,255,255,.45);transition:transform .08s ease,filter .12s ease}
.btn:hover{filter:brightness(1.05);transform:translateY(-1px)}
.btn strong{font-size:15px;font-weight:700;letter-spacing:.02em}
.btn span{font-family:var(--mono);font-size:11px;color:#55534d}
.btn.primary{background:linear-gradient(180deg,#ff8c33,#f06f0d);border-color:#c2560c;color:#1c1208}
.btn.primary span{color:#5c2f06}
.src{align-self:center;font-family:var(--mono);font-size:12px;color:var(--dim)}
.src a{color:#c2560c}

.plate-frame{width:100%;max-width:900px;margin:38px auto 0;position:relative;
     filter:drop-shadow(0 14px 34px rgba(0,0,0,.28))}
.plate-frame::before{content:'';display:block;padding-top:57.78%}
.plate-inner{position:absolute;inset:0;overflow:hidden}
.plate-inner>div{transform-origin:top left}

section{padding:44px 0;border-bottom:1px solid var(--rule);box-shadow:0 1px 0 rgba(255,255,255,.5)}
h2{font-size:12px;font-weight:600;letter-spacing:.24em;color:var(--dim);margin:0 0 20px;text-transform:uppercase}
h3{font-size:16px;font-weight:600;margin:24px 0 6px}
p{max-width:66ch}

.screen{background:var(--screen);border:1px solid var(--screen-edge);border-radius:4px;
     box-shadow:inset 0 2px 12px rgba(0,0,0,.75),0 1px 0 rgba(255,255,255,.4);padding:20px 22px;overflow-x:auto}
.screen pre{margin:0;font-family:var(--mono);font-size:12.5px;line-height:1.75;color:#c8c4bc}
.screen pre b{color:var(--accent);font-weight:500}
.screen pre i{color:var(--screen-lab);font-style:normal}

table{border-collapse:collapse;width:100%;font-size:14.5px}
th{text-align:left;font-size:10.5px;letter-spacing:.2em;color:var(--dim);font-weight:600;
   padding:0 12px 9px 0;border-bottom:1px solid var(--rule);text-transform:uppercase}
td{padding:11px 12px 11px 0;border-bottom:1px solid rgba(181,178,170,.5);vertical-align:top}
td.k{font-weight:700;letter-spacing:.06em;white-space:nowrap}
td.g{font-family:var(--mono);font-size:11px;color:var(--dim);white-space:nowrap}
td.r{font-family:var(--mono);font-size:12px;color:#55534d;white-space:nowrap}

code{font-family:var(--mono);font-size:.88em;background:rgba(0,0,0,.06);padding:2px 6px;border-radius:3px}
.note{border-left:3px solid var(--accent);padding:2px 0 2px 15px;margin:20px 0;max-width:66ch}
.note b{font-weight:700}
ol{max-width:66ch;padding-left:20px}
li{margin:6px 0}
footer{padding:32px 0 60px;font-family:var(--mono);font-size:12px;color:var(--dim)}
footer a{color:#c2560c}
@media (max-width:640px){ .btn{flex:1 1 100%} }
</style>
</head>
<body>
<div class="wrap">

<header>
  <h1>ROYAL RUMBLE</h1>
  <div class="strap">Kick tail generator &middot; VST3 &amp; AU</div>
  <p class="lede">Insert it on a kick track. It detects every hit, builds a convolution reverb tail
  from the kick itself, <b>ducks that tail out of the way</b> on each transient, and mixes it back
  underneath — the low-frequency bed that sits between kicks in techno.</p>

  <div class="dl">
    <a class="btn primary" href="https://github.com/__REPO__/releases/latest/download/RoyalRumble-macOS.zip">
      <strong>Download for macOS</strong><span>VST3 + AU &middot; universal</span></a>
    <a class="btn" href="https://github.com/__REPO__/releases/latest/download/RoyalRumble-Windows.zip">
      <strong>Download for Windows</strong><span>VST3 &middot; x64</span></a>
    <span class="src"><a href="https://github.com/__REPO__">Source on GitHub</a> &middot;
      <a href="https://github.com/__REPO__/releases">All releases</a></span>
  </div>

  <div class="plate-frame"><div class="plate-inner">__FACEPLATE__</div></div>
</header>

<section>
  <h2>How it works</h2>
  <p>The input splits in two. One branch is lowpassed at 200&nbsp;Hz and rectified to track the kick —
  hats and percussion bleed live above that, so they cannot false-trigger. The other branch runs
  through a convolution, which replaces every input sample with a scaled copy of an entire impulse
  response. One 5&nbsp;ms transient becomes a multi-second wash; four kicks a bar overlap into a
  continuous bed.</p>
  <div class="screen"><pre>in <i>──┬─────────────────────────────────────────────────────►</i> dry
   <i>│</i>
   <i>├─►</i> lowpass 200Hz <i>─►</i> envelope follower <i>─────┐</i>  duck key
   <i>│</i>                                        <i>▼</i>
   <i>└─►</i> convolution <i>─►</i> drive <i>─►</i> tone <i>─►</i> <b>duck</b> <i>─►</i> hpf <i>─►</i> × amount <i>─►</i> rumble</pre></div>
  <h3>The impulse response is generated, not sampled</h3>
  <p>Exponentially decaying noise, progressively darkened by a filter whose cutoff falls as the tail
  dies — which is what makes it read as a large room rather than a burst of static — plus sparse
  early reflections. Nothing is sampled, so there is no asset to install and no licence attached to
  the sound. It also makes <code>LENGTH</code> natural: a different decay is simply a different IR.</p>
</section>

<section>
  <h2>Controls</h2>
  <table>
    <thead><tr><th>Knob</th><th>Group</th><th>Range</th><th></th></tr></thead>
    <tbody>__ROWS__</tbody>
  </table>
  <p style="margin-top:18px"><b>SOLO</b> mutes the dry kick so the rumble can be dialled in on its own.
  Mono rumble path and a 1&nbsp;ms duck attack are fixed.</p>
</section>

<section>
  <h2>Install</h2>
  <h3>macOS</h3>
  <ol>
    <li>Unzip and copy <code>Royal Rumble.vst3</code> to <code>/Library/Audio/Plug-Ins/VST3/</code></li>
    <li>For Logic or GarageBand, also copy <code>Royal Rumble.component</code> to <code>/Library/Audio/Plug-Ins/Components/</code></li>
    <li>Rescan plug-ins in your DAW</li>
  </ol>
  <div class="note"><b>Gatekeeper.</b> These builds are not notarised, so macOS will say the developer
  cannot be verified. Clear the quarantine flag once:
  <div class="screen" style="margin-top:10px"><pre>xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/Royal Rumble.vst3"</pre></div></div>
  <h3>Windows</h3>
  <ol>
    <li>Unzip and copy <code>Royal Rumble.vst3</code> to <code>C:\\Program Files\\Common Files\\VST3\\</code></li>
    <li>Rescan plug-ins in your DAW</li>
  </ol>
</section>

<section>
  <h2>Build from source</h2>
  <p>CMake fetches JUCE automatically — nothing else to install beyond a compiler and CMake 3.22+.</p>
  <div class="screen"><pre>git clone https://github.com/__REPO__.git
cd royal-rumble
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8</pre></div>
  <p style="margin-top:16px">There is also an offline harness that renders a kick pattern through the
  engine and measures what the rumble actually did — ducking depth, tail decay, peak level — plus
  <code>auval</code> for AU validation on macOS.</p>
</section>

<section style="border-bottom:none;box-shadow:none">
  <h2>Licence</h2>
  <p><b>AGPLv3.</b> JUCE is dual-licensed AGPLv3 or commercial, and this project takes the copyleft
  option — which is what permits building without the JUCE splash screen. Embedded fonts (Chakra
  Petch, JetBrains Mono) are SIL Open Font Licence 1.1. Full detail in
  <a href="https://github.com/__REPO__/blob/main/THIRD-PARTY.md">THIRD-PARTY.md</a>.</p>
  <p style="font-size:14px;color:var(--dim)">VST is a trademark of Steinberg Media Technologies GmbH.</p>
</section>

<footer>Built with JUCE &middot; <a href="https://github.com/__REPO__">github.com/__REPO__</a></footer>
</div>

<script>
// Scale the 900px faceplate to whatever width the column actually is.
(function () {
  var frame = document.querySelector('.plate-frame');
  var plate = document.querySelector('.plate-inner > div');
  if (!frame || !plate) return;
  function fit() { plate.style.transform = 'scale(' + (frame.clientWidth / 900) + ')'; }
  fit();
  window.addEventListener('resize', fit);
})();
</script>
</body>
</html>
'''

out = (HTML.replace("__FACEPLATE__", faceplate)
           .replace("__ROWS__", rows)
           .replace("__REPO__", REPO)
           .replace("техно", "techno"))
d = pathlib.Path("../docs"); d.mkdir(exist_ok=True)
(d / "index.html").write_text(out)
print("docs/index.html written, %d bytes" % len(out))
