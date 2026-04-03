#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace resona
{
struct ChromeState
{
    int width {};
    int height {};
    int titleBarHeight {};
    int contentTop {};
    int statusBarHeight {};
    bool clickTrackEnabled {};
    float masterVolume {};
    juce::String statusMessage;
    double visibleBeats {};
    double minimumVisibleBeats {};
    double maximumVisibleBeats {};
};

class ChromePresenter final
{
public:
    void paintToolbar(juce::Graphics&, const ChromeState&) const;
    void paintStatusBar(juce::Graphics&, const ChromeState&) const;
};
}
