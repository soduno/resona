#include "ui/timeline/TimelinePresenter.h"
#include "ui/common/Drawing.h"
#include "ui/common/MaterialIcons.h"
#include "ui/inspector/InspectorPresenter.h"
#include "ui/common/Theme.h"
#include <cmath>

namespace resona
{
namespace
{
void paintMarquee(juce::Graphics& g, juce::Rectangle<int> bounds, bool visible)
{
    if (!visible || bounds.isEmpty()) return;
    g.setColour(palette::violet.withAlpha(0.16f));
    g.fillRect(bounds);
    g.setColour(palette::violet.brighter(0.2f));
    g.drawRect(bounds, 1);
}
}
void TimelinePresenter::paint(juce::Graphics& g, const TimelinePaintState& s) const
{
    auto area = s.area;
    const auto trackList = area.removeFromLeft(s.leftSidebarWidth);
    const auto inspector = area.removeFromRight(s.inspectorWidth);
    if (s.verticalScrollbar) area.removeFromRight(s.scrollbarThickness);
    if (s.horizontalScrollbar) area.removeFromBottom(s.scrollbarThickness);

    g.setColour(palette::panel);
    g.fillRect(trackList);
    g.fillRect(inspector);
    g.setColour(palette::line);
    g.drawVerticalLine(trackList.getRight(), (float) trackList.getY(), (float) trackList.getBottom());
    g.drawVerticalLine(inspector.getX(), (float) inspector.getY(), (float) inspector.getBottom());
    g.drawHorizontalLine(area.getY() + s.rulerHeight, (float) trackList.getX(), (float) trackList.getRight());

    const auto y = area.getY() + s.rulerHeight;
    g.setColour(palette::violetHeader);
    g.fillRect(trackList.getX() + 7, y, 13, s.laneHeight);
    g.setColour(palette::line);
    g.drawHorizontalLine(y + s.laneHeight, (float) trackList.getX(), (float) trackList.getRight());
    const auto compact = trackList.getWidth() < 205;
    const auto iconSize = compact ? 30 : 40;
    const auto controlsX = trackList.getX() + (compact ? 64 : 108);
    const auto buttonWidth = compact ? 30 : 39;
    const auto gap = compact ? 5 : 11;
    const auto controlsRight = trackList.getRight() - (compact ? 8 : 24);
    drawing::trackIcon(g, { trackList.getX() + (compact ? 26 : 42), y + (compact ? 30 : 34), iconSize, iconSize }, 0);
    g.setColour(palette::text);
    g.setFont(compact ? 14.0f : 17.0f);
    g.drawText("MIDI Piano", controlsX, y + 22, juce::jmax(20, controlsRight - controlsX), 32,
               juce::Justification::centredLeft, true);
    drawing::smallButton(g, { controlsX, y + 62, buttonWidth, 36 }, "M",
                         s.project.isTrackMuted());
    drawing::smallButton(g, { controlsX + buttonWidth + gap, y + 62, buttonWidth, 36 }, "S",
                         s.project.isTrackSoloed());

    const auto panLeft = controlsX;
    const auto panWidth = juce::jmax(24, controlsRight - controlsX);
    const auto panY = y + 109;
    const auto panX = panLeft + juce::roundToInt(
        (s.project.getTrackPan() + 1.0f) * 0.5f * panWidth);
    g.setColour(palette::grid);
    g.fillRoundedRectangle((float) panLeft, (float) panY - 2.0f,
                           (float) panWidth, 4.0f, 2.0f);
    g.setColour(palette::muted.withAlpha(0.7f));
    g.drawVerticalLine(panLeft + panWidth / 2, (float) panY - 5.0f, (float) panY + 5.0f);
    g.setColour(palette::violet);
    g.fillEllipse((float) panX - 5.0f, (float) panY - 5.0f, 10.0f, 10.0f);

    const auto knobX = s.volumeSlider.getX() + juce::roundToInt(s.volumeSlider.getWidth() * s.project.getTrackVolume());
    g.setColour(palette::line);
    g.fillRoundedRectangle((float) s.volumeSlider.getX(), (float) s.volumeSlider.getCentreY() - 3.0f,
                           (float) s.volumeSlider.getWidth(), 7.0f, 3.5f);
    g.setColour(palette::violet);
    g.fillRoundedRectangle((float) s.volumeSlider.getX(), (float) s.volumeSlider.getCentreY() - 3.0f,
                           (float) knobX - s.volumeSlider.getX(), 7.0f, 3.5f);
    g.fillEllipse((float) knobX - 9.0f, (float) s.volumeSlider.getCentreY() - 9.0f, 18.0f, 18.0f);
    const auto peak = juce::jlimit(0.0f, 1.0f, std::sqrt(s.peak * 3.0f));
    const auto meterWidth = juce::roundToInt(s.meter.getWidth() * peak);
    for (int channel = 0; channel < 2; ++channel)
    {
        const auto bar = juce::Rectangle<int>(s.meter.getX(), s.meter.getY() + channel * 6, s.meter.getWidth(), 4);
        g.setColour(palette::grid);
        g.fillRoundedRectangle(bar.toFloat(), 2.0f);
        if (meterWidth > 0)
        {
            g.setColour(peak > 0.88f ? palette::red : palette::mint);
            g.fillRoundedRectangle(bar.withWidth(meterWidth).toFloat(), 2.0f);
        }
    }
    g.setColour(palette::muted);
    g.setFont(16.0f);
    material::draw(g, material::Icon::add,
                   { (float) trackList.getX() + 27.0f,
                     (float) y + s.laneHeight + 30.0f, 20.0f, 20.0f }, palette::muted);
    g.drawText("Add Track", trackList.getX() + 54, y + s.laneHeight + 20,
               juce::jmax(30, trackList.getWidth() - 68), 42,
               juce::Justification::centredLeft, true);

    auto ruler = area.removeFromTop(s.rulerHeight);
    g.setColour(palette::background.darker(0.32f));
    g.fillRect(ruler);
    g.setColour(palette::line.brighter(0.08f));
    g.drawHorizontalLine(ruler.getBottom() - 1, (float) ruler.getX(), (float) ruler.getRight());
    const auto start = s.timeline.getStartBeat(), beats = s.timeline.getVisibleBeats();
    const auto subdivision = beats <= 8.0 ? 0.25 : beats <= 16.0 ? 0.5 : 1.0;
    for (auto beat = std::floor(start / subdivision) * subdivision;
         beat <= start + beats + 0.0001; beat += subdivision)
    {
        if (beat < start - 0.0001) continue;
        const auto x = ruler.getX() + juce::roundToInt(s.timeline.ratioForBeat(beat) * ruler.getWidth());
        const auto whole = std::abs(beat - std::round(beat)) < 0.0001;
        const auto bar = whole && juce::roundToInt(beat) % 4 == 0;
        g.setColour(bar ? palette::line : palette::grid);
        g.drawVerticalLine(x, (float) ruler.getY(), (float) area.getBottom());
        if (bar && beat < s.projectLengthBeats)
        {
            g.setColour(palette::muted);
            g.setFont(12.0f);
            g.drawText(juce::String(juce::roundToInt(beat) + 1), x + 8, ruler.getY(), 40,
                       ruler.getHeight() - 18, juce::Justification::centredLeft);
        }
    }
    g.setColour(palette::line);
    g.drawHorizontalLine(area.getY() + s.laneHeight, (float) area.getX(), (float) area.getRight());
    g.saveState();
    g.reduceClipRegion(area);
    for (size_t i = 0; i < s.clips.size(); ++i)
    {
        const auto& clip = s.clips[i];
        const auto x = area.getX() + juce::roundToInt(s.timeline.ratioForBeat(clip.startBeat) * area.getWidth());
        const auto width = juce::jmax(10, juce::roundToInt(clip.lengthBeats / beats * area.getWidth()));
        const auto bounds = juce::Rectangle<int>(x, area.getY() + clip.track * s.laneHeight + 16,
                                                  width, s.laneHeight - 32);
        drawing::clip(g, bounds, clip.name, clip.track == 1 ? palette::mintBody : palette::violetSoft,
                      clip.midi, s.timeline.isSelected(i), s.notes, s.midiClipLengthBeats);
    }
    g.restoreState();
    const auto playhead = s.transport.getPlayheadBeats();
    if (playhead >= start && playhead <= start + beats)
    {
        const auto x = area.getX() + juce::roundToInt(s.timeline.ratioForBeat(playhead) * area.getWidth());
        drawing::playheadMarker(g, { area.getX(), ruler.getY(), area.getWidth(), area.getBottom() - ruler.getY() }, x);
    }
    paintMarquee(g, s.marquee, s.marqueeVisible);
    if (!s.inspectorCollapsed) inspectorPresenter.paintInstrument(g, inspector);
}
}
