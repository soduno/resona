#include "Theme.h"
#include "ui/common/MaterialIcons.h"

namespace resona
{
namespace palette
{
const juce::Colour background { 0xff14181c };
const juce::Colour panel { 0xff1b2025 };
const juce::Colour panelLight { 0xff20262c };
const juce::Colour raised { 0xff292f36 };
const juce::Colour line { 0xff343b44 };
const juce::Colour grid { 0xff272d34 };
const juce::Colour text { 0xfff2f3f5 };
const juce::Colour muted { 0xffaeb4bb };
const juce::Colour violet { 0xff9b82ff };
const juce::Colour violetSoft { 0xff514578 };
const juce::Colour violetHeader { 0xffa58dff };
const juce::Colour mint { 0xff8ee3ba };
const juce::Colour mintBody { 0xff32664f };
const juce::Colour red { 0xfff06b6e };
}

ResonaLookAndFeel::ResonaLookAndFeel()
{
    setColour(juce::TextButton::buttonColourId, palette::raised);
    setColour(juce::TextButton::buttonOnColourId, palette::violet);
    setColour(juce::TextButton::textColourOffId, palette::text);
    setColour(juce::TextButton::textColourOnId, palette::text);
    setColour(juce::PopupMenu::backgroundColourId, palette::panelLight);
    setColour(juce::PopupMenu::textColourId, palette::text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, palette::violet.withAlpha(0.28f));
    setColour(juce::PopupMenu::highlightedTextColourId, palette::text);
}

void ResonaLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool highlighted, bool down)
{
    auto colour = backgroundColour;
    if (highlighted) colour = colour.brighter(0.08f);
    if (down) colour = colour.darker(0.1f);
    g.setColour(colour);
    g.fillRoundedRectangle(button.getLocalBounds().toFloat(), 8.0f);
}

void ResonaLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                       bool, bool)
{
    const auto id = button.getComponentID();
    const auto bounds = button.getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();
    const auto iconColour = button.findColour(button.getToggleState()
                                              ? juce::TextButton::textColourOnId
                                              : juce::TextButton::textColourOffId);
    g.setColour(iconColour);
    if (id == "play")
    {
        material::draw(g, button.getButtonText() == "Ⅱ" ? material::Icon::pause
                                                         : material::Icon::play,
                       { centre.x - 14.0f, centre.y - 14.0f, 28.0f, 28.0f },
                       iconColour);
        return;
    }
    if (id == "stop")
    {
        material::draw(g, material::Icon::stop,
                       { centre.x - 12.0f, centre.y - 12.0f, 24.0f, 24.0f },
                       iconColour);
        return;
    }
    if (id == "record")
    {
        material::draw(g, material::Icon::record,
                       { centre.x - 14.0f, centre.y - 14.0f, 28.0f, 28.0f },
                       palette::red);
        return;
    }
    if (id == "minimise")
    {
        material::draw(g, material::Icon::minimise,
                       { centre.x - 10.0f, centre.y - 10.0f, 20.0f, 20.0f },
                       iconColour);
        return;
    }
    if (id == "maximise")
    {
        material::draw(g, material::Icon::maximise,
                       { centre.x - 9.0f, centre.y - 9.0f, 18.0f, 18.0f },
                       iconColour);
        return;
    }
    if (id == "close")
    {
        material::draw(g, material::Icon::close,
                       { centre.x - 10.0f, centre.y - 10.0f, 20.0f, 20.0f },
                       iconColour);
        return;
    }
    if (id == "inspectorToggle")
    {
        material::draw(g, button.getButtonText() == ">" ? material::Icon::forward
                                                         : material::Icon::back,
                       { centre.x - 11.0f, centre.y - 11.0f, 22.0f, 22.0f },
                       iconColour);
        return;
    }
    if (id == "timeline" || id == "mixer")
    {
        material::draw(g, id == "timeline" ? material::Icon::timeline
                                            : material::Icon::mixer,
                       { bounds.getX() + 23.0f, centre.y - 11.0f, 22.0f, 22.0f },
                       iconColour);
        g.setFont(15.0f);
        g.drawText(button.getButtonText(), 55, 0, button.getWidth() - 66,
                   button.getHeight(), juce::Justification::centredLeft);
        return;
    }
    juce::LookAndFeel_V4::drawButtonText(g, button, false, false);
}

void ResonaLookAndFeel::drawMenuBarBackground(juce::Graphics& g, int, int, bool,
                                               juce::MenuBarComponent&)
{
    g.fillAll(palette::panel);
    g.setColour(palette::line);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(g.getClipBounds().getWidth()));
    g.drawHorizontalLine(g.getClipBounds().getHeight() - 1, 0.0f,
                         static_cast<float>(g.getClipBounds().getWidth()));
}

void ResonaLookAndFeel::drawMenuBarItem(juce::Graphics& g, int width, int height, int,
                                        const juce::String& text, bool highlighted, bool open,
                                        bool, juce::MenuBarComponent&)
{
    if (highlighted || open)
    {
        g.setColour(palette::raised);
        g.fillRoundedRectangle(juce::Rectangle<float>(3.0f, 5.0f,
                                                      static_cast<float>(width - 6),
                                                      static_cast<float>(height - 10)), 6.0f);
    }
    g.setColour(palette::text);
    g.setFont(15.0f);
    g.drawText(text, 12, 0, width - 24, height, juce::Justification::centredLeft);
}

void ResonaLookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar&, int x, int y,
                                      int width, int height, bool isVertical,
                                      int thumbStart, int thumbSize, bool, bool isMouseDown)
{
    auto track = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    g.setColour(palette::panelLight);
    g.fillRoundedRectangle(track, 4.0f);
    auto thumb = isVertical
        ? juce::Rectangle<float>(track.getX(), static_cast<float>(thumbStart), track.getWidth(),
                                 static_cast<float>(thumbSize))
        : juce::Rectangle<float>(static_cast<float>(thumbStart), track.getY(),
                                 static_cast<float>(thumbSize), track.getHeight());
    g.setColour(isMouseDown ? palette::violet.brighter(0.12f) : palette::muted.withAlpha(0.7f));
    g.fillRoundedRectangle(thumb.reduced(1.0f), 4.0f);
}
}
