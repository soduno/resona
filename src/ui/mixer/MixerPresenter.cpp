#include "ui/mixer/MixerPresenter.h"
#include "ui/common/Drawing.h"
#include "ui/common/MaterialIcons.h"
#include "ui/common/Theme.h"
#include "ui/inspector/InspectorPresenter.h"
#include <cmath>

namespace resona
{
namespace
{
constexpr int labelWidth = 49;

juce::Rectangle<int> stripBounds(juce::Rectangle<int> area, int inspectorWidth,
                                 int index, int stripWidth)
{
    area.removeFromRight(inspectorWidth);
    return { area.getX() + index * stripWidth, area.getY(), stripWidth, area.getHeight() };
}

juce::Rectangle<int> footerBounds(juce::Rectangle<int> strip)
{
    const auto surface = strip.reduced(5, 4);
    return { surface.getX(), surface.getBottom() - 30, surface.getWidth(), 30 };
}

juce::Rectangle<int> muteBounds(juce::Rectangle<int> strip)
{
    const auto footer = footerBounds(strip);
    return { strip.getCentreX() - 40, footer.getY() - 30, 32, 24 };
}

juce::Rectangle<int> soloBounds(juce::Rectangle<int> strip)
{
    const auto footer = footerBounds(strip);
    return { strip.getCentreX() + 8, footer.getY() - 30, 32, 24 };
}

juce::Rectangle<int> panBounds(juce::Rectangle<int> strip)
{
    return { strip.getX() + labelWidth + 8, strip.getY() + 226,
             strip.getWidth() - labelWidth - 20, 52 };
}

juce::Rectangle<int> faderTrack(juce::Rectangle<int> strip)
{
    const auto top = strip.getY() + 338;
    const auto bottom = muteBounds(strip).getY() - 12;
    return { strip.getCentreX() - 5, top, 10, juce::jmax(40, bottom - top) };
}

void drawSectionLabel(juce::Graphics& g, juce::Rectangle<int> surface,
                      int y, const juce::String& text)
{
    g.setColour(palette::muted.brighter(0.12f).withAlpha(0.96f));
    g.setFont(11.5f);
    g.drawText(text, surface.getX() + 5, surface.getY() + y,
               labelWidth - 10, 24, juce::Justification::centredRight);
}

void drawValueBox(juce::Graphics& g, juce::Rectangle<int> bounds,
                  const juce::String& text, juce::Colour accent,
                  bool prominent = false)
{
    g.setColour(prominent ? accent.withAlpha(0.22f) : palette::background.withAlpha(0.72f));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(prominent ? accent.withAlpha(0.55f) : palette::line.withAlpha(0.75f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 4.0f, 1.0f);
    g.setColour(prominent ? palette::text : palette::muted);
    g.setFont(11.5f);
    auto textBounds = bounds.reduced(7, 0);
    auto displayText = text;
    if (text.startsWith("+ "))
    {
        material::draw(g, material::Icon::add,
                       { (float) textBounds.getX(), (float) textBounds.getCentreY() - 7.0f,
                         14.0f, 14.0f }, palette::muted);
        textBounds.removeFromLeft(18);
        displayText = text.substring(2);
    }
    g.drawText(displayText, textBounds, juce::Justification::centredLeft, true);
}

void drawPanKnob(juce::Graphics& g, juce::Rectangle<int> bounds,
                 float pan, juce::Colour accent)
{
    const auto knob = bounds.withSizeKeepingCentre(42, 42).toFloat();
    const auto centre = knob.getCentre();

    // A restrained hardware-like shadow and rim keep the control consistent
    // with Resona while giving it enough depth to read as a rotary control.
    g.setColour(juce::Colours::black.withAlpha(0.42f));
    g.fillEllipse(knob.translated(0.0f, 2.5f).expanded(2.0f));

    juce::ColourGradient rim(palette::raised.brighter(0.28f), knob.getX(), knob.getY(),
                             palette::background.darker(0.35f), knob.getRight(),
                             knob.getBottom(), false);
    g.setGradientFill(rim);
    g.fillEllipse(knob);
    g.setColour(palette::line.brighter(0.20f));
    g.drawEllipse(knob.reduced(0.5f), 1.0f);

    const auto face = knob.reduced(4.0f);
    juce::ColourGradient faceGradient(palette::raised.brighter(0.13f),
                                      face.getX() + 8.0f, face.getY() + 6.0f,
                                      palette::raised.darker(0.28f),
                                      face.getRight(), face.getBottom(), false);
    g.setGradientFill(faceGradient);
    g.fillEllipse(face);
    juce::Path faceHighlight;
    faceHighlight.addCentredArc(face.getCentreX(), face.getCentreY(),
                                face.getWidth() * 0.5f - 1.0f,
                                face.getHeight() * 0.5f - 1.0f, 0.0f,
                                juce::MathConstants<float>::pi * 1.10f,
                                juce::MathConstants<float>::pi * 1.92f, true);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.strokePath(faceHighlight, juce::PathStrokeType(1.4f));

    constexpr auto startAngle = juce::MathConstants<float>::pi * 1.25f;
    constexpr auto sweep = juce::MathConstants<float>::pi * 1.5f;
    const auto normalised = (juce::jlimit(-1.0f, 1.0f, pan) + 1.0f) * 0.5f;
    const auto angle = startAngle + normalised * sweep;

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, 23.0f, 23.0f, 0.0f,
                           startAngle, angle, true);
    g.setColour(accent.brighter(0.08f));
    g.strokePath(valueArc, juce::PathStrokeType(2.2f,
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // Build the indicator at twelve o'clock, then rotate the complete shape
    // around the knob centre. This keeps the stripe attached to the knob face
    // throughout the full -135 to +135 degree pan range.
    const auto indicatorRotation = (normalised - 0.5f) * sweep;
    juce::Path indicatorShadow;
    indicatorShadow.addRoundedRectangle(centre.x - 1.8f, centre.y - 15.0f,
                                        3.6f, 11.5f, 1.8f);
    indicatorShadow.applyTransform(
        juce::AffineTransform::rotation(indicatorRotation, centre.x, centre.y));
    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.fillPath(indicatorShadow, juce::AffineTransform::translation(0.8f, 1.0f));

    juce::Path indicator;
    indicator.addRoundedRectangle(centre.x - 1.25f, centre.y - 15.0f,
                                  2.5f, 11.5f, 1.25f);
    indicator.applyTransform(
        juce::AffineTransform::rotation(indicatorRotation, centre.x, centre.y));
    g.setColour(palette::text.withAlpha(0.96f));
    g.fillPath(indicator);
    g.setColour(accent.withAlpha(0.45f));
    g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.16f));
    g.fillEllipse(face.getX() + 7.0f, face.getY() + 5.0f, 5.0f, 3.0f);
}

void drawEq(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour accent)
{
    g.setColour(palette::background.withAlpha(0.78f));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(palette::grid.withAlpha(0.72f));
    for (int column = 1; column < 4; ++column)
        g.drawVerticalLine(bounds.getX() + column * bounds.getWidth() / 4,
                           (float) bounds.getY() + 4.0f, (float) bounds.getBottom() - 4.0f);
    g.drawHorizontalLine(bounds.getCentreY(), (float) bounds.getX() + 4.0f,
                         (float) bounds.getRight() - 4.0f);
    juce::Path response;
    response.startNewSubPath((float) bounds.getX() + 4.0f, (float) bounds.getCentreY() + 4.0f);
    response.cubicTo((float) bounds.getX() + bounds.getWidth() * 0.28f,
                     (float) bounds.getCentreY() + 4.0f,
                     (float) bounds.getX() + bounds.getWidth() * 0.38f,
                     (float) bounds.getCentreY() - 6.0f,
                     (float) bounds.getX() + bounds.getWidth() * 0.52f,
                     (float) bounds.getCentreY() - 3.0f);
    response.cubicTo((float) bounds.getX() + bounds.getWidth() * 0.70f,
                     (float) bounds.getCentreY() + 2.0f,
                     (float) bounds.getX() + bounds.getWidth() * 0.82f,
                     (float) bounds.getCentreY() + 3.0f,
                     (float) bounds.getRight() - 4.0f,
                     (float) bounds.getCentreY() - 1.0f);
    g.setColour(accent.withAlpha(0.9f));
    g.strokePath(response, juce::PathStrokeType(1.4f));
}
}

void MixerPresenter::setChannelStripWidth(int width) noexcept
{
    channelStripWidth = juce::jlimit(minimumChannelStripWidth,
                                     maximumChannelStripWidth, width);
}

juce::Rectangle<int> MixerPresenter::faderHitArea(juce::Rectangle<int> area,
                                                   int inspectorWidth, int trackIndex) const
{
    return faderTrack(stripBounds(area, inspectorWidth, trackIndex,
                                  channelStripWidth)).expanded(30, 12);
}

juce::Rectangle<int> MixerPresenter::panHitArea(juce::Rectangle<int> area,
                                                int inspectorWidth, int trackIndex) const
{
    return panBounds(stripBounds(area, inspectorWidth, trackIndex, channelStripWidth));
}

juce::Rectangle<int> MixerPresenter::muteButtonArea(juce::Rectangle<int> area,
                                                     int inspectorWidth, int trackIndex) const
{
    return muteBounds(stripBounds(area, inspectorWidth, trackIndex, channelStripWidth));
}

juce::Rectangle<int> MixerPresenter::soloButtonArea(juce::Rectangle<int> area,
                                                     int inspectorWidth, int trackIndex) const
{
    return soloBounds(stripBounds(area, inspectorWidth, trackIndex, channelStripWidth));
}

juce::Rectangle<int> MixerPresenter::stripResizeHandleArea(juce::Rectangle<int> area,
                                                            int inspectorWidth,
                                                            int trackIndex) const
{
    const auto strip = stripBounds(area, inspectorWidth, trackIndex, channelStripWidth);
    return { strip.getRight() - 4, strip.getY(), 8, strip.getHeight() };
}

float MixerPresenter::volumeForY(juce::Rectangle<int> area, int inspectorWidth,
                                 int y, int trackIndex) const
{
    const auto fader = faderTrack(stripBounds(area, inspectorWidth, trackIndex,
                                              channelStripWidth));
    return juce::jlimit(0.0f, 1.0f,
        (float) (fader.getBottom() - y) / (float) fader.getHeight());
}

float MixerPresenter::panForX(juce::Rectangle<int> area, int inspectorWidth,
                              int x, int trackIndex) const
{
    const auto bounds = panBounds(stripBounds(area, inspectorWidth, trackIndex,
                                              channelStripWidth));
    return juce::jlimit(-1.0f, 1.0f,
        2.0f * (float) (x - bounds.getX()) / (float) bounds.getWidth() - 1.0f);
}

void MixerPresenter::paint(juce::Graphics& g, juce::Rectangle<int> area,
                           int inspectorWidth, bool inspectorCollapsed,
                           const MixerController& mixer, float trackPeak,
                           float masterPeak) const
{
    const auto inspector = area.removeFromRight(inspectorWidth);
    if (inspectorWidth > 0)
    {
        g.setColour(palette::panel);
        g.fillRect(inspector);
        g.setColour(palette::line);
        g.drawVerticalLine(inspector.getX(), (float) inspector.getY(), (float) inspector.getBottom());
    }

    for (int i = 0; i < (int) mixer.channelCount(); ++i)
    {
        const auto index = (size_t) i;
        const auto& channel = mixer.channel(index);
        const auto master = mixer.isMasterChannel(index);
        const auto accent = master ? palette::mint : palette::violet;
        const auto strip = stripBounds(area, 0, i, channelStripWidth);
        const auto surface = strip.reduced(5, 4);

        g.setColour(master ? palette::mint.withAlpha(0.07f) : palette::panel.brighter(0.012f));
        g.fillRoundedRectangle(surface.toFloat(), 6.0f);
        g.setColour(master ? palette::mint.withAlpha(0.30f) : palette::line.brighter(0.035f));
        g.drawRoundedRectangle(surface.toFloat().reduced(0.5f), 6.0f, 1.0f);

        const auto valueX = surface.getX() + labelWidth;
        const auto valueWidth = surface.getRight() - valueX - 6;
        drawSectionLabel(g, surface, 7, "EQ");
        drawEq(g, { valueX, surface.getY() + 6, valueWidth, 42 }, accent);

        drawSectionLabel(g, surface, 53, "Input");
        drawValueBox(g, { valueX, surface.getY() + 53, valueWidth, 23 },
                     channel.instrumentName, accent, true);

        drawSectionLabel(g, surface, 80, "Audio FX");
        for (int slot = 0; slot < 3; ++slot)
            drawValueBox(g, { valueX, surface.getY() + 80 + slot * 24, valueWidth, 21 },
                         "+ Add effect", accent, false);

        drawSectionLabel(g, surface, 155, "Sends");
        drawValueBox(g, { valueX, surface.getY() + 155, valueWidth, 22 },
                     "+ Add send", accent, false);

        drawSectionLabel(g, surface, 181, "Output");
        drawValueBox(g, { valueX, surface.getY() + 181, valueWidth, 23 },
                     master ? "Speakers" : "Master", accent, true);

        drawSectionLabel(g, surface, 223, master ? "Balance" : "Pan");
        const auto pan = mixer.channelPan(index);
        const auto panArea = panBounds(strip);
        drawPanKnob(g, panArea, pan, accent);

        drawSectionLabel(g, surface, 278, "VCA");
        drawValueBox(g, { valueX, surface.getY() + 278, valueWidth, 21 }, "—", accent, false);

        const auto volume = mixer.channelVolume(index);
        const auto volumeText = volume <= 0.0001f
            ? juce::String("-inf") : juce::String(20.0f * std::log10(volume), 1);
        drawSectionLabel(g, surface, 303, "dB");
        drawValueBox(g, { valueX, surface.getY() + 303, valueWidth, 22 },
                     volumeText, accent, false);

        const auto fader = faderTrack(strip);
        g.setColour(palette::background.darker(0.08f));
        g.fillRoundedRectangle(fader.toFloat(), 4.0f);
        g.setColour(palette::muted.withAlpha(0.43f));
        for (int tick = 0; tick <= 8; ++tick)
        {
            const auto tickY = fader.getY() + juce::roundToInt(tick / 8.0f * fader.getHeight());
            const auto length = tick % 2 == 0 ? 8 : 5;
            g.drawHorizontalLine(tickY, (float) fader.getX() - length - 4.0f,
                                 (float) fader.getX() - 4.0f);
            g.drawHorizontalLine(tickY, (float) fader.getRight() + 4.0f,
                                 (float) fader.getRight() + length + 4.0f);
        }

        const auto sourcePeak = master ? masterPeak : trackPeak;
        const auto peak = juce::jlimit(0.0f, 1.0f, std::sqrt(sourcePeak * 3.0f));
        const auto meterY = juce::roundToInt(fader.getBottom() - peak * fader.getHeight());
        if (meterY < fader.getBottom())
        {
            g.setColour(peak > 0.88f ? palette::red : palette::mint);
            g.fillRoundedRectangle((float) fader.getX() + 2.0f, (float) meterY,
                                   (float) fader.getWidth() - 4.0f,
                                   (float) fader.getBottom() - meterY, 3.0f);
        }

        const auto capY = juce::roundToInt(fader.getBottom() - volume * fader.getHeight());
        const auto cap = juce::Rectangle<float>((float) fader.getCentreX() - 18.0f,
                                                (float) capY - 12.0f, 36.0f, 24.0f);
        g.setColour(juce::Colours::black.withAlpha(0.38f));
        g.fillRoundedRectangle(cap.translated(0.0f, 2.0f), 3.5f);
        juce::ColourGradient gradient(palette::raised.brighter(0.10f), cap.getX(), cap.getY(),
                                      palette::raised.darker(0.22f), cap.getX(), cap.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(cap, 3.5f);
        g.setColour(palette::line.brighter(0.18f));
        g.drawRoundedRectangle(cap.reduced(0.5f), 3.5f, 1.0f);
        g.setColour(palette::text.withAlpha(0.16f));
        for (float groove : { -5.0f, 0.0f, 5.0f })
            g.drawHorizontalLine(juce::roundToInt(cap.getCentreY() + groove),
                                 cap.getX() + 6.0f, cap.getRight() - 6.0f);
        g.setColour(accent.brighter(0.12f));
        g.fillRoundedRectangle(cap.getX() + 5.0f, cap.getCentreY() - 1.0f,
                               cap.getWidth() - 10.0f, 2.0f, 1.0f);

        drawing::smallButton(g, muteBounds(strip), "M", mixer.channelMuted(index));
        drawing::smallButton(g, soloBounds(strip), master ? "-" : "S",
                             !master && mixer.channelSoloed(index));

        const auto footer = footerBounds(strip);
        g.setColour(accent.withAlpha(master ? 0.72f : 0.62f));
        g.fillRoundedRectangle(footer.toFloat(), 5.0f);
        g.fillRect(footer.getX(), footer.getY(), footer.getWidth(), 5);
        g.setColour(palette::text);
        g.setFont(12.0f);
        g.drawText(channel.name, footer.reduced(8, 0), juce::Justification::centred, true);
    }

    if (!inspectorCollapsed)
        inspectorPresenter.paintInstrument(g, inspector);
}
}
