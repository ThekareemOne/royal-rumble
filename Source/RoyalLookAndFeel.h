#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/** Light industrial chassis: warm grey plate, dark inset screens, orange
    accent. Lifted from design/Main.dc.html. */
namespace RoyalColours
{
    const juce::Colour plateTop    { 0xffe2e0da };
    const juce::Colour plateMid    { 0xffd3d0c9 };
    const juce::Colour plateBottom { 0xffc6c3bb };
    const juce::Colour rule        { 0xffb5b2aa };
    const juce::Colour highlight   { 0x80ffffff };
    const juce::Colour screenBg    { 0xff0e0e10 };
    const juce::Colour screenEdge  { 0xff96938b };
    const juce::Colour accent      { 0xffff7a18 };
    const juce::Colour ink         { 0xff1c1c1e };
    const juce::Colour readout     { 0xff26262a };
    const juce::Colour label       { 0xff6d6a63 };
    const juce::Colour labelDim    { 0xff827f77 };
    const juce::Colour tickMajor   { 0xff8e8b83 };
    const juce::Colour tickMinor   { 0xffadaaa2 };
    const juce::Colour track       { 0xff0f0f11 };
    const juce::Colour indicator   { 0xfff4f4f2 };
    const juce::Colour gridFine    { 0xff1c1c20 };
    const juce::Colour gridBar     { 0xff26262b };
    const juce::Colour ghost       { 0xff4a3f31 };
    const juce::Colour screenLabel { 0xff6a675f };
    const juce::Colour kickTrace   { 0xffe9e7e1 };
    const juce::Colour capPrimary  { 0xff3c3a35 };
    const juce::Colour capSecondary{ 0xff6d6a63 };
    const juce::Colour capUtility  { 0xff8a877f };
}

class RoyalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RoyalLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
};
