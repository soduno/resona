#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace resona
{
inline constexpr float uiScale = 0.82f;
inline constexpr int referenceWindowWidth = 1584;
inline constexpr int referenceWindowHeight = 992;
inline constexpr int scaledWindowWidth = static_cast<int>(referenceWindowWidth / uiScale + 0.5f);
inline constexpr int scaledWindowHeight = static_cast<int>(referenceWindowHeight / uiScale + 0.5f);

enum class PreviewMode
{
    normal,
    zoomed,
    zoomedOut,
    verticalZoomed,
    clickActive,
    midiEditor,
    mixer,
    narrowSidebar
};

std::unique_ptr<juce::Component> createMainComponent(PreviewMode preview = PreviewMode::normal);
}
