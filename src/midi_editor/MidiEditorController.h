#pragma once

#include <cstddef>
#include <set>

namespace resona
{
class MidiEditorController final
{
public:
    void zoomTime(double anchorRatio, double wheelDelta) noexcept;
    void zoomPitch(double anchorRatio, double wheelDelta) noexcept;
    void setViewport(double startBeat, double visibleBeats, double topNote,
                     double visibleNoteRows) noexcept;
    void setHorizontalScrollRatio(double ratio) noexcept;
    void setVerticalScrollRatio(double ratio) noexcept;

    double beatAtRatio(double ratio) const noexcept;
    double ratioForBeat(double beat) const noexcept;
    double rowForNote(int midiNote) const noexcept;
    double snapToGrid(double beat) const noexcept;

    double getStartBeat() const noexcept;
    double getVisibleBeats() const noexcept;
    double getTopNote() const noexcept;
    double getVisibleNoteRows() const noexcept;
    double getHorizontalScrollRatio() const noexcept;
    double getVerticalScrollRatio() const noexcept;

    void selectOnly(size_t index);
    void toggleSelection(size_t index);
    void selectAll(size_t noteCount);
    void setSelection(std::set<size_t> indices);
    void clearSelection();
    bool isSelected(size_t index) const;
    size_t selectionSize() const noexcept;

private:
    double startBeat = 0.0;
    double visibleBeats = 32.0;
    double topNote = 72.0;
    double visibleNoteRows = 36.0;
    std::set<size_t> selection;
};
}
