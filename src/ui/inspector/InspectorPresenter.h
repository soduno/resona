#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace resona
{
class InspectorPresenter final
{
public:
    void paintInstrument(juce::Graphics&, juce::Rectangle<int>) const;
    void paintNotes(juce::Graphics&, juce::Rectangle<int>) const;
    juce::Rectangle<int> instrumentKnobArea(juce::Rectangle<int> area, int index) const;
    float instrumentValue(int index) const noexcept;
    void setInstrumentValue(int index, float value) noexcept;

private:
    std::array<float, 3> instrumentValues { 0.62f, 0.45f, 0.70f };
};
}
