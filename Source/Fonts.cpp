#include "Fonts.h"
#include <BinaryData.h>

namespace RoyalFonts
{
    static juce::Font make (const char* data, int size, float height)
    {
        static std::map<const char*, juce::Typeface::Ptr> cache;
        auto& tf = cache[data];
        if (tf == nullptr)
            tf = juce::Typeface::createSystemTypefaceFor (data, (size_t) size);
        return juce::Font (juce::FontOptions (tf).withHeight (height));
    }

    juce::Font display (float h) { return make (BinaryData::ChakraPetchBold_ttf,
                                                BinaryData::ChakraPetchBold_ttfSize, h); }
    juce::Font label   (float h) { return make (BinaryData::ChakraPetchSemiBold_ttf,
                                                BinaryData::ChakraPetchSemiBold_ttfSize, h); }
    juce::Font body    (float h) { return make (BinaryData::ChakraPetchMedium_ttf,
                                                BinaryData::ChakraPetchMedium_ttfSize, h); }
    juce::Font numeric (float h, bool bold)
    {
        return bold ? make (BinaryData::JetBrainsMonoBold_ttf,
                            BinaryData::JetBrainsMonoBold_ttfSize, h)
                    : make (BinaryData::JetBrainsMonoMedium_ttf,
                            BinaryData::JetBrainsMonoMedium_ttfSize, h);
    }
}
