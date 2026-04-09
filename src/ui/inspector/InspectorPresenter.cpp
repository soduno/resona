#include "ui/inspector/InspectorPresenter.h"
#include "ui/common/Drawing.h"
#include "ui/common/MaterialIcons.h"
#include "ui/common/Theme.h"
#include <array>

namespace resona
{
juce::Rectangle<int> InspectorPresenter::instrumentKnobArea(juce::Rectangle<int> area,
                                                             int index) const
{
    if (!juce::isPositiveAndBelow(index, 3)) return {};
    const auto piano = juce::Rectangle<int>(area.getX() + 18, area.getY() + 76,
                                             area.getWidth() - 36, 182);
    const auto knobY = piano.getBottom() + 34;
    const auto third = area.getWidth() / 3;
    return { area.getX() + third * index + third / 2 - 32, knobY, 64, 64 };
}

float InspectorPresenter::instrumentValue(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, 3)
        ? instrumentValues[(size_t) index] : 0.0f;
}

void InspectorPresenter::setInstrumentValue(int index, float value) noexcept
{
    if (juce::isPositiveAndBelow(index, 3))
        instrumentValues[(size_t) index] = juce::jlimit(0.0f, 1.0f, value);
}

void InspectorPresenter::paintInstrument(juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setColour(palette::text);
    g.setFont(21.0f);
    g.drawText("Soft Piano", area.getX() + 58, area.getY(), area.getWidth() - 176, 60,
               juce::Justification::centredLeft);

    auto selector = juce::Rectangle<int>(area.getRight() - 108, area.getY() + 18, 86, 36);
    g.setColour(palette::raised);
    g.fillRoundedRectangle(selector.toFloat(), 6.0f);
    g.setColour(palette::text);
    g.setFont(13.0f);
    g.drawText("VST3", selector.reduced(10).withTrimmedRight(16), juce::Justification::centredLeft);
    material::draw(g, material::Icon::arrowDropDown,
                   { (float) selector.getRight() - 27.0f,
                     (float) selector.getCentreY() - 10.0f, 20.0f, 20.0f }, palette::text);

    auto piano = juce::Rectangle<int>(area.getX() + 18, area.getY() + 76, area.getWidth() - 36, 182);
    g.setColour(juce::Colour(0xff101317));
    g.fillRoundedRectangle(piano.toFloat(), 5.0f);
    drawing::piano(g, piano.reduced(22, 20));

    const auto knobY = piano.getBottom() + 34;
    const auto third = area.getWidth() / 3;
    for (int index = 0; index < 3; ++index)
        drawing::knob(g, instrumentKnobArea(area, index), instrumentValues[(size_t) index]);
    g.setColour(palette::text);
    g.setFont(13.5f);
    g.drawText("Tone", area.getX(), knobY + 70, third, 30, juce::Justification::centred);
    g.drawText("Space", area.getX() + third, knobY + 70, third, 30, juce::Justification::centred);
    g.drawText("Mix", area.getX() + third * 2, knobY + 70, third, 30, juce::Justification::centred);

    const auto dividerY = knobY + 116;
    g.setColour(palette::line);
    g.drawHorizontalLine(dividerY, (float) area.getX() + 18.0f, (float) area.getRight() - 18.0f);
    g.setColour(palette::muted);
    g.setFont(15.0f);
    material::draw(g, material::Icon::add,
                   { (float) area.getX() + 24.0f, (float) dividerY + 29.0f,
                     22.0f, 22.0f }, palette::muted);
    g.drawText("Add Effect", area.getX() + 54, dividerY + 20, area.getWidth() - 78, 40,
               juce::Justification::centredLeft);
}

void InspectorPresenter::paintNotes(juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setColour(palette::text);
    g.setFont(20.0f);
    g.drawText("Notes", area.getX() + 58, area.getY(), area.getWidth() - 78, 48,
               juce::Justification::centredLeft);
    const std::array<std::pair<juce::String, juce::String>, 4> fields {{
        { "Start", "5.1.1" }, { "Length", "0.2.0" },
        { "Velocity", "92" }, { "Transpose", "0" }
    }};
    int y = area.getY() + 86;
    for (const auto& [label, value] : fields)
    {
        g.setColour(palette::muted);
        g.setFont(13.0f);
        g.drawText(label, area.getX() + 20, y, 80, 38, juce::Justification::centredLeft);
        const auto box = juce::Rectangle<int>(area.getRight() - 112, y, 92, 38);
        g.setColour(palette::raised);
        g.fillRoundedRectangle(box.toFloat(), 7.0f);
        g.setColour(palette::text);
        g.drawText(value, box.reduced(10), juce::Justification::centredLeft);
        y += 72;
    }
}
}
