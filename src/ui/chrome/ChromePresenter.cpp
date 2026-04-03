#include "ui/chrome/ChromePresenter.h"
#include "ui/common/Drawing.h"
#include "ui/common/MaterialIcons.h"
#include "ui/common/Theme.h"
#include <cmath>

namespace resona
{
void ChromePresenter::paintToolbar(juce::Graphics& g, const ChromeState& state) const
{
    g.setColour(palette::background);
    g.fillRect(0, 0, state.width, state.titleBarHeight);
    g.setColour(palette::line);
    g.drawHorizontalLine(state.titleBarHeight - 1, 0.0f, (float) state.width);

    g.setColour(palette::panel);
    g.fillRect(0, state.titleBarHeight, state.width, state.contentTop - state.titleBarHeight);
    g.setColour(palette::line);
    g.drawHorizontalLine(state.contentTop - 1, 0.0f, (float) state.width);

    const auto toolY = state.titleBarHeight + 14;
    g.drawVerticalLine(state.width - 325, (float) toolY, (float) toolY + 46.0f);
    g.drawVerticalLine(state.width - 86, (float) toolY, (float) toolY + 46.0f);
    const auto centreY = (float) toolY + 23.0f;

    g.setColour(palette::text);
    g.setFont(17.0f);
    g.drawText("120 BPM", state.width / 2 + 116, toolY, 100, 46, juce::Justification::centredLeft);
    material::draw(g, material::Icon::arrowDropDown,
                   { (float) state.width / 2 + 201.0f, (float) toolY + 11.0f,
                     24.0f, 24.0f }, palette::text);

    const auto metroX = (float) state.width - 355.0f;
    if (state.clickTrackEnabled)
    {
        g.setColour(palette::violet.withAlpha(0.22f));
        g.fillRoundedRectangle(metroX - 18.0f, centreY - 18.0f, 36.0f, 36.0f, 7.0f);
    }
    material::draw(g, material::Icon::clickTrack,
                   { metroX - 12.0f, centreY - 12.0f, 24.0f, 24.0f },
                   state.clickTrackEnabled ? palette::violet : palette::muted);

    g.setColour(palette::grid);
    g.fillRoundedRectangle((float) state.width - 320.0f, (float) toolY + 20.0f, 158.0f, 7.0f, 3.5f);
    const auto masterAmount = juce::jlimit(0.0f, 1.0f, state.masterVolume);
    const auto masterWidth = 158.0f * masterAmount;
    const auto masterKnobX = (float) state.width - 320.0f + masterWidth;
    g.setColour(palette::violet);
    if (masterWidth > 0.0f)
        g.fillRoundedRectangle((float) state.width - 320.0f, (float) toolY + 20.0f,
                               masterWidth, 7.0f, 3.5f);
    g.fillEllipse(masterKnobX - 9.0f, (float) toolY + 14.0f, 18.0f, 18.0f);

    const auto speakerX = (float) state.width - 122.0f;
    material::draw(g, material::Icon::volume,
                   { speakerX - 12.0f, centreY - 12.0f, 24.0f, 24.0f }, palette::text);

    const auto gear = juce::Point<float>((float) state.width - 43.0f, centreY);
    material::draw(g, material::Icon::settings,
                   { gear.x - 12.0f, gear.y - 12.0f, 24.0f, 24.0f }, palette::text);
}

void ChromePresenter::paintStatusBar(juce::Graphics& g, const ChromeState& state) const
{
    const auto area = juce::Rectangle<int>(0, state.height - state.statusBarHeight,
                                           state.width, state.statusBarHeight);
    g.setColour(palette::panel);
    g.fillRect(area);
    g.setColour(palette::line);
    g.drawHorizontalLine(area.getY(), 0.0f, (float) state.width);
    g.setColour(juce::Colour(0xff72dd72));
    g.fillEllipse(27.0f, (float) area.getY() + 21.0f, 14.0f, 14.0f);
    g.setColour(palette::text);
    g.setFont(13.5f);
    g.drawText(state.statusMessage, area.getX() + 58, area.getY(), 240, area.getHeight(),
               juce::Justification::centredLeft);
    g.setColour(palette::line);
    for (int x : { area.getRight() - 690, area.getRight() - 570,
                   area.getRight() - 420, area.getRight() - 318 })
        g.drawVerticalLine(x, (float) area.getY() + 15.0f, (float) area.getBottom() - 15.0f);
    g.setColour(palette::text);
    g.drawText("ASIO", area.getRight() - 790, area.getY(), 74, area.getHeight(), juce::Justification::centredRight);
    g.drawText("44.1 kHz", area.getRight() - 560, area.getY(), 118, area.getHeight(), juce::Justification::centredLeft);
    g.drawText("256 samples", area.getRight() - 410, area.getY(), 112, area.getHeight(), juce::Justification::centredLeft);
    g.drawText("CPU 4%", area.getRight() - 290, area.getY(), 90, area.getHeight(), juce::Justification::centredLeft);
    drawing::magnifier(g, { area.getRight() - 174, area.getCentreY() - 9, 18, 18 }, false);
    const auto amount = (float) (1.0 - (state.visibleBeats - state.minimumVisibleBeats)
                                     / (state.maximumVisibleBeats - state.minimumVisibleBeats));
    const auto trackX = area.getRight() - 130, trackWidth = 72;
    const auto knobX = trackX + juce::roundToInt(amount * trackWidth);
    g.setColour(palette::grid);
    g.fillRoundedRectangle((float) trackX, (float) area.getCentreY() - 3.0f, (float) trackWidth, 6.0f, 3.0f);
    g.setColour(palette::violet);
    if (knobX > trackX)
        g.fillRoundedRectangle((float) trackX, (float) area.getCentreY() - 3.0f,
                               (float) knobX - trackX, 6.0f, 3.0f);
    g.fillEllipse((float) knobX - 8.0f, (float) area.getCentreY() - 8.0f, 16.0f, 16.0f);
    g.setColour(palette::text);
    drawing::magnifier(g, { area.getRight() - 42, area.getCentreY() - 9, 18, 18 }, true);
}
}
