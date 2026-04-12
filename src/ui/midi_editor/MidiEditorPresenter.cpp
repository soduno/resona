#include "ui/midi_editor/MidiEditorPresenter.h"
#include "ui/common/Drawing.h"
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

juce::Rectangle<int> noteBounds(const MidiPaintState& s, int index, juce::Rectangle<int> grid)
{
    const auto& note = s.notes[(size_t) index];
    const auto rowHeight = (double) grid.getHeight() / s.editor.getVisibleNoteRows();
    const auto x = grid.getX() + juce::roundToInt(s.editor.ratioForBeat(note.beat) * grid.getWidth());
    const auto width = juce::jmax(5, juce::roundToInt(note.length / s.editor.getVisibleBeats() * grid.getWidth()));
    const auto y = grid.getY() + juce::roundToInt(s.editor.rowForNote(note.note) * rowHeight) + 2;
    return { x, y, width, juce::jmax(4, juce::roundToInt(rowHeight) - 4) };
}
}
void MidiEditorPresenter::paint(juce::Graphics& g, const MidiPaintState& s) const
{
    constexpr int velocityHeight = 150;
    auto area = s.area;
    const auto inspector = area.removeFromRight(s.inspectorWidth);
    if (s.verticalScrollbar) area.removeFromRight(s.scrollbarThickness);
    if (s.horizontalScrollbar) area.removeFromBottom(s.scrollbarThickness);
    auto velocity = area.removeFromBottom(velocityHeight);
    const auto keyboardColumn = area.removeFromLeft(s.keyboardWidth);
    const auto noteArea = area;
    const auto ruler = juce::Rectangle<int>(noteArea.getX(), noteArea.getY() + s.headerHeight,
                                             noteArea.getWidth(), s.rulerHeight);
    const auto grid = noteArea.withTrimmedTop(s.headerHeight + s.rulerHeight);
    const auto keyboard = juce::Rectangle<int>(keyboardColumn.getX(), grid.getY(), keyboardColumn.getWidth(), grid.getHeight());
    g.setColour(palette::panel);
    g.fillRect(keyboardColumn);
    g.fillRect(inspector);
    g.setColour(palette::text);
    g.setFont(18.0f);
    g.drawText("<   Keys - Soft Piano", noteArea.getX() + 14, noteArea.getY() + 8, 260, 32, juce::Justification::centredLeft);
    g.setColour(palette::muted);
    g.setFont(13.0f);
    g.drawText("Select     Draw     Split     Erase                         1/16     Quantize",
               noteArea.getX() + 290, noteArea.getY() + 8, noteArea.getWidth() - 300, 32, juce::Justification::centredRight);
    g.setColour(palette::background.darker(0.32f));
    g.fillRect(ruler);
    g.fillRect(juce::Rectangle<int>(keyboardColumn.getX(), ruler.getY(),
                                    keyboardColumn.getWidth(), ruler.getHeight()));
    g.setColour(palette::line.brighter(0.08f));
    g.drawHorizontalLine(ruler.getBottom() - 1, (float) keyboardColumn.getX(), (float) ruler.getRight());

    const auto rowHeight = (float) (grid.getHeight() / s.editor.getVisibleNoteRows());
    const auto rows = juce::roundToInt(std::ceil(s.editor.getVisibleNoteRows()));
    for (int row = 0; row <= rows; ++row)
    {
        const auto y = grid.getY() + juce::roundToInt(row * rowHeight);
        const auto midi = juce::roundToInt(std::floor(s.editor.getTopNote() - row));
        const auto pc = (midi % 12 + 12) % 12;
        const auto black = pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
        g.setColour(black ? palette::grid.darker(0.16f) : palette::background);
        g.fillRect(grid.getX(), y, grid.getWidth(), juce::roundToInt(rowHeight));
        g.setColour(palette::grid);
        g.drawHorizontalLine(y, (float) grid.getX(), (float) grid.getRight());
        g.setColour(black ? palette::raised : palette::text);
        g.fillRect(keyboard.getX(), y, keyboard.getWidth(), juce::roundToInt(rowHeight));
        if (midi % 12 == 0)
        {
            g.setColour(black ? palette::text : palette::background);
            g.setFont(10.0f);
            g.drawText("C" + juce::String(midi / 12 - 1), keyboard.getX() + 5, y,
                       keyboard.getWidth() - 10, juce::roundToInt(rowHeight), juce::Justification::centredRight);
        }
    }
    const auto start = s.editor.getStartBeat(), beats = s.editor.getVisibleBeats();
    const auto subdivision = beats <= 8.0 ? 0.25 : beats <= 16.0 ? 0.5 : 1.0;
    for (auto beat = std::floor(start / subdivision) * subdivision;
         beat <= start + beats + 0.0001; beat += subdivision)
    {
        if (beat < start - 0.0001) continue;
        const auto x = grid.getX() + juce::roundToInt(s.editor.ratioForBeat(beat) * grid.getWidth());
        const auto whole = std::abs(beat - std::round(beat)) < 0.0001;
        const auto bar = whole && juce::roundToInt(beat) % 4 == 0;
        g.setColour(bar ? palette::line : palette::grid);
        g.drawVerticalLine(x, (float) ruler.getY(), (float) velocity.getBottom());
        if (bar)
        {
            g.setColour(palette::muted);
            g.setFont(12.0f);
            g.drawText(juce::String(juce::roundToInt(beat) + 1), x + 8, ruler.getY(), 40,
                       ruler.getHeight() - 18, juce::Justification::centredLeft);
        }
    }
    g.saveState();
    g.reduceClipRegion(grid);
    for (int i = 0; i < (int) s.notes.size(); ++i)
    {
        const auto& note = s.notes[(size_t) i];
        const auto bounds = noteBounds(s, i, grid);
        const auto selected = s.editor.isSelected((size_t) i);
        g.setColour(selected ? palette::violetHeader.brighter(0.22f)
                             : note.note >= 64 ? palette::violet.brighter(0.1f) : palette::violet);
        g.fillRoundedRectangle(bounds.toFloat(), 3.0f);
        if (selected)
        {
            g.setColour(palette::text.withAlpha(0.9f));
            g.drawRoundedRectangle(bounds.toFloat().reduced(0.75f), 3.0f, 1.5f);
        }
    }
    g.restoreState();
    g.setColour(palette::panel);
    g.fillRect(velocity);
    g.setColour(palette::line);
    g.drawHorizontalLine(velocity.getY(), (float) velocity.getX(), (float) velocity.getRight());
    g.setColour(palette::text);
    g.setFont(13.0f);
    g.drawText("Velocity", velocity.getX() + 12, velocity.getY() + 8, 80, 24, juce::Justification::centredLeft);
    const auto velocityGrid = velocity.withTrimmedLeft(s.keyboardWidth);
    g.saveState();
    g.reduceClipRegion(velocityGrid);
    for (size_t i = 0; i < s.notes.size(); ++i)
    {
        const auto& note = s.notes[i];
        const auto x = velocityGrid.getX() + juce::roundToInt(s.editor.ratioForBeat(note.beat) * velocityGrid.getWidth());
        const auto height = juce::roundToInt(note.velocity * (velocity.getHeight() - 42));
        g.setColour(s.editor.isSelected(i) ? palette::text : palette::mint);
        g.fillRoundedRectangle((float) x, (float) velocity.getBottom() - height, 4.0f, (float) height, 2.0f);
    }
    g.restoreState();
    const auto sourceBeat = s.transport.getPlayheadBeats() - s.project.getMidiClipStart();
    if (sourceBeat >= start && sourceBeat <= start + beats)
    {
        const auto x = ruler.getX() + juce::roundToInt(s.editor.ratioForBeat(sourceBeat) * ruler.getWidth());
        drawing::playheadMarker(g, { ruler.getX(), ruler.getY(), ruler.getWidth(), velocity.getBottom() - ruler.getY() }, x);
    }
    paintMarquee(g, s.marquee, s.marqueeVisible);
    if (!s.inspectorCollapsed) inspectorPresenter.paintNotes(g, inspector);
}
}
