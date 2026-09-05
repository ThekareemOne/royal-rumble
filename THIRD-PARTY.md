# Third-party components

## JUCE 8.0.4
Dual-licensed AGPLv3 or commercial. This project takes the **AGPLv3** option,
which is what permits building with `JUCE_DISPLAY_SPLASH_SCREEN=0`. JUCE is not
vendored here — CMake fetches it at configure time.
<https://github.com/juce-framework/JUCE>

## VST3 SDK (bundled inside JUCE)
Steinberg proprietary licence **or** GPLv3. Used here under GPLv3, which is
compatible with this project's AGPLv3 licence.

VST is a trademark of Steinberg Media Technologies GmbH.

## Fonts (embedded in the plugin binary)
| Font | Licence |
|---|---|
| Chakra Petch | SIL Open Font Licence 1.1 — `Fonts/OFL-ChakraPetch.txt` |
| JetBrains Mono | SIL Open Font Licence 1.1 — `Fonts/OFL-JetBrainsMono.txt` |

Both permit embedding and redistribution. Neither is sold as part of this
software.

## Not used
No sampled impulse responses ship with this plugin — the reverb IR is generated
procedurally at runtime, so there is nothing to license.
