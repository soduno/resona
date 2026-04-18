#pragma once

#include "core/ProjectModel.h"
#include "timeline/TimelineController.h"
#include "transport/TransportController.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <span>

namespace resona
{
class InspectorPresenter;

struct TimelinePaintState
{
    juce::Rectangle<int> area, volumeSlider, meter, marquee;
    int leftSidebarWidth, inspectorWidth, scrollbarThickness, rulerHeight, laneHeight;
    bool verticalScrollbar, horizontalScrollbar, inspectorCollapsed, marqueeVisible;
    double projectLengthBeats, midiClipLengthBeats;
    float peak;
    const ProjectModel& project;
    const TransportController& transport;
    const TimelineController& timeline;
    std::span<const TimelineClip> clips;
    std::span<const MidiNote> notes;
};

class TimelinePresenter final
{
public:
    explicit TimelinePresenter(const InspectorPresenter& inspector) : inspectorPresenter(inspector) {}
    void paint(juce::Graphics&, const TimelinePaintState&) const;

private:
    const InspectorPresenter& inspectorPresenter;
};
}
