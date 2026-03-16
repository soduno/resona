#include "ProjectModel.h"

namespace resona
{
ProjectModel::ProjectModel()
{
    constexpr std::array<std::array<int, 3>, 8> chords {{
        {{ 48, 52, 55 }}, {{ 45, 48, 52 }}, {{ 43, 47, 50 }}, {{ 48, 52, 55 }},
        {{ 41, 45, 48 }}, {{ 43, 47, 50 }}, {{ 45, 48, 52 }}, {{ 48, 52, 55 }}
    }};
    constexpr std::array<int, 16> melody {{
        67, 69, 71, 67, 64, 67, 69, 72, 71, 69, 67, 64, 62, 64, 67, 64
    }};

    for (int bar = 0; bar < 7; ++bar)
        for (int note : chords[static_cast<size_t>(bar)])
            notes.push_back({ bar * 4.0, 3.85, note, 0.55f });

    for (int i = 0; i < 14; ++i)
        notes.push_back({ i * 2.0, i % 4 == 0 ? 1.6 : 0.85,
                          melody[static_cast<size_t>(i)], 0.72f + (i % 3) * 0.08f });
}

void ProjectModel::setMidiClipStart(double beat) noexcept
{
    midiClipStart.store(beat);
}

double ProjectModel::getMidiClipStart() const noexcept
{
    return midiClipStart.load();
}

void ProjectModel::setTrackVolume(float volume) noexcept
{
    trackVolume.store(juce::jlimit(0.0f, 1.0f, volume));
}

float ProjectModel::getTrackVolume() const noexcept
{
    return trackVolume.load();
}

void ProjectModel::setTrackPan(float pan) noexcept
{
    trackPan.store(juce::jlimit(-1.0f, 1.0f, pan));
}

float ProjectModel::getTrackPan() const noexcept
{
    return trackPan.load();
}

void ProjectModel::setTrackMuted(bool muted) noexcept
{
    trackMuted.store(muted);
}

bool ProjectModel::isTrackMuted() const noexcept
{
    return trackMuted.load();
}

void ProjectModel::setTrackSoloed(bool soloed) noexcept
{
    trackSoloed.store(soloed);
}

bool ProjectModel::isTrackSoloed() const noexcept
{
    return trackSoloed.load();
}

void ProjectModel::setMasterVolume(float volume) noexcept
{
    masterVolume.store(juce::jlimit(0.0f, 1.0f, volume));
}

float ProjectModel::getMasterVolume() const noexcept
{
    return masterVolume.load();
}

void ProjectModel::setMasterPan(float pan) noexcept
{
    masterPan.store(juce::jlimit(-1.0f, 1.0f, pan));
}

float ProjectModel::getMasterPan() const noexcept
{
    return masterPan.load();
}

void ProjectModel::setMasterMuted(bool muted) noexcept
{
    masterMuted.store(muted);
}

bool ProjectModel::isMasterMuted() const noexcept
{
    return masterMuted.load();
}
}
