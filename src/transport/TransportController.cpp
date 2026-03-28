#include "TransportController.h"

namespace resona
{
bool TransportController::togglePlayback() noexcept
{
    const auto next = !playing.load();
    playing.store(next);
    return next;
}

void TransportController::setPlaying(bool shouldPlay) noexcept
{
    playing.store(shouldPlay);
}

bool TransportController::toggleClickTrack() noexcept
{
    const auto next = !clickTrackEnabled.load();
    clickTrackEnabled.store(next);
    return next;
}

void TransportController::returnToStart() noexcept
{
    playheadBeats.store(0.0);
}

bool TransportController::isPlaying() const noexcept { return playing.load(); }
bool TransportController::isClickTrackEnabled() const noexcept { return clickTrackEnabled.load(); }
double TransportController::getPlayheadBeats() const noexcept { return playheadBeats.load(); }
void TransportController::setPlayheadBeats(double beat) noexcept { playheadBeats.store(beat); }
}
