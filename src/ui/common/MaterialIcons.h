#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace resona::material
{
enum class Icon
{
    add,
    arrowDropDown,
    close,
    maximise,
    record,
    back,
    forward,
    minimise,
    clickTrack,
    piano,
    pause,
    play,
    settings,
    stop,
    mixer,
    timeline,
    volume,
    zoomIn,
    zoomOut,
    count
};

void draw(juce::Graphics&, Icon, juce::Rectangle<float>, juce::Colour,
          float opacity = 1.0f);
}
