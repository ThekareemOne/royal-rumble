#include "EnvelopeDisplay.h"
#include "RoyalLookAndFeel.h"
#include "Fonts.h"

using namespace RoyalColours;

EnvelopeDisplay::EnvelopeDisplay (RumbleEngine& e) : engine (e)
{
    startTimerHz (30);
}

juce::Path EnvelopeDisplay::traceFrom (
    const std::array<std::atomic<float>, RumbleEngine::kScopePoints>& src,
    juce::Rectangle<float> area, float scale) const
{
    constexpr int N = RumbleEngine::kScopePoints;
    const int write = engine.getScopeWriteIndex();

    juce::Path p;
    for (int i = 0; i < N; ++i)
    {
        // oldest sample first, so the trace scrolls right to left
        const float v = src[(size_t) ((write + i) % N)].load();
        const float x = area.getX() + area.getWidth()  * (float) i / (float) (N - 1);
        const float y = area.getBottom() - area.getHeight() * juce::jlimit (0.0f, 1.0f, v * scale);
        if (i == 0) p.startNewSubPath (x, y);
        else        p.lineTo (x, y);
    }
    return p;
}

void EnvelopeDisplay::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();

    g.setColour (screenBg);
    g.fillRoundedRectangle (b, 4.0f);

    const auto plot = b.reduced (1.0f);

    // --- grid ---------------------------------------------------------------
    g.setColour (gridFine);
    for (int i = 1; i < 4; ++i)
    {
        const float y = plot.getY() + plot.getHeight() * (float) i / 4.0f;
        g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
    }
    g.setColour (gridBar);
    for (int i = 1; i < 4; ++i)
    {
        const float x = plot.getX() + plot.getWidth() * (float) i / 4.0f;
        g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
    }

    const auto area = plot.reduced (0.0f, 14.0f).withTrimmedTop (10.0f);
    constexpr float kScale = 3.2f;   // the tail sits well below full scale

    // --- raw tail (what it would be without ducking) -------------------------
    {
        auto raw = traceFrom (engine.getRawScope(), area, kScale);
        const float dashes[] = { 3.0f, 4.0f };
        g.setColour (ghost);
        juce::Path dashed;
        juce::PathStrokeType (1.1f).createDashedStroke (dashed, raw, dashes, 2);
        g.fillPath (dashed);
    }

    // --- ducked output, filled ----------------------------------------------
    {
        auto out = traceFrom (engine.getRumbleScope(), area, kScale);

        auto filled = out;
        filled.lineTo (area.getRight(), area.getBottom());
        filled.lineTo (area.getX(),     area.getBottom());
        filled.closeSubPath();

        g.setGradientFill ({ accent.withAlpha (0.30f), 0.0f, area.getY(),
                             accent.withAlpha (0.015f), 0.0f, area.getBottom(), false });
        g.fillPath (filled);

        g.setColour (accent);
        g.strokePath (out, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // --- dry kick ------------------------------------------------------------
    {
        auto kick = traceFrom (engine.getKickScope(), area, 1.6f);
        g.setColour (kickTrace.withAlpha (0.82f));
        g.strokePath (kick, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // --- chrome --------------------------------------------------------------
    g.setFont (RoyalFonts::label (8.5f));
    g.setColour (screenLabel);
    g.drawText ("RUMBLE ENVELOPE - 2 S", plot.reduced (10.0f, 7.0f),
                juce::Justification::topLeft, false);

    auto legend = plot.reduced (10.0f, 7.0f);
    g.setColour (kickTrace.withAlpha (0.7f));
    g.drawText ("KICK", legend, juce::Justification::topRight, false);
    g.setColour (ghost.brighter (0.35f));
    g.drawText ("RAW TAIL", legend.withTrimmedRight (36), juce::Justification::topRight, false);
    g.setColour (accent);
    g.drawText ("OUT", legend.withTrimmedRight (100), juce::Justification::topRight, false);

    g.setColour (screenEdge);
    g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);
}
