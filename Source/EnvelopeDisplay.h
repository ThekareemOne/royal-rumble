#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "RumbleEngine.h"

/** The functional centrepiece: rumble envelope over time, with the raw
    (un-ducked) tail behind it so the duck's effect is visible as the gap. */
class EnvelopeDisplay : public juce::Component,
                        private juce::Timer
{
public:
    explicit EnvelopeDisplay (RumbleEngine&);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }
    juce::Path traceFrom (const std::array<std::atomic<float>, RumbleEngine::kScopePoints>&,
                          juce::Rectangle<float> area, float scale) const;

    RumbleEngine& engine;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeDisplay)
};
