import math, json

W, H = 800.0, 200.0
BEATS = 4
N = 800
KICKS = [i / BEATS for i in range(BEATS)]

DUCK_DEPTH   = 0.88   # how far the duck pulls the rumble down at each hit
DUCK_RELEASE = 0.115  # seconds-ish, in normalised bar units
TAIL_DECAY   = 0.42   # convolution tail decay

def since_kick(t):
    prev = max([k for k in KICKS if k <= t], default=KICKS[0] - 1.0 / BEATS)
    return t - prev

def raw_tail(t):
    """Sum of decaying convolution tails triggered at each kick."""
    v = 0.0
    for k in KICKS:
        for rep in (0, -1):          # include the previous bar's spill
            d = t - (k + rep)
            if d >= 0:
                v += math.exp(-d / TAIL_DECAY) * (1 - math.exp(-d / 0.02))
    return v

def duck_gain(t):
    d = since_kick(t)
    return 1.0 - DUCK_DEPTH * math.exp(-d / DUCK_RELEASE)

def kick_spike(t):
    d = since_kick(t)
    return math.exp(-d / 0.006) * (1 - math.exp(-d / 0.0008))

raw  = [raw_tail(i / (N - 1)) for i in range(N)]
peak = max(raw) or 1.0
raw  = [v / peak for v in raw]
duck = [duck_gain(i / (N - 1)) for i in range(N)]
out  = [raw[i] * duck[i] for i in range(N)]
op   = max(out) or 1.0
out  = [v / op for v in out]
kick = [kick_spike(i / (N - 1)) for i in range(N)]

def pts(series, height, baseline=None, scale=1.0):
    base = H if baseline is None else baseline
    return " ".join(
        "%.1f,%.1f" % (i / (N - 1) * W, base - v * height * scale)
        for i, v in enumerate(series)
    )

def area(series, height):
    return "0,%.1f " % H + pts(series, height) + " %.1f,%.1f" % (W, H)

data = {
    "W": W, "H": H,
    "rumble_area":  area(out, 150),
    "rumble_line":  pts(out, 150),
    "ghost_line":   pts(raw, 150),          # what it would be without ducking
    "duck_line":    pts(duck, 40, baseline=48),
    "kick_line":    pts(kick, 170),
    "kick_x":       [k * W for k in KICKS],
}
json.dump(data, open("curves.json", "w"))
print("kicks at x:", [round(x) for x in data["kick_x"]])
print("rumble peaks between hits — sample of ducked output every 50th point:")
print("  " + " ".join("%.2f" % v for v in out[::50]))
