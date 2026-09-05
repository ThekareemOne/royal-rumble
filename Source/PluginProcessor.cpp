#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

namespace
{
    juce::String pct (float v, int)      { return juce::String (juce::roundToInt (v * 100.0f)) + "%"; }
    juce::String secs (float v, int)     { return v < 1.0f ? juce::String (juce::roundToInt (v * 1000.0f)) + " ms"
                                                          : juce::String (v, 2) + " s"; }
    juce::String hertz (float v, int)    { return juce::String (juce::roundToInt (v)) + " Hz"; }
}

APVTS::ParameterLayout RoyalRumbleProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using A = juce::AudioParameterFloatAttributes;
    APVTS::ParameterLayout layout;

    layout.add (std::make_unique<P> (juce::ParameterID { kAmount, 1 }, "Amount",
        juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.62f,
        A{}.withStringFromValueFunction (pct)));

    layout.add (std::make_unique<P> (juce::ParameterID { kLength, 1 }, "Length",
        juce::NormalisableRange<float> { 0.15f, 6.0f, 0.0f, 0.45f }, 1.8f,
        A{}.withStringFromValueFunction (secs)));

    layout.add (std::make_unique<P> (juce::ParameterID { kTone, 1 }, "Tone",
        juce::NormalisableRange<float> { 40.0f, 300.0f, 0.0f, 0.6f }, 128.0f,
        A{}.withStringFromValueFunction (hertz)));

    layout.add (std::make_unique<P> (juce::ParameterID { kDrive, 1 }, "Drive",
        juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.55f,
        A{}.withStringFromValueFunction (pct)));

    layout.add (std::make_unique<P> (juce::ParameterID { kDuck, 1 }, "Duck",
        juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.70f,
        A{}.withStringFromValueFunction (pct)));

    layout.add (std::make_unique<P> (juce::ParameterID { kRelease, 1 }, "Release",
        juce::NormalisableRange<float> { 10.0f, 500.0f, 0.0f, 0.5f }, 90.0f,
        A{}.withStringFromValueFunction ([] (float v, int) {
            return juce::String (juce::roundToInt (v)) + " ms"; })));

    layout.add (std::make_unique<P> (juce::ParameterID { kSpace, 1 }, "Space",
        juce::NormalisableRange<float> { 0.0f, 100.0f }, 50.0f,
        A{}.withStringFromValueFunction ([] (float v, int) {
            return juce::String (juce::roundToInt (v)); })));

    layout.add (std::make_unique<P> (juce::ParameterID { kHighpass, 1 }, "Highpass",
        juce::NormalisableRange<float> { 20.0f, 120.0f, 0.0f, 0.7f }, 30.0f,
        A{}.withStringFromValueFunction (hertz)));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kSolo, 1 }, "Solo", false));

    return layout;
}

RoyalRumbleProcessor::RoyalRumbleProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

bool RoyalRumbleProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void RoyalRumbleProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    engine.prepare (spec);
    engine.rebuildImpulseResponse();   // prepareToPlay is not the audio thread
}

void RoyalRumbleProcessor::handleAsyncUpdate()
{
    engine.rebuildImpulseResponse();
    rebuildQueued.store (false);
}

void RoyalRumbleProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    engine.setParameters (apvts.getRawParameterValue (kAmount)->load(),
                          apvts.getRawParameterValue (kLength)->load(),
                          apvts.getRawParameterValue (kTone)->load(),
                          apvts.getRawParameterValue (kDrive)->load(),
                          apvts.getRawParameterValue (kDuck)->load(),
                          apvts.getRawParameterValue (kRelease)->load(),
                          apvts.getRawParameterValue (kSpace)->load(),
                          apvts.getRawParameterValue (kHighpass)->load(),
                          apvts.getRawParameterValue (kSolo)->load() > 0.5f);

    // LENGTH changes mean a new impulse response, which allocates - so it is
    // queued to the message thread rather than done here.
    if (engine.impulseResponseIsStale() && ! rebuildQueued.exchange (true))
        triggerAsyncUpdate();

    engine.process (buffer);
}

juce::AudioProcessorEditor* RoyalRumbleProcessor::createEditor()
{
    return new RoyalRumbleEditor (*this);
}

void RoyalRumbleProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void RoyalRumbleProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoyalRumbleProcessor();
}
