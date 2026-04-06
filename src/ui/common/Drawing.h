#pragma once

#include "core/ProjectModel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <span>

namespace resona::drawing
{
void smallButton(juce::Graphics&, juce::Rectangle<int>, const juce::String&, bool active);
void magnifier(juce::Graphics&, juce::Rectangle<int>, bool plus);
void trackIcon(juce::Graphics&, juce::Rectangle<int>, int kind);
void piano(juce::Graphics&, juce::Rectangle<int>);
void knob(juce::Graphics&, juce::Rectangle<int>, float value);
void clip(juce::Graphics&, juce::Rectangle<int>, const juce::String&, juce::Colour,
          bool midi, bool selected, std::span<const MidiNote>, double clipLengthBeats);
void playheadMarker(juce::Graphics&, juce::Rectangle<int>, int x);
}
