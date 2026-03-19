#include "MidiEditorController.h"
#include "core/ProjectModel.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace resona
{
namespace
{
constexpr double minimumVisibleNotes = 12.0;
constexpr double maximumVisibleNotes = 72.0;
constexpr double sixteenthNoteInBeats = 0.25;
}

void MidiEditorController::zoomTime(double anchorRatio, double wheelDelta) noexcept
{
    anchorRatio = std::clamp(anchorRatio, 0.0, 1.0);
    const auto anchorBeat = beatAtRatio(anchorRatio);
    const auto nextVisible = std::clamp(visibleBeats * std::pow(2.0, -wheelDelta * 2.0),
                                        minimumVisibleBeats, maximumVisibleBeats);
    visibleBeats = nextVisible;
    startBeat = std::clamp(anchorBeat - anchorRatio * visibleBeats,
                           0.0, projectLengthBeats - visibleBeats);
}

void MidiEditorController::zoomPitch(double anchorRatio, double wheelDelta) noexcept
{
    anchorRatio = std::clamp(anchorRatio, 0.0, 1.0);
    const auto anchorNote = topNote - anchorRatio * visibleNoteRows;
    visibleNoteRows = std::clamp(visibleNoteRows * std::pow(2.0, -wheelDelta * 1.5),
                                 minimumVisibleNotes, maximumVisibleNotes);
    topNote = std::clamp(anchorNote + anchorRatio * visibleNoteRows,
                         visibleNoteRows, 127.0);
}

void MidiEditorController::setViewport(double requestedStart, double requestedVisible,
                                       double requestedTop, double requestedRows) noexcept
{
    visibleBeats = std::clamp(requestedVisible, minimumVisibleBeats, maximumVisibleBeats);
    startBeat = std::clamp(requestedStart, 0.0, projectLengthBeats - visibleBeats);
    visibleNoteRows = std::clamp(requestedRows, minimumVisibleNotes, maximumVisibleNotes);
    topNote = std::clamp(requestedTop, visibleNoteRows, 127.0);
}

void MidiEditorController::setHorizontalScrollRatio(double ratio) noexcept
{
    startBeat = std::clamp(ratio, 0.0, 1.0) * (projectLengthBeats - visibleBeats);
}

void MidiEditorController::setVerticalScrollRatio(double ratio) noexcept
{
    topNote = 127.0 - std::clamp(ratio, 0.0, 1.0) * (127.0 - visibleNoteRows);
}

double MidiEditorController::beatAtRatio(double ratio) const noexcept { return startBeat + ratio * visibleBeats; }
double MidiEditorController::ratioForBeat(double beat) const noexcept { return (beat - startBeat) / visibleBeats; }
double MidiEditorController::rowForNote(int midiNote) const noexcept { return topNote - midiNote; }
double MidiEditorController::snapToGrid(double beat) const noexcept
{
    return std::round(beat / sixteenthNoteInBeats) * sixteenthNoteInBeats;
}
double MidiEditorController::getStartBeat() const noexcept { return startBeat; }
double MidiEditorController::getVisibleBeats() const noexcept { return visibleBeats; }
double MidiEditorController::getTopNote() const noexcept { return topNote; }
double MidiEditorController::getVisibleNoteRows() const noexcept { return visibleNoteRows; }
double MidiEditorController::getHorizontalScrollRatio() const noexcept
{
    const auto range = projectLengthBeats - visibleBeats;
    return range > 0.0 ? startBeat / range : 0.0;
}
double MidiEditorController::getVerticalScrollRatio() const noexcept
{
    const auto range = 127.0 - visibleNoteRows;
    return range > 0.0 ? (127.0 - topNote) / range : 0.0;
}

void MidiEditorController::selectOnly(size_t index) { selection = { index }; }
void MidiEditorController::toggleSelection(size_t index)
{
    if (selection.contains(index)) selection.erase(index); else selection.insert(index);
}
void MidiEditorController::selectAll(size_t noteCount)
{
    selection.clear();
    for (size_t i = 0; i < noteCount; ++i) selection.insert(i);
}
void MidiEditorController::setSelection(std::set<size_t> indices) { selection = std::move(indices); }
void MidiEditorController::clearSelection() { selection.clear(); }
bool MidiEditorController::isSelected(size_t index) const { return selection.contains(index); }
size_t MidiEditorController::selectionSize() const noexcept { return selection.size(); }
}
