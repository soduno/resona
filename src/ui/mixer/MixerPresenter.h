#pragma once

#include "mixer/MixerController.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace resona
{
class InspectorPresenter;

class MixerPresenter final
{
public:
    explicit MixerPresenter(const InspectorPresenter& inspector) : inspectorPresenter(inspector) {}
    void setChannelStripWidth(int width) noexcept;
    int getChannelStripWidth() const noexcept { return channelStripWidth; }
    void paint(juce::Graphics&, juce::Rectangle<int>, int inspectorWidth,
               bool inspectorCollapsed, const MixerController&, float trackPeak,
               float masterPeak) const;
    juce::Rectangle<int> faderHitArea(juce::Rectangle<int>, int inspectorWidth,
                                      int trackIndex = 0) const;
    juce::Rectangle<int> panHitArea(juce::Rectangle<int>, int inspectorWidth,
                                    int trackIndex = 0) const;
    juce::Rectangle<int> muteButtonArea(juce::Rectangle<int>, int inspectorWidth,
                                        int trackIndex = 0) const;
    juce::Rectangle<int> soloButtonArea(juce::Rectangle<int>, int inspectorWidth,
                                        int trackIndex = 0) const;
    juce::Rectangle<int> stripResizeHandleArea(juce::Rectangle<int>, int inspectorWidth,
                                               int trackIndex) const;
    float volumeForY(juce::Rectangle<int>, int inspectorWidth, int y,
                     int trackIndex = 0) const;
    float panForX(juce::Rectangle<int>, int inspectorWidth, int x,
                  int trackIndex = 0) const;

private:
    static constexpr int minimumChannelStripWidth = 150;
    static constexpr int maximumChannelStripWidth = 280;
    const InspectorPresenter& inspectorPresenter;
    int channelStripWidth = 178;
};
}
