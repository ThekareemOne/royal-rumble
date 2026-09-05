# Royal Rumble

Techno kick-rumble plugin. Insert it on a kick track: it detects each hit, builds
a convolution-reverb tail from the kick itself, ducks that tail out of the way on
every transient, and mixes it back underneath.

**[Download](https://github.com/ThekareemOne/royal-rumble/releases/latest)** &nbsp;·&nbsp;
**[royalrumble.bykarimhisham.com](https://royalrumble.bykarimhisham.com)**

VST3 + AU + Standalone. macOS (universal) and Windows (x64). Built with JUCE 8.

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

CMake fetches JUCE 8.0.4 automatically; if a `JUCE/` checkout already exists it
uses that instead, which is faster for local iteration. Builds install into the
user plug-in folders by default — pass `-DRR_INSTALL_AFTER_BUILD=OFF` to skip
that (CI does).

macOS builds are universal by default. For quicker local builds:
`-DCMAKE_OSX_ARCHITECTURES=arm64`.

> **Name:** "Royal Rumble" is a live WWE trademark. Fine for a personal tool,
> not a name this could be released under. The artwork uses no WWE assets — the
> theme is built from generic royal/championship vocabulary — so only the
> wordmark would need to change if it ever shipped.

## Controls

Eight knobs in four functional groups, sized in three tiers so the ones you
reach for read first.

| Knob | Group | Tier | Range | Default | Does |
|---|---|---|---|---|---|
| `LENGTH` | TAIL | large | 0.15–6 s | 1.8 s | Tail duration — regenerates the impulse response |
| `SPACE` | TAIL | small | 0–100 | 50 | Room character: tight and boxy → large and diffuse |
| `TONE` | FILTER | medium | 40–300 Hz | 128 Hz | Lowpass cutoff, the darkness of the tail |
| `DRIVE` | FILTER | medium | 0–100% | 55% | Saturation after the reverb |
| `HPF` | FILTER | small | 20–120 Hz | 30 Hz | Highpass — stops the rumble stacking under the kick's own sub |
| `DUCK` | DUCK | large | 0–100% | 70% | Sidechain depth, keyed off the detected kick |
| `RELEASE` | DUCK | medium | 10–500 ms | 90 ms | How fast the rumble swells back — the groove control |
| `AMOUNT` | OUTPUT | large | 0–100% | 62% | Rumble mixed under the dry kick |

Plus **SOLO**, which mutes the dry path so the rumble can be dialled in alone.

Fixed, not exposed: the rumble path is mono, duck attack is 1 ms.

## Signal flow

```
in ──┬──────────────────────────────────────────────────────► dry
     │
     ├─► lowpass 200Hz ─► envelope follower ──────┐  duck key
     │                                            ▼
     └─► convolution ─► drive ─► tone LPF ─► duck ─► sub cut ─► × AMOUNT ─► rumble
            LENGTH       DRIVE     TONE      DUCK
```

## The impulse response is generated, not sampled

`RumbleEngine::generateImpulseResponse` synthesises it: exponentially decaying
noise (−60 dB over LENGTH), progressively darkened by a one-pole whose cutoff
falls with the envelope, plus nine sparse early reflections in the first 90 ms.

This was a deliberate choice over shipping a sampled IR. No licensing, no file
loading, no asset to install — and since the rumble is lowpassed below 300 Hz
anyway, the artifacts that distinguish a synthetic IR from a real space live in
a band that gets deleted. It also makes LENGTH natural: a different decay time
is simply a different IR.

## Gotchas found while building

- **JUCE swaps a newly loaded IR in from inside `process()`, not from
  `loadImpulseResponse()`.** Before any block is processed `getCurrentIRSize()`
  reports 1 sample and the convolution is a passthrough. The offline harness
  pumps silent blocks until the IR goes live; the plugin reaches the same state
  within a few blocks of a LENGTH change.
- **Normalise a long IR by energy, not by peak.** An 87k-sample decaying-noise
  IR has a modest peak but enormous total energy; peak-normalising made the
  convolution roughly 100× too loud (rumble at 98× the dry level, peaking above
  full scale). `Convolution::Normalise::yes` fixes it.
- **LENGTH changes allocate**, so the rebuild is queued to the message thread
  via `AsyncUpdater` rather than done in `processBlock`.

## Verifying

`auval -v aufx RyRm Khsm` validates the AU end to end (instantiate, render,
parameters, state) — currently passes.

The offline harness renders a kick pattern through the engine and measures what
the rumble did:

```sh
cmake --build build --target RoyalRumbleTest -j8
cd test && ../build/RoyalRumbleTest_artefacts/Release/RoyalRumbleTest
```

Current output on a 130 BPM two-bar pattern:

| Measure | Value |
|---|---|
| IR loaded | 86,975 samples (1.81 s) |
| Detected kick pitch | 50.0 Hz (source is 52 Hz) |
| Ducking at the hit | −7.0 dB |
| Rumble vs dry, at the hit | 0.10× — out of the transient's way |
| Rumble vs dry, before next hit | 21× — filling the gap |
| Peak output | 0.90 — no clipping |

It also writes `kick_dry.wav` and `kick_rumble.wav` for listening.

## Licence

**AGPLv3.** JUCE 8 is dual-licensed AGPLv3 or commercial, and this project takes
the copyleft option — which is what permits building with the JUCE splash screen
disabled. Embedded fonts are SIL OFL 1.1. See [THIRD-PARTY.md](THIRD-PARTY.md).

## Landing page

`docs/` is a static site deployed to Netlify (`netlify.toml` sets the publish
directory; there is no build step). The hero faceplate is generated from the
same theme source as `design/Main.dc.html`, so the page cannot drift from the
plugin — regenerate both with:

```sh
cd design && python3 build_royal.py && python3 build_landing.py
```

`REPO` and `SITE` at the top of `design/build_landing.py` are the only two
values to change if the repo or domain moves.

## Design

Light industrial chassis — warm grey plate, dark inset screens, orange accent.
Palette lives in `Source/RoyalLookAndFeel.h`, mirrored by `design/Main.dc.html`.

Typefaces are embedded in the binary: **Chakra Petch** (Bold for the wordmark,
SemiBold for labels, Medium for body) and **JetBrains Mono** for anything
numeric.

Knobs are sized in three tiers so the hierarchy reads before you can read the
labels: 116 px for `LENGTH` / `DUCK` / `AMOUNT`, 86 px for `TONE` / `DRIVE` /
`RELEASE`, 66 px and dimmed for `SPACE` / `HPF`. Caption and value baselines are
shared across the whole row so the labels line up despite the size differences.

One fix worth recording: `DRIVE` at 0 used to apply a fixed 1.31× boost and a
little saturation, because the waveshaper was normalised by `tanh(pre)`. It is
now normalised by `tanh'(0) = 1`, so zero drive is genuinely transparent.
