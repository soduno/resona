#include "mixer/MixerController.h"
#include <juce_core/juce_core.h>

namespace resona
{
size_t MixerController::channelCount() const noexcept
{
    return project.instrumentTracks.size() + 1;
}

const InstrumentTrack& MixerController::channel(size_t index) const
{
    return isMasterChannel(index) ? masterChannel : project.instrumentTracks.at(index);
}

float MixerController::channelVolume(size_t index) const noexcept
{
    if (index == 0) return project.getTrackVolume();
    return isMasterChannel(index) ? project.getMasterVolume() : 0.0f;
}

void MixerController::setChannelVolume(size_t index, float volume) noexcept
{
    if (index == 0)
        project.setTrackVolume(juce::jlimit(0.0f, 1.0f, volume));
    else if (isMasterChannel(index))
        project.setMasterVolume(juce::jlimit(0.0f, 1.0f, volume));
}

float MixerController::channelPan(size_t index) const noexcept
{
    if (index == 0) return project.getTrackPan();
    return isMasterChannel(index) ? project.getMasterPan() : 0.0f;
}

void MixerController::setChannelPan(size_t index, float pan) noexcept
{
    if (index == 0)
        project.setTrackPan(pan);
    else if (isMasterChannel(index))
        project.setMasterPan(pan);
}

bool MixerController::channelMuted(size_t index) const noexcept
{
    if (index == 0) return project.isTrackMuted();
    return isMasterChannel(index) && project.isMasterMuted();
}

bool MixerController::channelSoloed(size_t index) const noexcept
{
    return index == 0 && project.isTrackSoloed();
}

bool MixerController::toggleChannelMute(size_t index) noexcept
{
    if (index != 0 && !isMasterChannel(index)) return false;
    const auto muted = index == 0 ? !project.isTrackMuted() : !project.isMasterMuted();
    if (index == 0) project.setTrackMuted(muted);
    else project.setMasterMuted(muted);
    return muted;
}

bool MixerController::toggleChannelSolo(size_t index) noexcept
{
    if (index != 0) return false;
    const auto soloed = !project.isTrackSoloed();
    project.setTrackSoloed(soloed);
    return soloed;
}

bool MixerController::isMasterChannel(size_t index) const noexcept
{
    return index == project.instrumentTracks.size();
}

bool MixerController::channelSupportsSolo(size_t index) const noexcept
{
    return !isMasterChannel(index);
}
}
