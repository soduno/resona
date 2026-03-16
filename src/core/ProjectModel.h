#pragma once

#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <vector>

namespace resona
{
inline constexpr double tempoBpm = 120.0;
inline constexpr double projectLengthBeats = 128.0;
inline constexpr double initialVisibleBeats = 32.0;
inline constexpr double minimumVisibleBeats = 4.0;
inline constexpr double maximumVisibleBeats = 128.0;
inline constexpr double midiClipLengthBeats = 28.0;

struct MidiNote
{
    double beat = 0.0;
    double length = 1.0;
    int note = 60;
    float velocity = 0.8f;
};

struct TimelineClip
{
    juce::String name;
    int track = 0;
    double startBeat = 0.0;
    double lengthBeats = 4.0;
    double sourceOffsetBeats = 0.0;
    bool midi = false;
};

struct InstrumentTrack
{
    juce::String name;
    juce::String instrumentName;
    juce::String pluginFormat;
};

class ProjectModel final
{
public:
    ProjectModel();

    void setMidiClipStart(double beat) noexcept;
    double getMidiClipStart() const noexcept;
    void setTrackVolume(float volume) noexcept;
    float getTrackVolume() const noexcept;
    void setTrackPan(float pan) noexcept;
    float getTrackPan() const noexcept;
    void setTrackMuted(bool muted) noexcept;
    bool isTrackMuted() const noexcept;
    void setTrackSoloed(bool soloed) noexcept;
    bool isTrackSoloed() const noexcept;
    void setMasterVolume(float volume) noexcept;
    float getMasterVolume() const noexcept;
    void setMasterPan(float pan) noexcept;
    float getMasterPan() const noexcept;
    void setMasterMuted(bool muted) noexcept;
    bool isMasterMuted() const noexcept;

    const std::array<InstrumentTrack, 1> instrumentTracks {{
        { "MIDI Piano", "Soft Piano", "VST3" }
    }};

    std::vector<MidiNote> notes;
    std::array<TimelineClip, 1> clips {{
        { "Soft Piano", 0, 1.0, midiClipLengthBeats, 0.0, true }
    }};

private:
    std::atomic<double> midiClipStart { 1.0 };
    std::atomic<float> trackVolume { 0.72f };
    std::atomic<float> trackPan { 0.0f };
    std::atomic<bool> trackMuted { false };
    std::atomic<bool> trackSoloed { false };
    std::atomic<float> masterVolume { 0.86f };
    std::atomic<float> masterPan { 0.0f };
    std::atomic<bool> masterMuted { false };
};
}
