#pragma once
#include "PluginProcessor.h"
#include "RoyalLookAndFeel.h"
#include "EnvelopeDisplay.h"

class RoyalRumbleEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    explicit RoyalRumbleEditor (RoyalRumbleProcessor&);
    ~RoyalRumbleEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    /** Visual weight, not just size: which knobs the eye should land on first. */
    enum class Tier { Primary, Secondary, Utility };

    struct Knob
    {
        juce::String id, caption;
        Tier  tier;
        int   group;
        juce::Slider slider;
        juce::Label  captionLabel, valueLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    void timerCallback() override;
    void buildKnobs();
    void paintScreenPanel (juce::Graphics&, juce::Rectangle<int>, const juce::String&) const;
    static int diameterFor (Tier t) { return t == Tier::Primary ? 104 : t == Tier::Secondary ? 72 : 52; }

    RoyalRumbleProcessor& processor;
    RoyalLookAndFeel lnf;
    EnvelopeDisplay display;

    std::vector<std::unique_ptr<Knob>> knobs;
    juce::TextButton soloButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttach;

    juce::Rectangle<int> grPanel, hzPanel;
    std::array<juce::Rectangle<int>, 4> groupHeaders;
    float grValue { 0.0f }, hzValue { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoyalRumbleEditor)
};
