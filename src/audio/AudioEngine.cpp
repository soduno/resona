#include "AudioEngine.h"
#include <cmath>

namespace resona
{
AudioEngine::AudioEngine(ProjectModel& projectToUse, TransportController& transportToUse) noexcept
    : project(projectToUse), transport(transportToUse)
{
}

void AudioEngine::prepare(double sampleRate) noexcept
{
    currentSampleRate.store(sampleRate > 0.0 ? sampleRate : 44100.0);
}

void AudioEngine::render(const juce::AudioSourceChannelInfo& info) noexcept
{
    info.clearActiveBufferRegion();
    if (!transport.isPlaying())
    {
        trackPeak.store(trackPeak.load() * 0.72f);
        masterPeak.store(masterPeak.load() * 0.72f);
        return;
    }

    const auto beatPerSample = (tempoBpm / 60.0) / currentSampleRate.load();
    auto beat = transport.getPlayheadBeats();
    const auto clipStart = project.getMidiClipStart();
    const auto trackVolume = project.getTrackVolume();
    const auto trackPan = project.getTrackPan();
    const auto trackAudible = !project.isTrackMuted();
    const auto leftGain = trackPan > 0.0f ? 1.0f - trackPan : 1.0f;
    const auto rightGain = trackPan < 0.0f ? 1.0f + trackPan : 1.0f;
    const auto masterVolume = project.getMasterVolume();
    const auto masterPan = project.getMasterPan();
    const auto masterAudible = !project.isMasterMuted();
    const auto masterLeftGain = masterPan > 0.0f ? 1.0f - masterPan : 1.0f;
    const auto masterRightGain = masterPan < 0.0f ? 1.0f + masterPan : 1.0f;
    float blockPeak = 0.0f;
    float blockMasterPeak = 0.0f;

    for (int sample = 0; sample < info.numSamples; ++sample)
    {
        float trackOutput = 0.0f;
        const auto sourceBeat = beat >= clipStart && beat < clipStart + midiClipLengthBeats
                              ? beat - clipStart : -1.0;

        for (const auto& note : project.notes)
        {
            if (sourceBeat < note.beat || sourceBeat >= note.beat + note.length) continue;

            const auto localSeconds = (sourceBeat - note.beat) * 60.0 / tempoBpm;
            const auto noteSeconds = note.length * 60.0 / tempoBpm;
            const auto attack = juce::jlimit(0.0, 1.0, localSeconds / 0.012);
            const auto release = juce::jlimit(0.0, 1.0, (noteSeconds - localSeconds) / 0.04);
            const auto envelope = static_cast<float>(juce::jmin(attack, release));
            const auto frequency = 440.0 * std::pow(2.0, (note.note - 69) / 12.0);
            trackOutput += static_cast<float>(
                std::sin(juce::MathConstants<double>::twoPi * frequency * localSeconds)
                * note.velocity * envelope * 0.055);
        }

        trackOutput = trackAudible ? std::tanh(trackOutput) * trackVolume : 0.0f;
        blockPeak = juce::jmax(blockPeak, std::abs(trackOutput));
        auto clickOutput = 0.0f;

        if (transport.isClickTrackEnabled())
        {
            const auto beatNumber = std::floor(beat);
            const auto beatPhase = beat - beatNumber;
            constexpr double clickLengthBeats = 0.09;
            if (beatPhase < clickLengthBeats)
            {
                const auto clickSeconds = beatPhase * 60.0 / tempoBpm;
                const auto downbeat = static_cast<int>(beatNumber) % 4 == 0;
                const auto frequency = downbeat ? 1760.0 : 1180.0;
                const auto level = downbeat ? 0.20 : 0.13;
                clickOutput += static_cast<float>(
                    std::sin(juce::MathConstants<double>::twoPi * frequency * clickSeconds)
                    * level * std::exp(-clickSeconds * 72.0));
            }
        }

        const auto leftBus = trackOutput * leftGain + clickOutput;
        const auto rightBus = trackOutput * rightGain + clickOutput;
        const auto leftOutput = masterAudible
            ? std::tanh(leftBus * masterVolume * masterLeftGain) : 0.0f;
        const auto rightOutput = masterAudible
            ? std::tanh(rightBus * masterVolume * masterRightGain) : 0.0f;
        blockMasterPeak = juce::jmax(blockMasterPeak,
                                     juce::jmax(std::abs(leftOutput), std::abs(rightOutput)));
        const auto channels = info.buffer->getNumChannels();
        if (channels == 1)
            info.buffer->setSample(0, info.startSample + sample,
                                   0.5f * (leftOutput + rightOutput));
        else
        {
            info.buffer->setSample(0, info.startSample + sample, leftOutput);
            info.buffer->setSample(1, info.startSample + sample, rightOutput);
            for (int channel = 2; channel < channels; ++channel)
                info.buffer->setSample(channel, info.startSample + sample,
                                       0.5f * (leftOutput + rightOutput));
        }

        beat += beatPerSample;
        if (beat >= projectLengthBeats) beat -= projectLengthBeats;
    }

    transport.setPlayheadBeats(beat);
    trackPeak.store(juce::jmax(blockPeak, trackPeak.load() * 0.78f));
    masterPeak.store(juce::jmax(blockMasterPeak, masterPeak.load() * 0.78f));
}

float AudioEngine::getTrackPeak() const noexcept { return trackPeak.load(); }
float AudioEngine::getMasterPeak() const noexcept { return masterPeak.load(); }
}
