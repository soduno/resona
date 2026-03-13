#pragma once

#include "core/ProjectModel.h"
#include "transport/TransportController.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace resona
{
class AudioEngine final
{
public:
    AudioEngine(ProjectModel& projectToUse, TransportController& transportToUse) noexcept;

    void prepare(double sampleRate) noexcept;
    void render(const juce::AudioSourceChannelInfo& info) noexcept;
    float getTrackPeak() const noexcept;
    float getMasterPeak() const noexcept;

private:
    ProjectModel& project;
    TransportController& transport;
    std::atomic<double> currentSampleRate { 44100.0 };
    std::atomic<float> trackPeak { 0.0f };
    std::atomic<float> masterPeak { 0.0f };
};
}
