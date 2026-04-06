#include "ui/common/Drawing.h"
#include "ui/common/MaterialIcons.h"
#include "ui/common/Theme.h"
#include <cmath>

namespace resona::drawing
{
void smallButton(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text, bool active)
{
    g.setColour(active ? palette::violet.withAlpha(0.35f) : palette::raised);
    g.fillRoundedRectangle(area.toFloat(), 6.0f);
    g.setColour(palette::text);
    g.setFont(12.0f);
    g.drawText(text, area, juce::Justification::centred);
}

void magnifier(juce::Graphics& g, juce::Rectangle<int> area, bool plus)
{
    material::draw(g, plus ? material::Icon::zoomIn : material::Icon::zoomOut,
                   area.toFloat().expanded(2.0f), palette::text);
}

void trackIcon(juce::Graphics& g, juce::Rectangle<int> area, int kind)
{
    g.setColour(palette::text);
    if (kind == 0)
    {
        material::draw(g, material::Icon::piano, area.toFloat(), palette::text);
        return;
    }

    if (kind == 1)
    {
        g.drawRoundedRectangle(area.getCentreX() - 7.0f, area.getY() + 3.0f, 14.0f, 24.0f, 7.0f, 3.0f);
        g.drawLine((float) area.getCentreX() - 13.0f, (float) area.getY() + 19.0f,
                   (float) area.getCentreX() - 13.0f, (float) area.getY() + 23.0f, 2.0f);
        juce::Path stand;
        stand.addCentredArc((float) area.getCentreX(), (float) area.getY() + 18.0f,
                            13.0f, 13.0f, 0.0f, 0.0f, juce::MathConstants<float>::pi, true);
        g.strokePath(stand, juce::PathStrokeType(2.5f));
        g.drawVerticalLine(area.getCentreX(), (float) area.getY() + 29.0f, (float) area.getY() + 37.0f);
        g.drawHorizontalLine(area.getY() + 37, (float) area.getCentreX() - 9.0f, (float) area.getCentreX() + 9.0f);
        return;
    }

    g.drawEllipse(area.toFloat().reduced(5.0f, 10.0f), 2.5f);
    g.drawHorizontalLine(area.getY() + 13, (float) area.getX() + 6.0f, (float) area.getRight() - 6.0f);
    g.drawHorizontalLine(area.getBottom() - 13, (float) area.getX() + 6.0f, (float) area.getRight() - 6.0f);
    g.drawLine((float) area.getX() + 13.0f, (float) area.getY() + 15.0f,
               (float) area.getX() + 13.0f, (float) area.getBottom() - 14.0f, 2.0f);
    g.drawLine((float) area.getRight() - 13.0f, (float) area.getY() + 15.0f,
               (float) area.getRight() - 13.0f, (float) area.getBottom() - 14.0f, 2.0f);
}

void piano(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colour(0xff25292d));
    juce::Path body;
    body.startNewSubPath((float) area.getX() + 26.0f, (float) area.getY() + 82.0f);
    body.cubicTo((float) area.getX() + 58.0f, (float) area.getY() + 52.0f,
                 (float) area.getRight() - 118.0f, (float) area.getY() + 26.0f,
                 (float) area.getRight() - 72.0f, (float) area.getY() + 8.0f);
    body.lineTo((float) area.getRight() - 18.0f, (float) area.getY() + 23.0f);
    body.lineTo((float) area.getRight() - 48.0f, (float) area.getY() + 82.0f);
    body.closeSubPath();
    g.fillPath(body);
    g.setColour(juce::Colour(0xff34393e));
    g.drawLine((float) area.getX() + 34.0f, (float) area.getY() + 78.0f,
               (float) area.getRight() - 67.0f, (float) area.getY() + 8.0f, 2.0f);
    g.setColour(juce::Colour(0xff171a1e));
    g.fillRoundedRectangle((float) area.getX() + 16.0f, (float) area.getY() + 80.0f,
                           (float) area.getWidth() - 34.0f, 28.0f, 3.0f);
    g.setColour(palette::text);
    const auto keyY = area.getY() + 89;
    const auto keyboardX = area.getX() + 24;
    const auto keyboardWidth = area.getWidth() - 52;
    for (int i = 0; i < 24; ++i)
        g.fillRect(keyboardX + i * keyboardWidth / 24, keyY, juce::jmax(2, keyboardWidth / 24 - 1), 11);
    g.setColour(juce::Colour(0xff0c0e10));
    for (int i = 1; i < 24; ++i)
        if (i % 7 != 0 && i % 7 != 3) g.fillRect(keyboardX + i * keyboardWidth / 24 - 1, keyY, 3, 7);
    g.drawLine((float) area.getX() + 32.0f, (float) area.getY() + 106.0f,
               (float) area.getX() + 24.0f, (float) area.getBottom(), 3.0f);
    g.drawLine((float) area.getRight() - 48.0f, (float) area.getY() + 106.0f,
               (float) area.getRight() - 38.0f, (float) area.getBottom(), 3.0f);
}

void knob(juce::Graphics& g, juce::Rectangle<int> area, float value)
{
    const auto centre = area.toFloat().getCentre();
    const auto radius = area.getWidth() * 0.45f;
    const auto start = juce::MathConstants<float>::pi * 1.25f;
    const auto end = juce::MathConstants<float>::pi * 2.75f;
    juce::Path track, active;
    track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, start, end, true);
    active.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, start, start + (end - start) * value, true);
    g.setColour(palette::line);
    g.strokePath(track, juce::PathStrokeType(3.0f));
    g.setColour(palette::violet);
    g.strokePath(active, juce::PathStrokeType(3.0f));
    g.setColour(palette::raised);
    g.fillEllipse(area.toFloat().reduced(5.0f));
}

void clip(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& name,
          juce::Colour colour, bool midi, bool selected, std::span<const MidiNote> notes,
          double clipLengthBeats)
{
    g.setColour(colour);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);
    g.setColour(midi ? palette::violetHeader : palette::mint);
    g.fillRoundedRectangle((float) area.getX(), (float) area.getY(), (float) area.getWidth(), 30.0f, 5.0f);
    g.fillRect(area.getX(), area.getY() + 24, area.getWidth(), 7);
    g.setColour(palette::background);
    g.setFont(12.0f);
    g.drawText(name, area.getX() + 10, area.getY(), area.getWidth() - 20, 29, juce::Justification::centredLeft);
    g.setColour((midi ? palette::violetHeader : palette::mint).withAlpha(0.95f));
    if (midi)
    {
        constexpr int low = 40, high = 74;
        const auto top = area.getY() + 38;
        const auto height = juce::jmax(12, area.getHeight() - 46);
        for (const auto& note : notes)
        {
            const auto x = area.getX() + juce::roundToInt(note.beat / clipLengthBeats * area.getWidth());
            const auto width = juce::jmax(3, juce::roundToInt(note.length / clipLengthBeats * area.getWidth()));
            const auto pitch = juce::jlimit(low, high, note.note);
            const auto y = top + juce::roundToInt((double) (high - pitch) / (high - low) * (height - 4));
            g.fillRoundedRectangle((float) x, (float) y, (float) width, 4.0f, 1.5f);
        }
    }
    if (selected)
    {
        g.setColour(palette::text.withAlpha(0.82f));
        g.drawRoundedRectangle(area.toFloat().reduced(1.0f), 5.0f, 1.5f);
    }
}

void playheadMarker(juce::Graphics& g, juce::Rectangle<int> area, int x)
{
    g.setColour(palette::violet);
    g.drawVerticalLine(x, (float) area.getY(), (float) area.getBottom());
    juce::Path marker;
    marker.addTriangle((float) x - 6.0f, (float) area.getY(), (float) x + 6.0f,
                       (float) area.getY(), (float) x, (float) area.getY() + 9.0f);
    g.fillPath(marker);
}
}
