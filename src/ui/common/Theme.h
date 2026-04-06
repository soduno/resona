#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace resona
{
namespace palette
{
extern const juce::Colour background;
extern const juce::Colour panel;
extern const juce::Colour panelLight;
extern const juce::Colour raised;
extern const juce::Colour line;
extern const juce::Colour grid;
extern const juce::Colour text;
extern const juce::Colour muted;
extern const juce::Colour violet;
extern const juce::Colour violetSoft;
extern const juce::Colour violetHeader;
extern const juce::Colour mint;
extern const juce::Colour mintBody;
extern const juce::Colour red;
}

class ResonaLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ResonaLookAndFeel();

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawMenuBarBackground(juce::Graphics&, int, int, bool,
                               juce::MenuBarComponent&) override;
    void drawMenuBarItem(juce::Graphics&, int width, int height, int,
                         const juce::String&, bool highlighted, bool open,
                         bool, juce::MenuBarComponent&) override;
    void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int x, int y,
                       int width, int height, bool isVertical,
                       int thumbStart, int thumbSize, bool, bool isMouseDown) override;
};
}
