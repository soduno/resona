#pragma once

#include "core/ProjectModel.h"
#include <cstddef>

namespace resona
{
class MixerController final
{
public:
    explicit MixerController(ProjectModel& projectModel) noexcept : project(projectModel) {}

    size_t channelCount() const noexcept;
    const InstrumentTrack& channel(size_t index) const;
    float channelVolume(size_t index) const noexcept;
    void setChannelVolume(size_t index, float volume) noexcept;
    float channelPan(size_t index) const noexcept;
    void setChannelPan(size_t index, float pan) noexcept;
    bool channelMuted(size_t index) const noexcept;
    bool channelSoloed(size_t index) const noexcept;
    bool toggleChannelMute(size_t index) noexcept;
    bool toggleChannelSolo(size_t index) noexcept;
    bool isMasterChannel(size_t index) const noexcept;
    bool channelSupportsSolo(size_t index) const noexcept;

private:
    ProjectModel& project;
    const InstrumentTrack masterChannel { "Master", "Main Output", "" };
};
}
