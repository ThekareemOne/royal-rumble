#pragma once
#include <juce_graphics/juce_graphics.h>

/** Embedded typefaces. Chakra Petch for labels and the wordmark - angular and
    technical without tipping into sci-fi; JetBrains Mono for anything numeric. */
namespace RoyalFonts
{
    juce::Font display (float height);      // Chakra Petch Bold
    juce::Font label   (float height);      // Chakra Petch SemiBold
    juce::Font body    (float height);      // Chakra Petch Medium
    juce::Font numeric (float height, bool bold = false);   // JetBrains Mono
}
