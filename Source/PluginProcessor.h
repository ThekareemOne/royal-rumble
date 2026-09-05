#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "RumbleEngine.h"

class RoyalRumbleProcessor : public juce::AudioProcessor,
                             private juce::AsyncUpdater
{
public:
    RoyalRumbleProcessor();
    ~RoyalRumbleProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Rumble"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    RumbleEngine engine;

    static constexpr const char* kAmount = "amount";
    static constexpr const char* kLength = "length";
    static constexpr const char* kTone   = "tone";
    static constexpr const char* kDrive  = "drive";
    static constexpr const char* kDuck    = "duck";
    static constexpr const char* kRelease = "release";
    static constexpr const char* kSpace   = "space";
    static constexpr const char* kHighpass= "highpass";
    static constexpr const char* kSolo    = "solo";

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void handleAsyncUpdate() override;   // rebuilds the IR off the audio thread

    std::atomic<bool> rebuildQueued { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoyalRumbleProcessor)
};
