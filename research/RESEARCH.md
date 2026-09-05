# Kick rumble — research

Compiled 2026-08-26.

## 1. What "rumble" actually is

A rumble is **not** the kick's own tail. It is a **separate layer** derived from
the kick: a long, heavily filtered, distorted low-frequency wash that sits under
the dry kick and ducks out of its way on every hit. The genre home is techno
(especially hard/rumble techno), where it supplies the sustained low-end bed
between transients.

## 2. The canonical technique (reverb method)

The near-universal recipe across every tutorial source:

```
kick ──┬─────────────────────────────────────────► dry kick out
       │
       └─► [send] ─► REVERB ─► DISTORTION ─► LOWPASS/EQ ─► SIDECHAIN DUCK ─► rumble out
                    3–5 s      heavy         kill >150 Hz    4–8 dB GR,
                    100% wet   saturation    lane 30–200 Hz  keyed off dry kick
```

Concrete values that recur across sources:

| Stage | Value |
|---|---|
| Reverb decay | 3–5 s, 100% wet, large room/hall |
| Rumble band | 30–200 Hz (everything above ~150 Hz removed) |
| Sidechain gain reduction | 4–8 dB |
| Sidechain attack | fast (duck the transient immediately) |
| Sidechain release | sets the "bounce" — the main groove control |
| Bass mono fold | below ~120 Hz |
| Boxiness fix | dip around 200 Hz |
| Kick pitch envelope (for reference) | drops to fundamental over 10–20 ms |
| Kick fundamental | typically 40–80 Hz |

**Order matters.** Distortion *after* reverb is what makes it read as techno
rather than generic ambience — saturation adds harmonics that keep the tail
audible on small speakers where the sub is absent.

## 3. Alternative generation methods

Rumble Forge ships nine parallel engines, which is a useful map of the space:

| Method | Character |
|---|---|
| Reverb / reverse reverb | The classic; smooth, washy, pre-hit swell |
| Tempo-synced delay + feedback | Rhythmic, pulsing |
| Sub synth (pitch-locked oscillator) | Clean, controlled, tuneable |
| Saturated feedback | Aggressive, tape → wavefolder |
| Granular | Textural cloud |
| Phase / Reese | Wide, detuned movement |
| FM | Inharmonic sidebands, metallic |
| Reverse swell | Builds into the hit |
| Frequency shifter | Inharmonic, single-sideband drift |

A **sub synth** approach differs importantly from reverb: it is *tuned*, so it
can be locked to the track key. Reverb-based rumble inherits whatever the kick's
fundamental is and is harder to pitch.

## 4. Existing plugins (the market is crowded)

| Plugin | Price | Position |
|---|---|---|
| **Rumble Forge** (Kreuzberg Audio) | €49 | Deep. 9 parallel engines, YIN pitch detection, kick detector, MIDI learn, 51 presets, A/B, channel strip |
| **Seaquake** (CodWaves) | €9.99 | Minimal. 5 controls: Length, Delay, Dynamics, Distortion, Colour |
| **Rumble** (Soundspear) | — | Notch filters on spurious low-end + tuned reverbs |
| **T-Rumble** (Q-Audio) | — | Windows only, instrument not effect |
| **KFXT-41** (G-Sonique) | — | Kick processor: body, saturation, resonant tail |
| **kikzilla** (ISM) | — | Simple rumble creation |
| **TKT-Rumbler** (THINKTONEZ) | Free | Basic rumble generator |

**Two viable poles**, and the middle is the dead zone:

- **Minimal** — 5 knobs, €10, "one good sound, fast." Seaquake owns this.
- **Deep** — many engines, €49, sound-design tool. Rumble Forge owns this.

Differentiation will have to come from something other than feature count.
Candidates: a genuinely better *default* sound, visual feedback nobody else has,
tempo/key awareness, or a workflow angle (e.g. drag-out audio, per-hit variation).

## 5. Technical requirements implied

Whatever the generation method, an effect plugin on a kick track needs:

1. **Kick detection** — amplitude envelope weighted to low-end energy so hats and
   percussion bleed do not false-trigger. Rumble Forge does exactly this.
2. **Pitch detection** (optional) — YIN over ~40–80 Hz to track the fundamental,
   so the rumble locks to the kick without manual tuning.
3. **Internal sidechain** — the duck is keyed from the *input* kick, so no
   external routing is needed. This is the main usability win over doing it by
   hand with sends.
4. **Latency discipline** — reverse/pre-swell modes need lookahead, which means
   reported latency. Everything else should be zero-latency.
5. **Bass mono + sub cut** — non-negotiable for club low end.

## 6. Likely parameter set

Grouped by how often they appear across the plugins surveyed:

**Core (everyone has these)**
- Amount / Dry-Wet
- Length / Decay
- Tone / Colour (lowpass cutoff)
- Drive / Distortion

**Groove**
- Sidechain amount (target 4–8 dB GR)
- Sidechain release (the "bounce")
- Release curve shape
- Trigger: kick-detected vs tempo grid (1/4, 1/8, 1/16, triplets)
- Delay / phase offset

**Low-end hygiene**
- Bass mono frequency (~120 Hz)
- Sub cut (HPF, remove inaudible subsonics)
- Stereo width above the mono point

**Tuning**
- Auto-detect fundamental vs manual pitch
- Key/note lock

## Sources

- [MusicRadar — Create a rumbling techno kick in 10 easy steps](https://www.musicradar.com/how-to/rumbling-techno-kick)
- [The Producer School — How to Create Techno Rumble Kicks for 2025](https://theproducerschool.com/blogs/featured-blogs/how-to-create-techno-rumble-kicks-for-2025)
- [mastrng — Techno Rumble: How to make a Techno Kick that slams](https://www.mastrng.com/techno-rumble/)
- [Studio Brootle — Making A Techno Rumble Kick In Ableton Live](https://www.studiobrootle.com/making-a-techno-rumble-kick-in-ableton-live-step-by-step/)
- [IQSounds — Techno Rumble Kick Processing Guide](https://iqsounds.com/blogs/news/techno-rumble-kick-processing-guide)
- [Attack Magazine — Dark Techno Rumble](https://www.attackmagazine.com/technique/beat-dissected/dark-techno-rumble/)
- [KVR — Rumble Forge by Kreuzberg Audio](https://www.kvraudio.com/product/rumble-forge-by-kreuzberg-audio)
- [CodWaves — Seaquake](https://codwaves.com/seaquake/)
- [KVR — Rumble by Soundspear](https://www.kvraudio.com/product/rumble---techno-kick-generator-by-soundspear)
- [KVR — T-Rumble by Q-Audio](https://www.kvraudio.com/product/t-rumble-by-q-audio)
- [G-Sonique — KFXT-41](https://www.g-sonique.com/kfxt-41-techno-kickdrum-processor-vst-plugin)
- [KVR — kikzilla by ISM](https://www.kvraudio.com/product/kikzilla-by-intelligent-sounds-and-music-ism)
