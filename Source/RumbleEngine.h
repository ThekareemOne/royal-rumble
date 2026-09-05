#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

/**
    Generates a rumble tail from an incoming kick and ducks it against that kick.

    Signal flow:

        in ──┬──────────────────────────────────────────────► dry
             │
             ├─► lowpass 200Hz ─► envelope follower ──┐  duck key
             │                                        ▼
             └─► convolution ─► drive ─► tone LPF ─► duck ─► sub cut ─► rumble

    The impulse response is generated procedurally (exponentially decaying,
    progressively darkening noise with sparse early reflections) rather than
    loaded from a file. For a signal that is lowpassed below 300 Hz anyway this
    is indistinguishable from a sampled space, and it makes LENGTH a natural
    parameter: a different decay time is just a different IR.
*/
class RumbleEngine
{
public:
    static constexpr int kScopePoints = 512;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    void setParameters (float amount, float lengthSeconds, float toneHz,
                        float drive, float duck, float releaseMs, float space,
                        float highpassHz, bool solo) noexcept;

    /** Replaces the buffer contents with dry + rumble. */
    void process (juce::AudioBuffer<float>& buffer) noexcept;

    /** Rebuilds and swaps in the impulse response. Never call from the audio thread. */
    void rebuildImpulseResponse();

    /** True when LENGTH has moved far enough that the IR should be regenerated. */
    bool impulseResponseIsStale() const noexcept;

    /** Length in samples of the IR the convolution is actually running. */
    int getLoadedIrSamples() const noexcept { return convolution.getCurrentIRSize(); }

    // --- for the editor, read-only, tearing is acceptable ---------------------
    float getGainReductionDb()  const noexcept { return gainReductionDb.load(); }
    float getDetectedHz()       const noexcept { return detectedHz.load(); }
    int   getScopeWriteIndex()  const noexcept { return scopeIndex.load(); }
    const std::array<std::atomic<float>, kScopePoints>& getRumbleScope() const noexcept { return scopeRumble; }
    const std::array<std::atomic<float>, kScopePoints>& getKickScope()   const noexcept { return scopeKick; }
    const std::array<std::atomic<float>, kScopePoints>& getRawScope()    const noexcept { return scopeRaw; }

private:
    static juce::AudioBuffer<float> generateImpulseResponse (double sampleRate,
                                                             float lengthSeconds,
                                                             float space,
                                                             juce::Random& rng);
    void pushScope (float rumbleOut, float rumbleRaw, float kick) noexcept;

    double sampleRate { 44100.0 };
    int    maxBlock   { 512 };

    juce::dsp::Convolution convolution { juce::dsp::Convolution::NonUniform { 256 } };

    // kick detection
    juce::dsp::StateVariableTPTFilter<float> keyLowpass;
    float kickEnv { 0.0f }, kickAttackCoef { 0.0f }, kickReleaseCoef { 0.0f };
    std::atomic<float> requestedRelease { 90.0f };
    float appliedRelease { -1.0f };

    // rumble path
    juce::dsp::StateVariableTPTFilter<float> toneLowpass;
    juce::dsp::StateVariableTPTFilter<float> subHighpass;

    // pitch estimate (zero-crossing over the lowpassed key signal)
    float lastKeySample { 0.0f };
    int   samplesSinceCrossing { 0 };
    float smoothedHz { 50.0f };

    // parameters, smoothed on the audio thread
    juce::SmoothedValue<float> pAmount, pDrive, pDuck, pTone, pHighpass, pDryGain;
    std::atomic<float> requestedLength { 1.8f };
    std::atomic<float> requestedSpace  { 50.0f };
    float loadedLength { -1.0f }, loadedSpace { -1.0f };
    std::atomic<bool>  soloRumble { false };

    juce::AudioBuffer<float> rumbleBuffer;
    juce::Random random;

    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> detectedHz { 0.0f };
    std::array<std::atomic<float>, kScopePoints> scopeRumble {};
    std::array<std::atomic<float>, kScopePoints> scopeKick {};
    std::array<std::atomic<float>, kScopePoints> scopeRaw {};
    std::atomic<int> scopeIndex { 0 };
    int   scopeCounter { 0 }, scopeStride { 64 };
    float scopePeakR { 0.0f }, scopePeakK { 0.0f }, scopePeakRaw { 0.0f };
};
