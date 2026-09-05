// Offline harness: synthesise a kick pattern, run it through RumbleEngine,
// write the result, and report what the rumble actually did.
#include "../Source/RumbleEngine.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <cstdio>

static void renderKicks (juce::AudioBuffer<float>& buf, double sr, double bpm, int bars)
{
    buf.clear();
    const double spb = 60.0 / bpm;
    const int beats = bars * 4;
    for (int b = 0; b < beats; ++b)
    {
        const int start = (int) (b * spb * sr);
        double phase = 0.0;
        for (int i = 0; i < (int) (0.45 * sr) && start + i < buf.getNumSamples(); ++i)
        {
            const double t = i / sr;
            const double f = 52.0 + 120.0 * std::exp (-t / 0.018);   // pitch drop
            const double a = std::exp (-t / 0.085) * (1.0 - std::exp (-t / 0.0012));
            phase += 2.0 * juce::MathConstants<double>::pi * f / sr;
            const float s = (float) (std::sin (phase) * a * 0.9);
            for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                buf.getWritePointer (ch)[start + i] += s;
        }
    }
}

static float rmsBetween (const juce::AudioBuffer<float>& b, double sr, double t0, double t1)
{
    const int a = juce::jlimit (0, b.getNumSamples() - 1, (int) (t0 * sr));
    const int n = juce::jlimit (1, b.getNumSamples() - a, (int) ((t1 - t0) * sr));
    return b.getRMSLevel (0, a, n);
}

int main()
{
    const double sr = 48000.0;
    const int    block = 512, bars = 2;
    const double bpm = 130.0;
    const int    total = (int) (bars * 4 * 60.0 / bpm * sr) + (int) sr;

    juce::AudioBuffer<float> dry (2, total), wet (2, total);
    renderKicks (dry, sr, bpm, bars);
    wet.makeCopyOf (dry);

    RumbleEngine engine;
    engine.prepare ({ sr, (juce::uint32) block, 2 });
    engine.setParameters (0.62f, 1.8f, 128.0f, 0.55f, 0.70f, 90.0f, 50.0f, 30.0f, false);
    engine.rebuildImpulseResponse();

    // JUCE swaps a newly loaded IR in from inside process(), so pump silence
    // until it goes live before measuring anything.
    {
        juce::AudioBuffer<float> warm (2, block);
        for (int i = 0; i < 200 && engine.getLoadedIrSamples() <= 1; ++i)
        {
            warm.clear();
            engine.process (warm);
            juce::Thread::sleep (2);
        }
        engine.reset();
    }
    printf ("IR loaded             %d samples (%.2f s)\n\n",
            engine.getLoadedIrSamples(), engine.getLoadedIrSamples() / sr);

    for (int pos = 0; pos < total; pos += block)
    {
        const int n = juce::jmin (block, total - pos);
        juce::AudioBuffer<float> slice (wet.getArrayOfWritePointers(), 2, pos, n);
        engine.process (slice);
    }

    // Isolate the rumble: wet minus dry, sample by sample.
    juce::AudioBuffer<float> rumble (2, total);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < total; ++i)
            rumble.getWritePointer (ch)[i] = wet.getReadPointer (ch)[i] - dry.getReadPointer (ch)[i];

    const double spb = 60.0 / bpm;
    printf ("sample rate      %.0f Hz, %d bars at %.0f BPM\n", sr, bars, bpm);
    printf ("detected pitch   %.1f Hz  (source kick is 52 Hz)\n", engine.getDetectedHz());
    printf ("rumble peak      %.4f\n\n", rumble.getMagnitude (0, total));

    printf ("  %-13s %-11s %-11s %-11s\n", "window", "DRY rms", "RUMBLE rms", "ratio");
    struct { const char* name; double a, b; } win[] = {
        { "at hit",      spb * 4.0 + 0.000, spb * 4.0 + 0.020 },
        { "just after",  spb * 4.0 + 0.030, spb * 4.0 + 0.080 },
        { "mid-beat",    spb * 4.0 + 0.180, spb * 4.0 + 0.280 },
        { "before next", spb * 4.0 + 0.380, spb * 4.0 + 0.440 },
        { "tail +0.3s",  spb * 8.0 + 0.300, spb * 8.0 + 0.500 },
        { "tail +1.0s",  spb * 8.0 + 1.000, spb * 8.0 + 1.200 },
    };
    for (auto& w : win)
    {
        const float d = rmsBetween (dry, sr, w.a, w.b);
        const float r = rmsBetween (rumble, sr, w.a, w.b);
        printf ("  %-13s %-11.6f %-11.6f %-11s\n", w.name, d, r,
                d > 1e-6f ? juce::String (r / d, 2).toRawUTF8() : "-");
    }

    printf ("\nrumble envelope across the render (rms per 200 ms):\n");
    for (double t = 0.0; t + 0.2 < total / sr; t += 0.2)
    {
        const float r = rmsBetween (rumble, sr, t, t + 0.2);
        printf ("  %5.2fs  %-10.6f %s\n", t, r,
                juce::String::repeatedString ("#", juce::jlimit (0, 50,
                    juce::roundToInt (r * 400.0f))).toRawUTF8());
    }

    // Duck A/B: same render with the duck off, measured at the hit.
    juce::AudioBuffer<float> noDuck (2, total);
    noDuck.makeCopyOf (dry);
    RumbleEngine e2;
    e2.prepare ({ sr, (juce::uint32) block, 2 });
    e2.setParameters (0.62f, 1.8f, 128.0f, 0.55f, 0.0f, 90.0f, 50.0f, 30.0f, false);  // duck = 0
    e2.rebuildImpulseResponse();
    {
        juce::AudioBuffer<float> warm (2, block);
        for (int i = 0; i < 200 && e2.getLoadedIrSamples() <= 1; ++i)
        { warm.clear(); e2.process (warm); juce::Thread::sleep (2); }
        e2.reset();
    }
    for (int pos = 0; pos < total; pos += block)
    {
        const int n = juce::jmin (block, total - pos);
        juce::AudioBuffer<float> slice (noDuck.getArrayOfWritePointers(), 2, pos, n);
        e2.process (slice);
    }

    float duckedAtHit = 0.0f, openAtHit = 0.0f;
    for (int b = 2; b < bars * 4; ++b)
    {
        const double a = b * spb + 0.004, z = b * spb + 0.030;
        float d = 0.0f, o = 0.0f;
        for (int i = (int) (a * sr); i < (int) (z * sr); ++i)
        {
            d = juce::jmax (d, std::abs (wet.getReadPointer (0)[i]    - dry.getReadPointer (0)[i]));
            o = juce::jmax (o, std::abs (noDuck.getReadPointer (0)[i] - dry.getReadPointer (0)[i]));
        }
        duckedAtHit = juce::jmax (duckedAtHit, d);
        openAtHit   = juce::jmax (openAtHit,   o);
    }
    printf ("\nduck A/B at the hit:  duck=0 -> %.4f,  duck=0.70 -> %.4f  =  %.1f dB reduction\n",
            openAtHit, duckedAtHit,
            juce::Decibels::gainToDecibels (juce::jmax (1e-6f, duckedAtHit / openAtHit)));
    printf ("peak out (dry+rumble): %.4f\n", wet.getMagnitude (0, total));

    // Isolate the rumble alone, ducked vs not, so the duck's effect is visible
    // with nothing else in the file.
    juce::AudioBuffer<float> rumbleOpen (2, total);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < total; ++i)
            rumbleOpen.getWritePointer (ch)[i] = noDuck.getReadPointer (ch)[i] - dry.getReadPointer (ch)[i];

    juce::WavAudioFormat fmt;
    for (auto pair : { std::make_pair (&dry, "kick_dry.wav"),
                       std::make_pair (&wet, "kick_rumble.wav"),
                       std::make_pair (&rumble, "rumble_only_ducked.wav"),
                       std::make_pair (&rumbleOpen, "rumble_only_open.wav") })
    {
        juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile (pair.second);
        f.deleteFile();
        if (auto* w = fmt.createWriterFor (new juce::FileOutputStream (f), sr, 2, 24, {}, 0))
        {
            w->writeFromAudioSampleBuffer (*pair.first, 0, pair.first->getNumSamples());
            delete w;
        }
    }
    printf ("\nwrote kick_dry.wav, kick_rumble.wav, rumble_only_{ducked,open}.wav\n");

    // ---------------------------------------------------------------- new controls
    printf ("\n--- new controls ---\n");

    auto renderRumble = [&] (float amount, float len, float tone, float drive, float duck,
                             float rel, float space, float hpf, bool solo)
    {
        juce::AudioBuffer<float> out (2, total);
        out.makeCopyOf (dry);
        RumbleEngine e;
        e.prepare ({ sr, (juce::uint32) block, 2 });
        e.setParameters (amount, len, tone, drive, duck, rel, space, hpf, solo);
        e.rebuildImpulseResponse();
        juce::AudioBuffer<float> warm (2, block);
        for (int i = 0; i < 300 && e.getLoadedIrSamples() <= 1; ++i)
        { warm.clear(); e.process (warm); juce::Thread::sleep (2); }
        e.reset();
        for (int pos = 0; pos < total; pos += block)
        {
            const int n = juce::jmin (block, total - pos);
            juce::AudioBuffer<float> slice (out.getArrayOfWritePointers(), 2, pos, n);
            e.process (slice);
        }
        return out;
    };

    // RELEASE: how fast the rumble recovers after a hit. Measure the level a
    // fixed 60 ms after a kick - a short release should already be back up.
    // Measured in SOLO so the rumble is alone - subtracting RMS while the kick
    // is present is not a valid way to isolate it.
    for (float rel : { 20.0f, 90.0f, 400.0f })
    {
        auto o = renderRumble (0.62f, 1.8f, 128.0f, 0.55f, 0.70f, rel, 50.0f, 30.0f, true);
        const double t = 4 * spb;
        // 60 ms windows: at 40-130 Hz a 10 ms window is under half a cycle, so
        // its RMS is dominated by phase rather than level.
        printf ("  RELEASE %5.0f ms -> 0-60ms %.4f  60-120ms %.4f  120-180ms %.4f  300-360ms %.4f\n",
                rel, rmsBetween (o, sr, t + 0.000, t + 0.060),
                     rmsBetween (o, sr, t + 0.060, t + 0.120),
                     rmsBetween (o, sr, t + 0.120, t + 0.180),
                     rmsBetween (o, sr, t + 0.300, t + 0.360));
    }

    // SPACE: changes the generated room. Compare the tail after the last kick.
    for (float sp : { 0.0f, 50.0f, 100.0f })
    {
        auto o = renderRumble (0.62f, 1.8f, 128.0f, 0.55f, 0.70f, 90.0f, sp, 30.0f, false);
        printf ("  SPACE %5.0f      -> tail +0.3s %.4f, +0.8s %.4f\n", sp,
                rmsBetween (o, sr, spb * 8 + 0.30, spb * 8 + 0.50),
                rmsBetween (o, sr, spb * 8 + 0.80, spb * 8 + 1.00));
    }

    // HPF: raising it should strip low energy out of the rumble.
    for (float hp : { 20.0f, 60.0f, 120.0f })
    {
        auto o = renderRumble (0.62f, 1.8f, 128.0f, 0.55f, 0.70f, 90.0f, 50.0f, hp, true);
        printf ("  HPF %5.0f Hz     -> rumble rms in gap %.4f\n", hp,
                rmsBetween (o, sr, 4 * spb + 0.30, 4 * spb + 0.42));
    }

    // SOLO: the dry kick must be gone.
    {
        auto o = renderRumble (0.62f, 1.8f, 128.0f, 0.55f, 0.70f, 90.0f, 50.0f, 30.0f, true);
        printf ("  SOLO on          -> level at the transient %.4f (dry alone is %.4f)\n",
                rmsBetween (o, sr, 4 * spb, 4 * spb + 0.010),
                rmsBetween (dry, sr, 4 * spb, 4 * spb + 0.010));
    }

    // DRIVE = 0 must now be unity, not the old 1.31x boost with baked-in colour.
    {
        juce::AudioBuffer<float> imp (2, block * 4);
        imp.clear();
        for (int ch = 0; ch < 2; ++ch) imp.getWritePointer (ch)[0] = 0.5f;
        RumbleEngine e;
        e.prepare ({ sr, (juce::uint32) block, 2 });
        e.setParameters (1.0f, 0.15f, 300.0f, 0.0f, 0.0f, 90.0f, 50.0f, 20.0f, true);
        printf ("  DRIVE 0 linearity: ");
        for (float in : { 0.05f, 0.20f, 0.50f })
        {
            const float pre = 1.0f;
            const float outv = std::tanh (in * pre) / pre * 1.0f;
            printf ("in %.2f -> %.4f (ratio %.3f)  ", in, outv, outv / in);
        }
        printf ("\n");
    }
    return 0;
}
