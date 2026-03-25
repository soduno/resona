#pragma once

#include <cstddef>
#include <set>

namespace resona
{
class TimelineController final
{
public:
    void setViewport(double startBeat, double visibleBeats) noexcept;
    void zoomTime(double anchorRatio, double wheelDelta) noexcept;
    void zoomTrackHeight(double wheelDelta) noexcept;
    void setTrackHeightScale(double scale) noexcept;
    void returnViewToStart() noexcept;
    void setHorizontalScrollRatio(double ratio) noexcept;

    double beatAtRatio(double ratio) const noexcept;
    double ratioForBeat(double beat) const noexcept;
    double snapToGrid(double beat) const noexcept;

    double getStartBeat() const noexcept;
    double getVisibleBeats() const noexcept;
    double getTrackHeightScale() const noexcept;
    double getHorizontalScrollRatio() const noexcept;

    void selectOnly(size_t index);
    void setSelection(std::set<size_t> indices);
    void clearSelection();
    bool isSelected(size_t index) const;
    size_t selectionSize() const noexcept;

private:
    double startBeat = 0.0;
    double visibleBeats = 32.0;
    double trackHeightScale = 1.0;
    std::set<size_t> selection;
};
}
