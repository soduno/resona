#pragma once

#include "core/ProjectModel.h"
#include "midi_editor/MidiEditorController.h"
#include "transport/TransportController.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <span>

namespace resona
{
class InspectorPresenter;

struct MidiPaintState
{
    juce::Rectangle<int> area, marquee;
    int inspectorWidth, scrollbarThickness, keyboardWidth, headerHeight, rulerHeight;
    bool verticalScrollbar, horizontalScrollbar, inspectorCollapsed, marqueeVisible;
    const ProjectModel& project;
    const TransportController& transport;
    const MidiEditorController& editor;
    std::span<const MidiNote> notes;
};

class MidiEditorPresenter final
{
public:
    explicit MidiEditorPresenter(const InspectorPresenter& inspector) : inspectorPresenter(inspector) {}
    void paint(juce::Graphics&, const MidiPaintState&) const;

private:
    const InspectorPresenter& inspectorPresenter;
};
}
