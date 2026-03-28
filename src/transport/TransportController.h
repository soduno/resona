#pragma once

#include <atomic>

namespace resona
{
class TransportController final
{
public:
    bool togglePlayback() noexcept;
    void setPlaying(bool shouldPlay) noexcept;
    bool toggleClickTrack() noexcept;
    void returnToStart() noexcept;

    bool isPlaying() const noexcept;
    bool isClickTrackEnabled() const noexcept;
    double getPlayheadBeats() const noexcept;
    void setPlayheadBeats(double beat) noexcept;

private:
    std::atomic<bool> playing { false };
    std::atomic<bool> clickTrackEnabled { false };
    std::atomic<double> playheadBeats { 0.0 };
};
}
