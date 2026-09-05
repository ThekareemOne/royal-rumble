#include "PluginEditor.h"
#include "Fonts.h"

using namespace RoyalColours;

namespace
{
    constexpr int kWidth = 900, kHeight = 520;
    const char* kGroupNames[] = { "TAIL", "FILTER", "DUCK", "OUTPUT" };

    struct Spec { const char* id; const char* caption; int tier; int group; };
    const Spec kSpecs[] = {
        { RoyalRumbleProcessor::kSpace,    "SPACE",   2, 0 },
        { RoyalRumbleProcessor::kLength,   "LENGTH",  0, 0 },
        { RoyalRumbleProcessor::kHighpass, "HPF",     2, 1 },
        { RoyalRumbleProcessor::kTone,     "TONE",    1, 1 },
        { RoyalRumbleProcessor::kDrive,    "DRIVE",   1, 1 },
        { RoyalRumbleProcessor::kDuck,     "DUCK",    0, 2 },
        { RoyalRumbleProcessor::kRelease,  "RELEASE", 1, 2 },
        { RoyalRumbleProcessor::kAmount,   "AMOUNT",  0, 3 },
    };
}

RoyalRumbleEditor::RoyalRumbleEditor (RoyalRumbleProcessor& p)
    : AudioProcessorEditor (&p), processor (p), display (p.engine)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (display);
    buildKnobs();

    soloButton.setButtonText ("SOLO");
    soloButton.setClickingTogglesState (true);
    addAndMakeVisible (soloButton);
    soloAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                     (processor.apvts, RoyalRumbleProcessor::kSolo, soloButton);

    setSize (kWidth, kHeight);
    startTimerHz (20);
}

RoyalRumbleEditor::~RoyalRumbleEditor() { setLookAndFeel (nullptr); }

void RoyalRumbleEditor::buildKnobs()
{
    for (const auto& spec : kSpecs)
    {
        auto k = std::make_unique<Knob>();
        k->id      = spec.id;
        k->caption = spec.caption;
        k->tier    = (Tier) spec.tier;
        k->group   = spec.group;

        k->slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        k->slider.setRotaryParameters (juce::MathConstants<float>::pi * -0.75f,
                                       juce::MathConstants<float>::pi *  0.75f, true);
        k->slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (k->slider);
        k->attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                        (processor.apvts, k->id, k->slider);

        // Weight the labels with the tier so the hierarchy reads even in mono.
        const bool primary   = k->tier == Tier::Primary;
        const bool utility   = k->tier == Tier::Utility;
        const float capSize  = primary ? 13.5f : utility ? 11.0f : 12.0f;
        const auto  capCol   = primary ? capPrimary : utility ? capUtility : capSecondary;

        k->captionLabel.setText (k->caption, juce::dontSendNotification);
        k->captionLabel.setJustificationType (juce::Justification::centred);
        k->captionLabel.setFont (RoyalFonts::label (capSize));
        k->captionLabel.setColour (juce::Label::textColourId, capCol);
        addAndMakeVisible (k->captionLabel);

        k->valueLabel.setJustificationType (juce::Justification::centred);
        k->valueLabel.setFont (RoyalFonts::numeric (primary ? 15.5f : utility ? 12.0f : 13.5f, primary));
        k->valueLabel.setColour (juce::Label::textColourId,
                                 primary ? readout : utility ? capUtility : capSecondary);
        addAndMakeVisible (k->valueLabel);

        knobs.push_back (std::move (k));
    }
}

void RoyalRumbleEditor::timerCallback()
{
    const float gr = processor.engine.getGainReductionDb();
    const float hz = processor.engine.getDetectedHz();
    if (std::abs (gr - grValue) > 0.05f || std::abs (hz - hzValue) > 0.4f)
    {
        grValue = gr; hzValue = hz;
        repaint (grPanel.getUnion (hzPanel));
    }

    for (auto& k : knobs)
        if (auto* param = processor.apvts.getParameter (k->id))
            k->valueLabel.setText (param->getCurrentValueAsText(), juce::dontSendNotification);
}

void RoyalRumbleEditor::paintScreenPanel (juce::Graphics& g, juce::Rectangle<int> r,
                                                   const juce::String& title) const
{
    g.setColour (screenBg);
    g.fillRoundedRectangle (r.toFloat(), 4.0f);
    g.setColour (screenEdge);
    g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 4.0f, 1.0f);
    g.setFont (RoyalFonts::label (10.5f));
    g.setColour (screenLabel);
    g.drawText (title, r.reduced (13, 9), juce::Justification::topLeft, false);
}

void RoyalRumbleEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient plate (plateTop, 0.0f, 0.0f, plateBottom, 0.0f, (float) getHeight(), false);
    plate.addColour (0.52, plateMid);
    g.setGradientFill (plate);
    g.fillAll();

    // Fine vertical brushing across the plate
    g.setColour (juce::Colour (0x07000000));
    for (int x = 0; x < getWidth(); x += 3)
        g.drawVerticalLine (x, 0.0f, (float) getHeight());

    // Corner registration crosshairs
    g.setColour (juce::Colour (0xff5f5c55).withAlpha (0.32f));
    for (auto pt : { juce::Point<int> (12, 12), { getWidth() - 12, 12 },
                     { 12, getHeight() - 12 }, { getWidth() - 12, getHeight() - 12 } })
    {
        g.drawLine ((float) pt.x, pt.y - 4.5f, (float) pt.x, pt.y + 4.5f, 1.0f);
        g.drawLine (pt.x - 4.5f, (float) pt.y, pt.x + 4.5f, (float) pt.y, 1.0f);
    }

    // --- header --------------------------------------------------------------
    auto header = getLocalBounds().removeFromTop (54);
    g.setColour (rule);      g.drawHorizontalLine (header.getBottom(), 0.0f, (float) getWidth());
    g.setColour (highlight); g.drawHorizontalLine (header.getBottom() + 1, 0.0f, (float) getWidth());

    auto h = header.reduced (18, 0);
    g.setFont (RoyalFonts::display (25.0f));
    g.setColour (ink);
    g.drawText ("ROYAL RUMBLE", h, juce::Justification::centredLeft, false);
    const int markW = g.getCurrentFont().getStringWidth ("ROYAL RUMBLE") + 18;

    g.setColour (rule);
    g.drawVerticalLine (h.getX() + markW, header.getY() + 15.0f, (float) header.getBottom() - 15.0f);

    g.setFont (RoyalFonts::label (11.5f));
    g.setColour (labelDim);
    g.drawText ("KICK TAIL GENERATOR", h.withTrimmedLeft (markW + 14),
                juce::Justification::centredLeft, false);

    const juce::Rectangle<float> lamp { (float) h.getRight() - 13.0f,
                                        (float) header.getCentreY() - 6.5f, 13.0f, 13.0f };
    g.setColour (accent.withAlpha (0.35f)); g.fillEllipse (lamp.expanded (4.0f));
    g.setColour (accent);                   g.fillEllipse (lamp);

    g.setFont (RoyalFonts::numeric (11.0f));
    g.setColour (labelDim);
    g.drawText (juce::String (processor.getSampleRate() / 1000.0, 1) + " kHz",
                h.withTrimmedRight (30), juce::Justification::centredRight, false);

    // --- meters --------------------------------------------------------------
    paintScreenPanel (g, grPanel, "GAIN RED.");
    paintScreenPanel (g, hzPanel, "KICK DETECTED");

    auto drawLed = [&] (juce::Rectangle<int> panel, const juce::String& text, const juce::String& unit)
    {
        auto r = panel.reduced (13, 0).withTrimmedTop (26);
        g.setFont (RoyalFonts::numeric (28.0f, true));
        g.setColour (accent.withAlpha (0.45f));
        g.drawText (text, r.translated (0, 1), juce::Justification::topLeft, false);
        g.setColour (accent);
        g.drawText (text, r, juce::Justification::topLeft, false);
        g.setFont (RoyalFonts::numeric (10.5f));
        g.setColour (juce::Colour (0xff8a5a2c));
        g.drawText (unit, r.withTrimmedLeft (g.getCurrentFont().getStringWidth (text) + 50),
                    juce::Justification::topLeft, false);
    };
    drawLed (grPanel, juce::String (grValue, 1), "dB");
    drawLed (hzPanel, juce::String (juce::roundToInt (hzValue)), "Hz");

    auto bar = grPanel.reduced (13, 0).removeFromBottom (13).withHeight (5);
    g.setColour (juce::Colour (0xff1e1e22));
    g.fillRoundedRectangle (bar.toFloat(), 2.0f);
    const float amt = juce::jlimit (0.0f, 1.0f, -grValue / 24.0f);
    g.setColour (accent);
    g.fillRoundedRectangle (bar.withWidth (juce::roundToInt (bar.getWidth() * amt)).toFloat(), 2.0f);

    // --- group headers and dividers -----------------------------------------
    for (size_t i = 0; i < groupHeaders.size(); ++i)
    {
        auto r = groupHeaders[i];
        if (r.isEmpty()) continue;

        g.setFont (RoyalFonts::label (10.5f));
        g.setColour (juce::Colour (0xff8e8b83));
        const auto txt = juce::String (kGroupNames[i]);
        const int tw = g.getCurrentFont().getStringWidth (txt) + 14;
        g.drawText (txt, r.withWidth (tw), juce::Justification::centredLeft, false);

        // hairline running from the label to the end of the group
        g.setColour (rule.withAlpha (0.8f));
        g.drawHorizontalLine (r.getCentreY(), (float) (r.getX() + tw), (float) r.getRight());
        g.setColour (highlight);
        g.drawHorizontalLine (r.getCentreY() + 1, (float) (r.getX() + tw), (float) r.getRight());
    }

    // --- footer --------------------------------------------------------------
    auto footer = getLocalBounds().removeFromBottom (32);
    g.setColour (rule);      g.drawHorizontalLine (footer.getY(), 0.0f, (float) getWidth());
    g.setColour (highlight); g.drawHorizontalLine (footer.getY() + 1, 0.0f, (float) getWidth());
    g.setFont (RoyalFonts::body (11.0f));
    g.setColour (labelDim);
    g.drawText ("MONO RUMBLE PATH - DUCK ATTACK 1 MS", footer.reduced (18, 0),
                juce::Justification::centredLeft, false);
    g.drawText ("IR - SYNTHESISED HALL", footer.reduced (18, 0),
                juce::Justification::centredRight, false);
}

void RoyalRumbleEditor::resized()
{
    auto b = getLocalBounds();
    b.removeFromTop (54);
    b.removeFromBottom (32);

    // --- upper: display + meter column --------------------------------------
    auto upper = b.removeFromTop (240).reduced (18, 0).withTrimmedTop (12);
    auto meters = upper.removeFromRight (162);
    upper.removeFromRight (14);
    display.setBounds (upper);

    const int panelH = 88;
    grPanel = meters.removeFromTop (panelH);
    meters.removeFromTop (7);
    hzPanel = meters.removeFromTop (panelH);
    meters.removeFromTop (7);
    soloButton.setBounds (meters.removeFromTop (29));

    // --- lower: knobs grouped by function, sized by tier --------------------
    auto lower = b.reduced (18, 0).withTrimmedTop (6);

    constexpr int kIntraGap = 10, kInterGap = 18;
    std::array<int, 4> groupWidth {};
    for (auto& k : knobs)
    {
        auto& w = groupWidth[(size_t) k->group];
        w += diameterFor (k->tier) + (w > 0 ? kIntraGap : 0);
    }
    int totalW = kInterGap * 3;
    for (auto w : groupWidth) totalW += w;

    int x = lower.getX() + (lower.getWidth() - totalW) / 2;
    const int headerY = lower.getY();
    const int centreY = headerY + 17 + diameterFor (Tier::Primary) / 2;

    int lastGroup = -1, groupStart = x;
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = *knobs[i];
        if (k.group != lastGroup)
        {
            if (lastGroup >= 0) x += kInterGap;
            groupStart = x;
            groupHeaders[(size_t) k.group] =
                juce::Rectangle<int> (groupStart, headerY, groupWidth[(size_t) k.group], 13);
            lastGroup = k.group;
        }
        else
        {
            x += kIntraGap;
        }

        const int d = diameterFor (k.tier);
        k.slider.setBounds (x, centreY - d / 2, d, d);

        // Captions and values share a baseline across the whole row so the
        // labels read as one line despite the knobs being different sizes.
        const int capY = centreY + diameterFor (Tier::Primary) / 2 + 5;
        k.captionLabel.setBounds (x - 16, capY, d + 32, 16);
        k.valueLabel  .setBounds (x - 16, capY + 17, d + 32, 19);

        x += d;
    }
}
