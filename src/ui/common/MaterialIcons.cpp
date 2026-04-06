#include "ui/common/MaterialIcons.h"
#include <BinaryData.h>
#include <array>
#include <memory>

namespace resona::material
{
namespace
{
struct IconData
{
    const void* data {};
    int size {};
};

IconData dataFor(Icon icon)
{
    using namespace ResonaMaterialIcons;
    switch (icon)
    {
        case Icon::add:           return { add_24px_svg, add_24px_svgSize };
        case Icon::arrowDropDown: return { arrow_drop_down_24px_svg, arrow_drop_down_24px_svgSize };
        case Icon::close:         return { close_24px_svg, close_24px_svgSize };
        case Icon::maximise:      return { crop_square_24px_svg, crop_square_24px_svgSize };
        case Icon::record:        return { fiber_manual_record_24px_svg, fiber_manual_record_24px_svgSize };
        case Icon::back:          return { keyboard_arrow_left_24px_svg, keyboard_arrow_left_24px_svgSize };
        case Icon::forward:       return { keyboard_arrow_right_24px_svg, keyboard_arrow_right_24px_svgSize };
        case Icon::minimise:      return { minimize_24px_svg, minimize_24px_svgSize };
        case Icon::clickTrack:    return { music_note_24px_svg, music_note_24px_svgSize };
        case Icon::piano:         return { piano_24px_svg, piano_24px_svgSize };
        case Icon::pause:         return { pause_24px_svg, pause_24px_svgSize };
        case Icon::play:          return { play_arrow_24px_svg, play_arrow_24px_svgSize };
        case Icon::settings:      return { settings_24px_svg, settings_24px_svgSize };
        case Icon::stop:          return { stop_24px_svg, stop_24px_svgSize };
        case Icon::mixer:         return { tune_24px_svg, tune_24px_svgSize };
        case Icon::timeline:      return { view_timeline_24px_svg, view_timeline_24px_svgSize };
        case Icon::volume:        return { volume_up_24px_svg, volume_up_24px_svgSize };
        case Icon::zoomIn:        return { zoom_in_24px_svg, zoom_in_24px_svgSize };
        case Icon::zoomOut:       return { zoom_out_24px_svg, zoom_out_24px_svgSize };
        case Icon::count:         break;
    }
    return {};
}

struct CachedIcon
{
    std::unique_ptr<juce::Drawable> drawable;
    juce::Colour colour { juce::Colours::black };
};
}

void draw(juce::Graphics& g, Icon icon, juce::Rectangle<float> bounds,
          juce::Colour colour, float opacity)
{
    static std::array<CachedIcon, static_cast<size_t>(Icon::count)> cache;
    const auto index = static_cast<size_t>(icon);
    auto& entry = cache[index];
    if (entry.drawable == nullptr)
    {
        const auto source = dataFor(icon);
        entry.drawable = juce::Drawable::createFromImageData(source.data,
                                                              static_cast<size_t>(source.size));
    }
    if (entry.drawable == nullptr) return;
    if (entry.colour != colour)
    {
        entry.drawable->replaceColour(entry.colour, colour);
        entry.colour = colour;
    }
    entry.drawable->drawWithin(g, bounds, juce::RectanglePlacement::centred,
                               juce::jlimit(0.0f, 1.0f, opacity));
}
}
