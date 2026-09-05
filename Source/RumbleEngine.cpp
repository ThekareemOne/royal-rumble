#include "RumbleEngine.h"

namespace
{
    constexpr float kKeyLowpassHz   = 200.0f;   // isolate kick energy from hats/perc
    constexpr float kPreDelaySec    = 0.012f;
    constexpr float kMinIrSeconds   = 0.15f;
    constexpr float kMaxIrSeconds   = 6.0f;
}

void RumbleEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxBlock   = (int) spec.maximumBlockSize;

    juce::dsp::ProcessSpec mono { spec.sampleRate, spec.maximumBlockSize, 1 };
    convolution.prepare (mono);

    keyLowpass.prepare (mono);
    keyLowpass.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    keyLowpass.setCutoffFrequency (kKeyLowpassHz);

    toneLowpass.prepare (mono);
    toneLowpass.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    subHighpass.prepare (mono);
    subHighpass.setType (juce::dsp::StateVariableTPTFilterType::highpass);

    // 1 ms attack so the duck always catches the transient; the release is the
    // RELEASE parameter - it is what the groove actually rides on.
    kickAttackCoef = std::exp (-1.0f / (0.001f * (float) sampleRate));
    appliedRelease = -1.0f;

    rumbleBuffer.setSize (1, maxBlock, false, false, true);

    const double smoothSec = 0.02;
    for (auto* p : { &pAmount, &pDrive, &pDuck, &pTone, &pHighpass, &pDryGain })
        p->reset (sampleRate, smoothSec);
    pDryGain.setCurrentAndTargetValue (1.0f);

    // ~85 ms of scope history per point at 512 points => about 2 s on screen
    scopeStride = juce::jmax (1, (int) (sampleRate * 2.0 / kScopePoints));

    loadedLength = -1.0f;
    loadedSpace  = -1.0f;
    reset();
}

void RumbleEngine::reset()
{
    convolution.reset();
    keyLowpass.reset();
    toneLowpass.reset();
    subHighpass.reset();
    kickEnv = 0.0f;
    lastKeySample = 0.0f;
    samplesSinceCrossing = 0;
    scopePeakR = scopePeakK = scopePeakRaw = 0.0f;
    scopeCounter = 0;
}

void RumbleEngine::setParameters (float amount, float lengthSeconds, float toneHz,
                                 float drive, float duck, float releaseMs, float space,
                                 float highpassHz, bool solo) noexcept
{
    pAmount  .setTargetValue (amount);
    pDrive   .setTargetValue (drive);
    pDuck    .setTargetValue (duck);
    pTone    .setTargetValue (toneHz);
    pHighpass.setTargetValue (highpassHz);
    pDryGain .setTargetValue (solo ? 0.0f : 1.0f);
    soloRumble.store (solo);
    requestedRelease.store (releaseMs);
    requestedSpace .store (space);
    requestedLength.store (juce::jlimit (kMinIrSeconds, kMaxIrSeconds, lengthSeconds));
}

bool RumbleEngine::impulseResponseIsStale() const noexcept
{
    return loadedLength < 0.0f
        || std::abs (requestedLength.load() - loadedLength) > 0.02f
        || std::abs (requestedSpace .load() - loadedSpace)  > 1.0f;
}

//==============================================================================
juce::AudioBuffer<float> RumbleEngine::generateImpulseResponse (double sr,
                                                                float lengthSeconds,
                                                                float space,
                                                                juce::Random& rng)
{
    // SPACE morphs the character of the generated room, 0 = tight and boxy,
    // 100 = large and diffuse. It moves three things together: how bright the
    // tail starts, how many early reflections there are, and how far they
    // spread before the diffuse tail takes over.
    const float sp = juce::jlimit (0.0f, 1.0f, space * 0.01f);

    const int pre  = (int) (kPreDelaySec * sr);
    const int tail = juce::jmax (64, (int) (lengthSeconds * sr));
    const int n    = pre + tail;

    juce::AudioBuffer<float> ir (1, n);
    ir.clear();
    auto* d = ir.getWritePointer (0);

    // -60 dB over the requested length
    const float tau = juce::jmax (0.01f, lengthSeconds / 6.908f);

    // Decaying noise, progressively darkened by a one-pole whose cutoff falls
    // with time. That drop is what makes a tail read as a large space rather
    // than a burst of static.
    float lp = 0.0f;
    for (int i = 0; i < tail; ++i)
    {
        const float t     = (float) i / (float) sr;
        const float decay = std::exp (-t / tau);
        const float white = rng.nextFloat() * 2.0f - 1.0f;

        // Cutoff falls as the tail dies. A tight room stays darker throughout;
        // a large one opens up at the start and closes slowly.
        const float top  = juce::jmap (sp, 0.18f, 0.50f);
        const float coef = juce::jmap (decay, 0.02f, top);
        lp += coef * (white - lp);

        d[pre + i] = lp * decay;
    }

    // Early reflections give the tail grip. A small room has few, close and
    // strong; a large one has many, spread wide and weaker individually.
    const int   erCount = juce::roundToInt (juce::jmap (sp, 4.0f, 22.0f));
    const float erSpan  = juce::jmap (sp, 0.025f, 0.16f);   // seconds
    const float erLevel = juce::jmap (sp, 0.62f, 0.30f);
    for (int e = 0; e < erCount; ++e)
    {
        const float frac = (float) (e + 1) / (float) erCount;
        const int   pos  = pre + (int) (frac * erSpan * sr * (0.6f + 0.8f * rng.nextFloat()));
        if (pos < n)
            d[pos] += (rng.nextBool() ? 1.0f : -1.0f) * erLevel * (1.0f - frac);
    }

    // Fade the last 15% so the IR ends at exactly zero
    const int fade = juce::jmax (16, (int) (tail * 0.15f));
    for (int i = 0; i < fade && (n - 1 - i) >= 0; ++i)
        d[n - 1 - i] *= (float) i / (float) fade;

    // Energy, not peak. A long decaying-noise IR has a modest peak but enormous
    // total energy, so peak-normalising it makes the convolution roughly 100x
    // too loud. Normalise::yes below does the energy normalisation; this just
    // keeps the buffer in a sane range beforehand.
    const auto peak = ir.getMagnitude (0, n);
    if (peak > 0.0f)
        ir.applyGain (1.0f / peak);

    return ir;
}

void RumbleEngine::rebuildImpulseResponse()
{
    const auto len = requestedLength.load();
    const auto spc = requestedSpace.load();
    auto ir = generateImpulseResponse (sampleRate, len, spc, random);
    convolution.loadImpulseResponse (std::move (ir), sampleRate,
                                     juce::dsp::Convolution::Stereo::no,
                                     juce::dsp::Convolution::Trim::no,
                                     juce::dsp::Convolution::Normalise::yes);
    loadedLength = len;
    loadedSpace  = spc;
}

//==============================================================================
void RumbleEngine::pushScope (float rumbleOut, float rumbleRaw, float kick) noexcept
{
    scopePeakR   = juce::jmax (scopePeakR,   std::abs (rumbleOut));
    scopePeakRaw = juce::jmax (scopePeakRaw, std::abs (rumbleRaw));
    scopePeakK   = juce::jmax (scopePeakK,   std::abs (kick));

    if (++scopeCounter >= scopeStride)
    {
        const int i = scopeIndex.load();
        scopeRumble[(size_t) i].store (scopePeakR);
        scopeRaw   [(size_t) i].store (scopePeakRaw);
        scopeKick  [(size_t) i].store (scopePeakK);
        scopeIndex.store ((i + 1) % kScopePoints);
        scopeCounter = 0;
        scopePeakR = scopePeakK = scopePeakRaw = 0.0f;
    }
}

void RumbleEngine::process (juce::AudioBuffer<float>& buffer) noexcept
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
        return;

    if (rumbleBuffer.getNumSamples() < numSamples)
        rumbleBuffer.setSize (1, numSamples, false, false, true);

    // --- mono sum into the rumble path; everything here lives below 300 Hz ---
    auto* rum = rumbleBuffer.getWritePointer (0);
    for (int i = 0; i < numSamples; ++i)
    {
        float s = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            s += buffer.getReadPointer (ch)[i];
        rum[i] = s / (float) numChannels;
    }

    // RELEASE recomputes the follower coefficient; cheap, so done per block.
    const float relMs = juce::jlimit (5.0f, 800.0f, requestedRelease.load());
    if (std::abs (relMs - appliedRelease) > 0.5f)
    {
        kickReleaseCoef = std::exp (-1.0f / (relMs * 0.001f * (float) sampleRate));
        appliedRelease  = relMs;
    }

    // --- kick detection off the lowpassed mono sum ---------------------------
    // Held in a scratch array so the convolution can consume rum[] in place.
    juce::HeapBlock<float> keyEnv ((size_t) numSamples);
    keyLowpass.snapToZero();
    for (int i = 0; i < numSamples; ++i)
    {
        const float key = keyLowpass.processSample (0, rum[i]);
        const float mag = std::abs (key);

        kickEnv = mag > kickEnv ? kickAttackCoef  * (kickEnv - mag) + mag
                                : kickReleaseCoef * (kickEnv - mag) + mag;
        keyEnv[(size_t) i] = kickEnv;

        // fundamental estimate: interval between upward zero crossings
        ++samplesSinceCrossing;
        if (lastKeySample <= 0.0f && key > 0.0f && samplesSinceCrossing > 4)
        {
            const float hz = (float) sampleRate / (float) samplesSinceCrossing;
            if (hz > 25.0f && hz < 160.0f && mag > 0.01f)
                smoothedHz += 0.05f * (hz - smoothedHz);
            samplesSinceCrossing = 0;
        }
        lastKeySample = key;
    }
    detectedHz.store (smoothedHz);

    // --- convolution ---------------------------------------------------------
    {
        juce::dsp::AudioBlock<float> block (rumbleBuffer.getArrayOfWritePointers(), 1,
                                            (size_t) numSamples);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        convolution.process (ctx);
    }

    // --- drive, tone, duck, sub cut, mix ------------------------------------
    float worstGr = 0.0f;
    toneLowpass.snapToZero();
    subHighpass.snapToZero();

    for (int i = 0; i < numSamples; ++i)
    {
        const float amount = pAmount  .getNextValue();
        const float drive  = pDrive   .getNextValue();
        const float duck   = pDuck    .getNextValue();
        const float tone   = pTone    .getNextValue();
        const float hpf    = pHighpass.getNextValue();
        const float dryG   = pDryGain .getNextValue();

        toneLowpass.setCutoffFrequency (tone);
        subHighpass.setCutoffFrequency (hpf);

        float s = rum[i];

        // Saturation. This is what makes the tail read as techno rather than
        // ambience, and keeps it audible on speakers with no sub.
        //
        // Normalised by tanh'(0) = 1 (i.e. divided by `pre`, not by tanh(pre))
        // so that DRIVE = 0 is genuinely unity - the previous form left a fixed
        // 1.31x boost and a little saturation even at zero.
        const float pre = 1.0f + drive * 11.0f;
        s = std::tanh (s * pre) / pre * juce::jmap (drive, 1.0f, 5.6f);

        s = toneLowpass.processSample (0, s);
        const float preDuck = s;   // the raw tail, for the scope's ghost trace

        // duck keyed off the detected kick
        const float gain = juce::jlimit (0.0f, 1.0f,
                                         1.0f - duck * juce::jmin (1.0f, keyEnv[(size_t) i] * 6.0f));
        s *= gain;
        worstGr = juce::jmax (worstGr, 1.0f - gain);

        s = subHighpass.processSample (0, s);
        s *= amount * 1.4f;

        pushScope (s, preDuck * amount * 1.4f, keyEnv[(size_t) i]);

        // SOLO mutes the dry path so the rumble can be dialled in on its own.
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* w = buffer.getWritePointer (ch);
            w[i] = w[i] * dryG + s;
        }
    }

    // Peak-hold with decay so the meter reads steadily instead of flickering
    // with whichever block happened to land on a transient.
    const float blockDb = juce::Decibels::gainToDecibels (juce::jmax (0.001f, 1.0f - worstGr));
    const float shown   = gainReductionDb.load();
    gainReductionDb.store (blockDb < shown ? blockDb : shown + 0.25f * (blockDb - shown));
}
