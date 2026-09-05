#include "RoyalLookAndFeel.h"
#include "Fonts.h"

using namespace RoyalColours;

RoyalLookAndFeel::RoyalLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId, accent);
}

void RoyalLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width,
                                         int height, float sliderPos,
                                         float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const auto centre = bounds.getCentre();
    const float r     = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 11.0f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const float scale = juce::jlimit (0.6f, 1.0f, r / 41.0f);

    for (int i = 0; i <= 10; ++i)
    {
        const bool  major = (i % 5 == 0);
        const float a  = startAngle + (float) i / 10.0f * (endAngle - startAngle);
        const float r0 = r + 5.0f, r1 = r + (major ? 9.0f : 7.0f);
        g.setColour (major ? tickMajor : tickMinor);
        g.drawLine ({ { centre.x + r0 * std::sin (a), centre.y - r0 * std::cos (a) },
                      { centre.x + r1 * std::sin (a), centre.y - r1 * std::cos (a) } },
                    major ? 1.3f : 0.9f);
    }

    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, r, r, 0.0f, startAngle, endAngle, true);
    g.setColour (track);
    g.strokePath (arc, juce::PathStrokeType (2.6f * scale, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
    if (sliderPos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (centre.x, centre.y, r, r, 0.0f, startAngle, angle, true);
        g.setColour (accent);
        g.strokePath (val, juce::PathStrokeType (2.6f * scale, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // Cap: machined dark metal with a cast shadow under it.
    const float capR = r - 5.0f;
    g.setColour (juce::Colour (0x33000000));
    g.fillEllipse (juce::Rectangle<float> (capR * 2.0f + 3.0f, capR * 2.0f + 3.0f)
                       .withCentre (centre.translated (0.0f, 1.5f)));

    juce::ColourGradient cap (juce::Colour (0xff4a4a4f),
                              centre.x - capR * 0.3f, centre.y - capR * 0.45f,
                              juce::Colour (0xff151517),
                              centre.x + capR * 0.6f, centre.y + capR * 0.8f, true);
    cap.addColour (0.55, juce::Colour (0xff2a2a2d));
    g.setGradientFill (cap);
    g.fillEllipse (juce::Rectangle<float> (capR * 2.0f, capR * 2.0f).withCentre (centre));
    g.setColour (juce::Colour (0xff141416));
    g.drawEllipse (juce::Rectangle<float> (capR * 2.0f, capR * 2.0f).withCentre (centre), 1.0f);

    const float i0 = capR - 10.0f * scale, i1 = capR - 2.5f;
    g.setColour (indicator);
    g.drawLine ({ { centre.x + i0 * std::sin (angle), centre.y - i0 * std::cos (angle) },
                  { centre.x + i1 * std::sin (angle), centre.y - i1 * std::cos (angle) } },
                2.4f * scale);
}

void RoyalLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b,
                                             const juce::Colour&, bool over, bool)
{
    const auto r  = b.getLocalBounds().toFloat();
    const bool on = b.getToggleState();

    g.setColour (on ? accent : juce::Colour (0xff1a1a1d));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (on ? accent.brighter (0.25f) : screenEdge.withAlpha (over ? 1.0f : 0.8f));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    if (on)
    {
        g.setColour (accent.withAlpha (0.30f));
        g.fillRoundedRectangle (r.expanded (3.0f), 5.0f);
    }
}

void RoyalLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    g.setFont (RoyalFonts::label (11.0f));
    g.setColour (b.getToggleState() ? juce::Colour (0xff141210) : juce::Colour (0xff8a8780));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred, false);
}
