#include "TimelineController.h"
#include "core/ProjectModel.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace resona
{
namespace
{
constexpr double minimumTrackHeight = 0.85;
constexpr double maximumTrackHeight = 2.0;
constexpr double gridSizeBeats = 1.0;
}

void TimelineController::setViewport(double requestedStart, double requestedVisible) noexcept
{
    visibleBeats = std::clamp(requestedVisible, minimumVisibleBeats, maximumVisibleBeats);
    startBeat = std::clamp(requestedStart, 0.0, projectLengthBeats - visibleBeats);
}

void TimelineController::zoomTime(double anchorRatio, double wheelDelta) noexcept
{
    anchorRatio = std::clamp(anchorRatio, 0.0, 1.0);
    const auto anchorBeat = beatAtRatio(anchorRatio);
    const auto factor = std::pow(2.0, -wheelDelta * 2.0);
    const auto nextVisible = std::clamp(visibleBeats * factor,
                                        minimumVisibleBeats, maximumVisibleBeats);
    setViewport(anchorBeat - anchorRatio * nextVisible, nextVisible);
}

void TimelineController::zoomTrackHeight(double wheelDelta) noexcept
{
    trackHeightScale = std::clamp(trackHeightScale * std::pow(2.0, wheelDelta * 1.5),
                                  minimumTrackHeight, maximumTrackHeight);
}

void TimelineController::setTrackHeightScale(double scale) noexcept
{
    trackHeightScale = std::clamp(scale, minimumTrackHeight, maximumTrackHeight);
}

void TimelineController::returnViewToStart() noexcept { startBeat = 0.0; }
void TimelineController::setHorizontalScrollRatio(double ratio) noexcept
{
    startBeat = std::clamp(ratio, 0.0, 1.0) * (projectLengthBeats - visibleBeats);
}
double TimelineController::beatAtRatio(double ratio) const noexcept { return startBeat + ratio * visibleBeats; }
double TimelineController::ratioForBeat(double beat) const noexcept { return (beat - startBeat) / visibleBeats; }
double TimelineController::snapToGrid(double beat) const noexcept { return std::round(beat / gridSizeBeats) * gridSizeBeats; }
double TimelineController::getStartBeat() const noexcept { return startBeat; }
double TimelineController::getVisibleBeats() const noexcept { return visibleBeats; }
double TimelineController::getTrackHeightScale() const noexcept { return trackHeightScale; }
double TimelineController::getHorizontalScrollRatio() const noexcept
{
    const auto range = projectLengthBeats - visibleBeats;
    return range > 0.0 ? startBeat / range : 0.0;
}

void TimelineController::selectOnly(size_t index) { selection = { index }; }
void TimelineController::setSelection(std::set<size_t> indices) { selection = std::move(indices); }
void TimelineController::clearSelection() { selection.clear(); }
bool TimelineController::isSelected(size_t index) const { return selection.contains(index); }
size_t TimelineController::selectionSize() const noexcept { return selection.size(); }
}
