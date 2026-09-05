# Rumble — brief

Established by interview, 2026-08-26.

## Key facts

- **Personal tool.** Not a product. No differentiation, presets, installers, or
  support burden. The bar is "good enough that I'd reach for it on a real track",
  not "better than Seaquake".
- **Building it is the point.** Hasn't tried the existing plugins. This means
  shipping beats optimising, and scope discipline matters more than features.
- **Serum project paused.** `serum-core` is finished and tested; it stays as-is.
  The JUCE shell gets built here first, and Serum can inherit it later.
- **Effect plugin**, inserted on the kick track. Input is the kick; output is
  kick + rumble.

## Decisions

| Area | Decision |
|---|---|
| Engine | **Reverb-based** — the classic technique |
| Reverb | **Convolution with an impulse response**, not a hand-written algorithm |
| Ducking | **Internal**, keyed off the detected input kick |
| Parameters | **Five knobs**, nothing else |
| Visualizer | **Rumble envelope over time** — functional, not decorative |
| Formats | VST3 + AU + standalone from one JUCE build (assumed) |
| Host | Ableton Live, macOS (assumed) |

## The five controls

| Knob | Range | Does |
|---|---|---|
| `AMOUNT` | 0–100% | How much rumble is mixed under the dry kick |
| `LENGTH` | short–long | Tail duration (IR decay envelope / time-scaling) |
| `TONE` | 40–300 Hz | Lowpass cutoff — the darkness of the rumble |
| `DRIVE` | 0–100% | Saturation after the reverb. What makes it read as techno |
| `DUCK` | 0–100% | Sidechain depth and release, one control |

Baked-in defaults, not exposed:
- bass mono below ~120 Hz
- sub cut (HPF) below ~25 Hz
- fast duck attack
- reverb 100% wet inside the rumble path

## Signal flow

```
input kick ──┬─────────────────────────────────────────────► dry out
             │
             ├─► kick detector (low-weighted envelope follower)
             │        │
             │        └────────────────────────┐  duck key
             │                                 ▼
             └─► convolution (IR) ─► drive ─► LPF ─► duck ─► mono/subcut ─► rumble out
                     LENGTH          DRIVE    TONE   DUCK                    AMOUNT
```

## Risk register

| Risk | Status | Mitigation |
|---|---|---|
| **Reverb sounds bad** — user's own pre-mortem, rated most likely | Largely neutralised | Convolution *is* a real space. Plus the rumble is lowpassed below ~150 Hz, so metallic/comb artifacts (which live in the high mids) get deleted anyway. The bar is far lower than for a general-purpose reverb. |
| JUCE/CMake setup eats the time | Open | Get an audible pass-through plugin loading in Ableton before writing any DSP |
| Scope creep | Controlled | Five knobs is a hard ceiling for v1 |
| Convolution latency | Open | Long IRs need partitioned convolution; `juce::dsp::Convolution` handles it, but confirm reported latency is acceptable |
| IR licensing | Noted | Serum's 109 factory impulses are fine for personal testing. Would need replacing if this ever shipped. |

## Assumptions (unverified)

- Karim makes techno where a rumble belongs
- He can judge "good rumble" by ear well enough to know when it's done
- macOS + Ableton is the only target that matters
- One IR baked in is enough for v1 (no IR browser)

## Open questions

- ~~Which impulse response to ship as the default~~ — **resolved**: the IR is
  generated procedurally, so there is no file to choose and no licensing
  question. See README.
- ~~Whether `LENGTH` scales the IR or envelopes it~~ — **resolved**: LENGTH sets
  the decay time and the IR is regenerated.
- Whether `DUCK` folding depth and release into one knob is enough control —
  still open, needs use on a real track.
- Duck release is currently fixed at 90 ms. That is the "bounce" control the
  research called the main groove lever, so it may need exposing.

## Next step

Design the interface — five knobs plus a live rumble-envelope display, taking
visual cues from the FRACTION and prisma references.
