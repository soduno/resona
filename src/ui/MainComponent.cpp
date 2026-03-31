#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "audio/AudioEngine.h"
#include "core/ProjectModel.h"
#include "midi_editor/MidiEditorController.h"
#include "mixer/MixerController.h"
#include "timeline/TimelineController.h"
#include "ui/common/Drawing.h"
#include "ui/timeline/TimelinePresenter.h"
#include "ui/midi_editor/MidiEditorPresenter.h"
#include "ui/chrome/ChromePresenter.h"
#include "ui/inspector/InspectorPresenter.h"
#include "ui/mixer/MixerPresenter.h"
#include "ui/MainComponentFactory.h"
#include "ui/common/Theme.h"
#include <array>
#include <atomic>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

namespace resona
{
class MainComponent final : public juce::AudioAppComponent,
                            private juce::Timer,
                            private juce::ScrollBar::Listener,
                            public juce::MenuBarModel
{
public:
    MainComponent()
    {
        setLookAndFeel(&lookAndFeel);
        setOpaque(true);
        setWantsKeyboardFocus(true);
        menuBar.setVisible(false);

        configureButton(playButton, "▶", [this]
        {
            const auto shouldPlay = !transport.isPlaying();
            if (shouldPlay)
            {
                transport.setPlayheadBeats(timeline.snapToGrid(transport.getPlayheadBeats()));
                statusMessage = "Playing at 120 BPM";
            }
            else
            {
                statusMessage = "Paused on the beat grid";
            }
            transport.setPlaying(shouldPlay);
            playButton.setButtonText(shouldPlay ? "Ⅱ" : "▶");
            repaint();
        });
        playButton.setColour(juce::TextButton::buttonColourId, palette::raised);
        playButton.setComponentID("play");

        configureButton(stopButton, "■", [this]
        {
            transport.setPlaying(false);
            transport.returnToStart();
            playButton.setButtonText("▶");
            repaint();
        });
        stopButton.setComponentID("stop");
        configureButton(recordButton, "●", [this]
        {
            recording = !recording;
            recordButton.setToggleState(recording, juce::dontSendNotification);
            repaint();
        });
        recordButton.setColour(juce::TextButton::textColourOffId, palette::red);
        recordButton.setColour(juce::TextButton::buttonOnColourId, palette::red.withAlpha(0.18f));
        recordButton.setComponentID("record");

        configureButton(timelineButton, "Timeline", [this] { setView(View::timeline); });
        configureButton(mixerButton, "Mixer", [this] { toggleMixerPanel(); });
        timelineButton.setComponentID("timeline");
        mixerButton.setComponentID("mixer");
        playButton.setTooltip("Play / Pause");
        stopButton.setTooltip("Stop");
        recordButton.setTooltip("Record");
        timelineButton.setTooltip("Timeline");
        mixerButton.setTooltip("Mixer");
        configureButton(midiButton, "MIDI", [this] { setView(View::midi); });
        midiButton.setVisible(false);

        configureButton(inspectorToggleButton, "<", [this]
        {
            inspectorCollapsed = !inspectorCollapsed;
            inspectorToggleButton.setButtonText(inspectorCollapsed ? ">" : "<");
            inspectorToggleButton.setTooltip(inspectorCollapsed
                ? "Show inspector" : "Hide inspector");
            resized();
            repaint();
        });
        inspectorToggleButton.setTooltip("Hide inspector");
        inspectorToggleButton.setComponentID("inspectorToggle");

        for (auto* scrollbar : { &timelineHorizontalScroll, &timelineVerticalScroll,
                                 &midiHorizontalScroll, &midiVerticalScroll })
        {
            scrollbar->setAutoHide(false);
            scrollbar->setWantsKeyboardFocus(false);
            scrollbar->addListener(this);
            addAndMakeVisible(*scrollbar);
        }

        configureButton(minimiseButton, "—", [this]
        {
            if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
                window->setMinimised(true);
        });
        configureButton(maximiseButton, "□", [this]
        {
            if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
                window->setFullScreen(!window->isFullScreen());
        });
        configureButton(closeButton, "×", []
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        });
        for (auto* button : { &minimiseButton, &maximiseButton, &closeButton })
            button->setColour(juce::TextButton::buttonColourId, palette::background);
        minimiseButton.setComponentID("minimise");
        maximiseButton.setComponentID("maximise");
        closeButton.setComponentID("close");
        closeButton.setColour(juce::TextButton::buttonOnColourId, palette::red);

        setView(View::timeline);
        setSize(scaledWindowWidth, scaledWindowHeight);
        setAudioChannels(0, 2);
        startTimerHz(30);
    }

    ~MainComponent() override
    {
        shutdownAudio();
        for (auto* scrollbar : { &timelineHorizontalScroll, &timelineVerticalScroll,
                                 &midiHorizontalScroll, &midiVerticalScroll })
            scrollbar->removeListener(this);
        menuBar.setModel(nullptr);
        setLookAndFeel(nullptr);
    }

    void setTimelineViewportForPreview(double startBeat, double visibleBeats)
    {
        timeline.setViewport(startBeat, visibleBeats);
        resized();
        statusMessage = "Timeline zoom: " + juce::String(timeline.getVisibleBeats(), 1)
                        + " beats visible";
        repaint();
    }

    void setTimelineVerticalZoomForPreview(double zoom)
    {
        timeline.setTrackHeightScale(zoom);
        resized();
        statusMessage = "Track height: "
                        + juce::String(juce::roundToInt(timeline.getTrackHeightScale() * 100.0)) + "%";
        repaint();
    }

    void setClickTrackForPreview(bool enabled)
    {
        if (transport.isClickTrackEnabled() != enabled)
            transport.toggleClickTrack();
        statusMessage = enabled ? "Click track on - press C to toggle" : "Click track off";
        repaint();
    }

    void showMidiEditorForPreview()
    {
        setView(View::midi);
        midiEditor.setViewport(4.0, 12.0, 74.0, 24.0);
        resized();
        transport.setPlayheadBeats(project.getMidiClipStart() + 8.0);
        midiEditor.selectAll(notes.size());
        statusMessage = "All MIDI notes selected";
        repaint();
    }

    void showNarrowSidebarForPreview()
    {
        leftSidebarWidth = minimumTrackListWidth;
        resized();
        repaint();
    }

    void showMixerForPreview()
    {
        if (!mixerPanelOpen)
            toggleMixerPanel();
        mixer.setChannelPan(0, -0.35f);
        mixer.toggleChannelSolo(0);
    }

    void prepareToPlay(int, double newSampleRate) override
    {
        audioEngine.prepare(newSampleRate);
    }

    void releaseResources() override {}

    void scrollBarMoved(juce::ScrollBar* scrollbar, double start) override
    {
        if (scrollbar == &timelineHorizontalScroll)
            timeline.setViewport(start, timeline.getVisibleBeats());
        else if (scrollbar == &midiHorizontalScroll)
            midiEditor.setViewport(start, midiEditor.getVisibleBeats(),
                                   midiEditor.getTopNote(), midiEditor.getVisibleNoteRows());
        else if (scrollbar == &midiVerticalScroll)
            midiEditor.setViewport(midiEditor.getStartBeat(), midiEditor.getVisibleBeats(),
                                   127.0 - start, midiEditor.getVisibleNoteRows());
        repaint();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override
    {
        audioEngine.render(info);
    }

    void resized() override
    {
        syncScrollbars();
        auto transportArea = juce::Rectangle<int>((getWidth() - 212) / 2,
                                                   titleBarHeight + menuBarHeight + 12, 212, 50);
        stopButton.setBounds(transportArea.removeFromLeft(58));
        transportArea.removeFromLeft(8);
        playButton.setBounds(transportArea.removeFromLeft(58));
        transportArea.removeFromLeft(8);
        recordButton.setBounds(transportArea.removeFromLeft(58));

        auto tabs = juce::Rectangle<int>(20, titleBarHeight + menuBarHeight + 15, 292, 44);
        timelineButton.setBounds(tabs.removeFromLeft(146));
        tabs.removeFromLeft(4);
        mixerButton.setBounds(tabs.removeFromLeft(140));

        const auto windowButtonWidth = 46;
        closeButton.setBounds(getWidth() - windowButtonWidth, 0, windowButtonWidth, titleBarHeight);
        maximiseButton.setBounds(getWidth() - windowButtonWidth * 2, 0, windowButtonWidth, titleBarHeight);
        minimiseButton.setBounds(getWidth() - windowButtonWidth * 3, 0, windowButtonWidth, titleBarHeight);

        const auto contentBottom = getHeight() - statusBarHeight;
        const auto timelineContentBottom = timelineEditorArea().getBottom();
        const auto inspectorLeft = getWidth() - currentInspectorWidth();
        inspectorToggleButton.setBounds(inspectorLeft + 4, contentTop + 8,
                                        juce::jmin(36, currentInspectorWidth() - 8), 28);

        const auto timelineRight = inspectorLeft - (timelineVerticalNeeded ? scrollbarThickness : 0);
        timelineHorizontalScroll.setBounds(
            timelineHorizontalNeeded
                ? juce::Rectangle<int>(leftSidebarWidth, timelineContentBottom - scrollbarThickness,
                                       juce::jmax(0, timelineRight - leftSidebarWidth), scrollbarThickness)
                : juce::Rectangle<int>());
        timelineVerticalScroll.setBounds(
            timelineVerticalNeeded
                ? juce::Rectangle<int>(timelineRight, contentTop, scrollbarThickness,
                                       juce::jmax(0, timelineContentBottom - contentTop
                                                      - (timelineHorizontalNeeded ? scrollbarThickness : 0)))
                : juce::Rectangle<int>());

        const auto midiRight = inspectorLeft - (midiVerticalNeeded ? scrollbarThickness : 0);
        midiHorizontalScroll.setBounds(
            midiHorizontalNeeded
                ? juce::Rectangle<int>(midiKeyboardWidth, contentBottom - scrollbarThickness,
                                       juce::jmax(0, midiRight - midiKeyboardWidth), scrollbarThickness)
                : juce::Rectangle<int>());
        midiVerticalScroll.setBounds(
            midiVerticalNeeded
                ? juce::Rectangle<int>(midiRight, contentTop, scrollbarThickness,
                                       juce::jmax(0, contentBottom - contentTop
                                                      - (midiHorizontalNeeded ? scrollbarThickness : 0)))
                : juce::Rectangle<int>());
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(palette::background);
        const auto chrome = chromeState();
        chromePresenter.paintToolbar(g, chrome);

        const auto content = editorContentArea();
        switch (currentView)
        {
            case View::timeline:
                paintTimeline(g, timelineEditorArea());
                if (mixerPanelOpen)
                {
                    const auto console = mixerPanelArea();
                    g.setColour(palette::line.brighter(0.12f));
                    g.fillRect(console.getX(), console.getY(), console.getWidth(), 4);
                    g.setColour(palette::muted.withAlpha(0.48f));
                    g.fillRoundedRectangle((float) console.getCentreX() - 22.0f,
                                           (float) console.getY() + 1.0f,
                                           44.0f, 2.0f, 1.0f);
                    paintMixer(g, console.withTrimmedTop(4));
                }
                break;
            case View::midi: paintMidiEditor(g, content); break;
        }
        chromePresenter.paintStatusBar(g, chrome);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (event.y < titleBarHeight)
        {
            if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
            {
                draggingWindow = true;
                windowDragger.startDraggingComponent(window, event.getEventRelativeTo(window));
            }
            return;
        }

        if (metronomeButtonBounds().contains(event.getPosition()))
        {
            toggleClickTrack();
            return;
        }

        if (currentView == View::timeline
            && leftSidebarDividerBounds().contains(event.getPosition()))
        {
            resizingLeftSidebar = true;
            return;
        }

        if (currentView == View::timeline && mixerPanelOpen
            && mixerPanelDividerBounds().contains(event.getPosition()))
        {
            resizingMixerPanel = true;
            return;
        }

        if (currentView == View::timeline && mixerPanelOpen)
        {
            const auto handle = mixerStripResizeHandleAt(event.getPosition());
            if (handle >= 0)
            {
                resizingMixerStrips = true;
                mixerStripResizeStartX = event.x;
                mixerStripResizeStartWidth = mixerPresenter.getChannelStripWidth();
                return;
            }
        }

        if (currentView == View::timeline && timelineMuteButtonBounds().contains(event.getPosition()))
        {
            const auto active = mixer.toggleChannelMute(0);
            statusMessage = active ? "MIDI Piano muted" : "MIDI Piano unmuted";
            repaint();
            return;
        }

        if (currentView == View::timeline && timelineSoloButtonBounds().contains(event.getPosition()))
        {
            const auto active = mixer.toggleChannelSolo(0);
            statusMessage = active ? "MIDI Piano soloed" : "MIDI Piano solo off";
            repaint();
            return;
        }

        if (currentView == View::timeline && timelinePanBounds().contains(event.getPosition()))
        {
            draggingTrackPan = true;
            setTrackPanFromX(event.x);
            return;
        }

        if (currentView == View::timeline
            && trackVolumeSliderBounds().contains(event.getPosition()))
        {
            draggingTrackVolume = true;
            setTrackVolumeFromX(event.x);
            return;
        }

        if (currentView == View::timeline && mixerPanelOpen)
        {
            const auto console = mixerPanelArea().withTrimmedTop(4);
            for (size_t channel = 0; channel < mixer.channelCount(); ++channel)
            {
                const auto index = static_cast<int>(channel);
                const auto& channelInfo = mixer.channel(channel);
                if (mixerPresenter.muteButtonArea(console, 0, index).contains(event.getPosition()))
                {
                    const auto active = mixer.toggleChannelMute(channel);
                    statusMessage = channelInfo.name + (active ? " muted" : " unmuted");
                    repaint();
                    return;
                }
                if (mixer.channelSupportsSolo(channel)
                    && mixerPresenter.soloButtonArea(console, 0, index).contains(event.getPosition()))
                {
                    const auto active = mixer.toggleChannelSolo(channel);
                    statusMessage = channelInfo.name + (active ? " soloed" : " solo off");
                    repaint();
                    return;
                }
                if (mixerPresenter.panHitArea(console, 0, index).contains(event.getPosition()))
                {
                    draggedMixerChannel = index;
                    draggingMixerPan = true;
                    setMixerPanFromX(event.x, index);
                    return;
                }
                if (mixerPresenter.faderHitArea(console, 0, index).contains(event.getPosition()))
                {
                    draggedMixerChannel = index;
                    draggingMixerFader = true;
                    setMixerVolumeFromY(event.y, index);
                    return;
                }
            }
        }

        if (currentView == View::midi && midiRulerArea().contains(event.getPosition()))
        {
            draggingMidiPlayhead = true;
            setMidiPlayheadFromX(event.x);
            return;
        }

        if (currentView == View::midi && midiEditorSeekArea().contains(event.getPosition()))
        {
            int hitNote = -1;
            const auto grid = midiGridArea();
            if (grid.contains(event.getPosition()))
                for (int i = static_cast<int>(notes.size()) - 1; i >= 0; --i)
                    if (midiNoteBounds(i, grid).contains(event.getPosition()))
                    {
                        hitNote = i;
                        break;
                    }

            if (hitNote >= 0)
            {
                const auto index = static_cast<size_t>(hitNote);
                if (event.mods.isCommandDown())
                    midiEditor.toggleSelection(index);
                else
                    midiEditor.selectOnly(index);
                statusMessage = juce::String(midiEditor.selectionSize()) + " MIDI note(s) selected";
            }
            else
            {
                if (!event.mods.isCommandDown())
                    midiEditor.clearSelection();
                beginMarquee(MarqueeTarget::midi, event.getPosition());
            }
            repaint();
            return;
        }

        if (event.y < contentTop || currentView != View::timeline) return;
        const auto area = timelineArea();
        if (!area.contains(event.getPosition())) return;

        if (timelineRulerArea().contains(event.getPosition()))
        {
            draggingTimelinePlayhead = true;
            setTimelinePlayheadFromX(event.x);
            return;
        }

        for (int i = static_cast<int>(clips.size()) - 1; i >= 0; --i)
        {
            if (!clipBounds(i).contains(event.getPosition())) continue;
            draggedClipIndex = i;
            timeline.selectOnly(static_cast<size_t>(i));
            clipDragOffsetBeats = beatAtX(event.x) - clips[static_cast<size_t>(i)].startBeat;
            clipWasMoved = false;
            statusMessage = "Drag clip - snaps to 1 beat";
            repaint();
            return;
        }

        timeline.clearSelection();
        beginMarquee(MarqueeTarget::timeline, event.getPosition());
        repaint();
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (draggingWindow)
        {
            if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
                windowDragger.dragComponent(window, event.getEventRelativeTo(window), nullptr);
            return;
        }

        if (resizingLeftSidebar)
        {
            leftSidebarWidth = juce::jlimit(minimumTrackListWidth, maximumTrackListWidth, event.x);
            resized();
            repaint();
            return;
        }

        if (resizingMixerPanel)
        {
            mixerPanelHeight = editorContentArea().getBottom() - event.y;
            resized();
            repaint();
            return;
        }

        if (resizingMixerStrips)
        {
            mixerPresenter.setChannelStripWidth(
                mixerStripResizeStartWidth + event.x - mixerStripResizeStartX);
            statusMessage = "Channel strip width: "
                            + juce::String(mixerPresenter.getChannelStripWidth()) + " px";
            repaint();
            return;
        }

        if (draggingTrackVolume)
        {
            setTrackVolumeFromX(event.x);
            return;
        }

        if (draggingTrackPan)
        {
            setTrackPanFromX(event.x);
            return;
        }
        if (draggingMixerPan)
        {
            setMixerPanFromX(event.x, draggedMixerChannel);
            return;
        }

        if (draggingMixerFader)
        {
            setMixerVolumeFromY(event.y, draggedMixerChannel);
            return;
        }

        if (draggingTimelinePlayhead)
        {
            setTimelinePlayheadFromX(event.x);
            return;
        }

        if (draggingMidiPlayhead)
        {
            setMidiPlayheadFromX(event.x);
            return;
        }

        if (marqueeTarget != MarqueeTarget::none)
        {
            updateMarquee(event.getPosition());
            return;
        }

        if (draggedClipIndex < 0 || currentView != View::timeline) return;
        auto& clip = clips[static_cast<size_t>(draggedClipIndex)];
        const auto requestedBeat = beatAtX(event.x) - clipDragOffsetBeats;
        const auto newStart = juce::jlimit(0.0, projectLengthBeats - clip.lengthBeats,
                                          timeline.snapToGrid(requestedBeat));
        if (std::abs(newStart - clip.startBeat) < 0.001) return;

        clip.startBeat = newStart;
        clipWasMoved = true;
        saved = false;
        syncMidiClipPosition(draggedClipIndex);
        statusMessage = "Clip at beat " + juce::String(juce::roundToInt(newStart) + 1);
        repaint();
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        draggingWindow = false;
        resizingLeftSidebar = false;
        resizingMixerPanel = false;
        resizingMixerStrips = false;
        draggingTrackVolume = false;
        draggingTrackPan = false;
        draggingMixerFader = false;
        draggingMixerPan = false;
        draggedMixerChannel = -1;
        draggingTimelinePlayhead = false;
        draggingMidiPlayhead = false;
        if (marqueeTarget != MarqueeTarget::none)
        {
            marqueeTarget = MarqueeTarget::none;
            marqueeActive = false;
            marqueeBounds = {};
        }
        if (draggedClipIndex >= 0 && clipWasMoved)
            statusMessage = "Clip moved on the 120 BPM grid";
        draggedClipIndex = -1;
        clipWasMoved = false;
        repaint();
    }

    void mouseMove(const juce::MouseEvent& event) override
    {
        if (currentView == View::timeline
            && leftSidebarDividerBounds().contains(event.getPosition()))
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        else if (currentView == View::timeline && mixerPanelOpen
                 && mixerPanelDividerBounds().contains(event.getPosition()))
            setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        else if (currentView == View::timeline && mixerPanelOpen
                 && mixerStripResizeHandleAt(event.getPosition()) >= 0)
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        else if (currentView == View::timeline
                 && trackVolumeSliderBounds().contains(event.getPosition()))
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        else if (currentView == View::timeline
                 && (timelineMuteButtonBounds().contains(event.getPosition())
                     || timelineSoloButtonBounds().contains(event.getPosition())
                     || timelinePanBounds().contains(event.getPosition())))
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        else if (currentView == View::timeline && mixerPanelOpen
                 && mixerPresenter.faderHitArea(mixerPanelArea().withTrimmedTop(4), 0)
                        .contains(event.getPosition()))
            setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        else if (currentView == View::timeline && mixerPanelOpen
                 && (mixerPresenter.panHitArea(mixerPanelArea().withTrimmedTop(4), 0)
                         .contains(event.getPosition())
                     || mixerPresenter.muteButtonArea(mixerPanelArea().withTrimmedTop(4), 0)
                         .contains(event.getPosition())
                     || mixerPresenter.soloButtonArea(mixerPanelArea().withTrimmedTop(4), 0)
                         .contains(event.getPosition())))
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        else
            setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override
    {
        const auto delta = std::abs(wheel.deltaY) > 0.0001f ? wheel.deltaY : wheel.deltaX;
        if (std::abs(delta) < 0.0001f) return;
        const auto direction = delta > 0.0f ? 1.0f : -1.0f;
        const auto fine = event.mods.isShiftDown();

        if (masterVolumeSliderBounds().contains(event.getPosition()))
        {
            const auto master = mixer.channelCount() - 1;
            const auto value = juce::jlimit(0.0f, 1.0f,
                mixer.channelVolume(master) + direction * (fine ? 0.01f : 0.04f));
            mixer.setChannelVolume(master, value);
            statusMessage = "Master volume: "
                            + juce::String(juce::roundToInt(value * 100.0f)) + "%";
            repaint();
            return;
        }

        const auto inspector = instrumentInspectorArea();
        if (!inspector.isEmpty())
        {
            static constexpr const char* names[] { "Tone", "Space", "Mix" };
            for (int index = 0; index < 3; ++index)
            {
                if (!inspectorPresenter.instrumentKnobArea(inspector, index)
                         .expanded(5).contains(event.getPosition()))
                    continue;
                const auto value = juce::jlimit(0.0f, 1.0f,
                    inspectorPresenter.instrumentValue(index)
                        + direction * (fine ? 0.01f : 0.04f));
                inspectorPresenter.setInstrumentValue(index, value);
                statusMessage = juce::String(names[index]) + ": "
                                + juce::String(juce::roundToInt(value * 100.0f)) + "%";
                repaint();
                return;
            }
        }

        if (currentView == View::timeline && mixerPanelOpen)
        {
            const auto console = mixerPanelArea().withTrimmedTop(4);
            for (size_t channel = 0; channel < mixer.channelCount(); ++channel)
            {
                const auto index = static_cast<int>(channel);
                if (mixerPresenter.panHitArea(console, 0, index)
                        .contains(event.getPosition()))
                {
                    const auto value = juce::jlimit(-1.0f, 1.0f,
                        mixer.channelPan(channel) + direction * (fine ? 0.01f : 0.05f));
                    mixer.setChannelPan(channel, value);
                    statusMessage = mixer.channel(channel).name
                                    + (mixer.isMasterChannel(channel) ? " balance: " : " pan: ")
                                    + juce::String(juce::roundToInt(value * 100.0f));
                    repaint();
                    return;
                }
                if (mixerPresenter.faderHitArea(console, 0, index)
                        .contains(event.getPosition()))
                {
                    const auto value = juce::jlimit(0.0f, 1.0f,
                        mixer.channelVolume(channel) + direction * (fine ? 0.01f : 0.04f));
                    mixer.setChannelVolume(channel, value);
                    statusMessage = mixer.channel(channel).name + " volume: "
                                    + juce::String(juce::roundToInt(value * 100.0f)) + "%";
                    repaint();
                    return;
                }
            }
        }

        if (currentView == View::timeline
            && timelinePanBounds().contains(event.getPosition()))
        {
            const auto value = juce::jlimit(-1.0f, 1.0f,
                mixer.channelPan(0) + direction * (fine ? 0.01f : 0.05f));
            mixer.setChannelPan(0, value);
            statusMessage = "MIDI Piano pan: "
                            + juce::String(juce::roundToInt(value * 100.0f));
            repaint();
            return;
        }

        if (currentView == View::timeline
            && trackVolumeSliderBounds().contains(event.getPosition()))
        {
            const auto value = juce::jlimit(0.0f, 1.0f,
                mixer.channelVolume(0) + direction * (fine ? 0.01f : 0.04f));
            mixer.setChannelVolume(0, value);
            statusMessage = "MIDI Piano volume: "
                            + juce::String(juce::roundToInt(value * 100.0f)) + "%";
            repaint();
            return;
        }

        if (statusZoomSliderBounds().contains(event.getPosition()))
        {
            if (currentView == View::timeline)
                timeline.zoomTime(0.5, delta);
            else
                midiEditor.zoomTime(0.5, delta);
            resized();
            statusMessage = "Zoom: "
                            + juce::String(currentView == View::timeline
                                               ? timeline.getVisibleBeats()
                                               : midiEditor.getVisibleBeats(), 1)
                            + " beats visible";
            repaint();
            return;
        }

        if (currentView == View::midi && midiGridArea().contains(event.getPosition()))
        {
            const auto grid = midiGridArea();
            if (event.mods.isShiftDown())
            {
                const auto pointerRatio = juce::jlimit(0.0, 1.0,
                    static_cast<double>(event.y - grid.getY())
                    / static_cast<double>(juce::jmax(1, grid.getHeight())));
                midiEditor.zoomPitch(pointerRatio, delta);
                resized();
                statusMessage = "MIDI note height: "
                                + juce::String(juce::roundToInt(
                                    36.0 / midiEditor.getVisibleNoteRows() * 100.0)) + "%";
            }
            else
            {
                const auto pointerRatio = juce::jlimit(0.0, 1.0,
                    static_cast<double>(event.x - grid.getX())
                    / static_cast<double>(juce::jmax(1, grid.getWidth())));
                midiEditor.zoomTime(pointerRatio, delta);
                resized();
                statusMessage = "MIDI zoom: " + juce::String(midiEditor.getVisibleBeats(), 1)
                                + " beats visible";
            }
            repaint();
            return;
        }

        if (currentView != View::timeline || !timelineArea().contains(event.getPosition()))
            return;

        if (event.mods.isShiftDown())
        {
            timeline.zoomTrackHeight(delta);
            resized();
            statusMessage = "Track height: "
                            + juce::String(juce::roundToInt(
                                timeline.getTrackHeightScale() * 100.0)) + "%";
            repaint();
            return;
        }

        const auto area = arrangementArea();
        const auto pointerRatio = juce::jlimit(0.0, 1.0,
            static_cast<double>(event.x - area.getX()) / static_cast<double>(juce::jmax(1, area.getWidth())));
        timeline.zoomTime(pointerRatio, delta);
        resized();
        statusMessage = "Timeline zoom: " + juce::String(timeline.getVisibleBeats(), 1)
                        + " beats visible";
        repaint();
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        if (currentView == View::timeline && timelinePanBounds().contains(event.getPosition()))
        {
            mixer.setChannelPan(0, 0.0f);
            draggingTrackPan = false;
            statusMessage = "MIDI Piano pan: center";
            repaint();
            return;
        }

        if (currentView == View::timeline && mixerPanelOpen)
        {
            const auto console = mixerPanelArea().withTrimmedTop(4);
            for (size_t channel = 0; channel < mixer.channelCount(); ++channel)
            {
                const auto index = static_cast<int>(channel);
                if (!mixerPresenter.panHitArea(console, 0, index).contains(event.getPosition()))
                    continue;

                mixer.setChannelPan(channel, 0.0f);
                draggingMixerPan = false;
                draggedMixerChannel = -1;
                statusMessage = mixer.channel(channel).name
                                + (mixer.isMasterChannel(channel)
                                       ? " balance: center" : " pan: center");
                repaint();
                return;
            }
        }

        if (currentView == View::timeline && arrangementArea().contains(event.getPosition()))
            setView(View::midi);
    }

    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key.getKeyCode() == juce::KeyPress::numberPad0
            || key.getTextCharacter() == '0')
        {
            transport.returnToStart();
            timeline.returnViewToStart();
            resized();
            statusMessage = "Returned to start";
            repaint();
            return true;
        }
        if (key.getKeyCode() == juce::KeyPress::returnKey)
        {
            playButton.triggerClick();
            return true;
        }
        if (currentView == View::midi && key.getModifiers().isCommandDown()
            && juce::CharacterFunctions::toLowerCase(key.getTextCharacter()) == 'a')
        {
            midiEditor.selectAll(notes.size());
            statusMessage = "All MIDI notes selected";
            repaint();
            return true;
        }
        if (key.getModifiers().isCommandDown() && key.getTextCharacter() == 's')
        {
            markSaved();
            return true;
        }
        if (key == juce::KeyPress::spaceKey)
        {
            playButton.triggerClick();
            return true;
        }
        if (juce::CharacterFunctions::toLowerCase(key.getTextCharacter()) == 'c')
        {
            toggleClickTrack();
            return true;
        }
        if (juce::CharacterFunctions::toLowerCase(key.getTextCharacter()) == 'm')
        {
            toggleMixerPanel();
            return true;
        }
        if (key.getTextCharacter() == '1') { setView(View::timeline); return true; }
        if (key.getTextCharacter() == '2') { toggleMixerPanel(); return true; }
        if (key.getTextCharacter() == '3') { setView(View::midi); return true; }
        return false;
    }

    juce::StringArray getMenuBarNames() override
    {
        return { "File", "Edit", "View", "Track", "Help" };
    }

    juce::PopupMenu getMenuForIndex(int index, const juce::String&) override
    {
        juce::PopupMenu menu;
        if (index == 0)
        {
            menu.addItem(101, "New Project", true, false);
            menu.addItem(102, "Open...", true, false);
            menu.addSeparator();
            menu.addItem(103, "Save", true, false);
            menu.addItem(104, "Save As...", true, false);
            menu.addSeparator();
            menu.addItem(105, "Export Audio...", true, false);
        }
        else if (index == 1)
        {
            menu.addItem(201, "Undo", false, false);
            menu.addItem(202, "Redo", false, false);
            menu.addSeparator();
            menu.addItem(203, "Settings...", true, false);
        }
        else if (index == 2)
        {
            menu.addItem(301, "Timeline", true, currentView == View::timeline);
            menu.addItem(302, "Mixer", true, mixerPanelOpen);
            menu.addItem(303, "MIDI-editor", true, currentView == View::midi);
        }
        else if (index == 3)
        {
            menu.addItem(401, "Add Audio Track", true, false);
            menu.addItem(402, "Add Instrument Track", true, false);
            menu.addItem(403, "Scan VST3 Plug-ins...", true, false);
        }
        else
        {
            menu.addItem(501, "Keyboard Shortcuts", true, false);
            menu.addItem(502, "About Resona", true, false);
        }
        return menu;
    }

    void menuItemSelected(int itemId, int) override
    {
        if (itemId == 103) markSaved();
        else if (itemId == 301) setView(View::timeline);
        else if (itemId == 302) toggleMixerPanel();
        else if (itemId == 303) setView(View::midi);
        else if (itemId == 401 || itemId == 402)
        {
            saved = false;
            statusMessage = itemId == 401 ? "Audio track added" : "Instrument track added";
            repaint();
        }
        else if (itemId == 403)
        {
            statusMessage = "VST3 scanning is planned for the next engine milestone";
            repaint();
        }
        else if (itemId != 0)
        {
            statusMessage = "Command ready for the next milestone";
            repaint();
        }
    }

private:
    enum class View { timeline, midi };
    enum class MarqueeTarget { none, timeline, midi };

    static constexpr int titleBarHeight = 38;
    static constexpr int menuBarHeight = 0;
    static constexpr int topBarHeight = 76;
    static constexpr int contentTop = titleBarHeight + menuBarHeight + topBarHeight;
    static constexpr int statusBarHeight = 58;
    static constexpr int defaultTrackListWidth = 246;
    static constexpr int minimumTrackListWidth = 132;
    static constexpr int maximumTrackListWidth = 340;
    static constexpr int collapsedInspectorWidth = 48;
    static constexpr int timelineInspectorWidth = 354;
    static constexpr int midiInspectorWidth = 250;
    static constexpr int midiKeyboardWidth = 66;
    static constexpr int midiHeaderHeight = 48;
    static constexpr int timelineRulerHeight = 60;
    static constexpr int midiRulerHeight = timelineRulerHeight;
    static constexpr int scrollbarThickness = 13;
    static constexpr double tempo = 120.0;
    static constexpr double projectLengthBeats = 128.0;
    static constexpr double initialVisibleBeats = 32.0;
    static constexpr double minimumVisibleBeats = 4.0;
    static constexpr double maximumVisibleBeats = 128.0;
    static constexpr double minimumVerticalZoom = 0.85;
    static constexpr double maximumVerticalZoom = 2.0;
    static constexpr double minimumMidiVisibleNotes = 12.0;
    static constexpr double maximumMidiVisibleNotes = 72.0;
    static constexpr double gridSnapBeats = 1.0;
    static constexpr double firstMidiClipLength = 28.0;

    ResonaLookAndFeel lookAndFeel;
    ChromePresenter chromePresenter;
    InspectorPresenter inspectorPresenter;
    TimelinePresenter timelinePresenter { inspectorPresenter };
    MidiEditorPresenter midiPresenter { inspectorPresenter };
    MixerPresenter mixerPresenter { inspectorPresenter };
    juce::MenuBarComponent menuBar { this };
    juce::TooltipWindow tooltips { this, 600 };
    juce::TextButton playButton, stopButton, recordButton;
    juce::TextButton timelineButton, mixerButton, midiButton;
    juce::TextButton minimiseButton, maximiseButton, closeButton;
    juce::TextButton inspectorToggleButton;
    juce::ScrollBar timelineHorizontalScroll { false };
    juce::ScrollBar timelineVerticalScroll { true };
    juce::ScrollBar midiHorizontalScroll { false };
    juce::ScrollBar midiVerticalScroll { true };
    View currentView = View::timeline;
    bool recording = false;
    bool saved = true;
    bool draggingWindow = false;
    bool resizingLeftSidebar = false;
    bool resizingMixerPanel = false;
    bool resizingMixerStrips = false;
    bool draggingTrackVolume = false;
    bool draggingTrackPan = false;
    bool draggingMixerFader = false;
    bool draggingMixerPan = false;
    int draggedMixerChannel = -1;
    int mixerStripResizeStartX = 0;
    int mixerStripResizeStartWidth = 178;
    bool draggingTimelinePlayhead = false;
    bool draggingMidiPlayhead = false;
    bool mixerPanelOpen = false;
    bool inspectorCollapsed = false;
    bool timelineHorizontalNeeded = false;
    bool timelineVerticalNeeded = false;
    bool midiHorizontalNeeded = false;
    bool midiVerticalNeeded = false;
    bool clipWasMoved = false;
    int draggedClipIndex = -1;
    MarqueeTarget marqueeTarget = MarqueeTarget::none;
    bool marqueeActive = false;
    juce::Point<int> marqueeStart;
    juce::Rectangle<int> marqueeBounds;
    double clipDragOffsetBeats = 0.0;
    int leftSidebarWidth = defaultTrackListWidth;
    int mixerPanelHeight = 0;
    juce::String statusMessage { "Ready" };
    juce::ComponentDragger windowDragger;
    ProjectModel project;
    MixerController mixer { project };
    TransportController transport;
    TimelineController timeline;
    MidiEditorController midiEditor;
    AudioEngine audioEngine { project, transport };
    std::vector<MidiNote>& notes { project.notes };
    std::array<TimelineClip, 1>& clips { project.clips };

    void configureButton(juce::TextButton& button, const juce::String& text,
                         std::function<void()> onClick)
    {
        button.setButtonText(text);
        button.onClick = std::move(onClick);
        button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        button.setTooltip(text);
        button.setWantsKeyboardFocus(false);
        addAndMakeVisible(button);
    }

    void toggleClickTrack()
    {
        const auto enabled = transport.toggleClickTrack();
        statusMessage = enabled ? "Click track on - 120 BPM" : "Click track off";
        repaint();
    }

    void markSaved()
    {
        saved = true;
        statusMessage = "Project saved";
        repaint();
    }

    void setView(View view)
    {
        currentView = view;
        timelineButton.setToggleState(view == View::timeline, juce::dontSendNotification);
        mixerButton.setToggleState(mixerPanelOpen, juce::dontSendNotification);
        midiButton.setToggleState(view == View::midi, juce::dontSendNotification);
        if (view == View::midi && mixerPanelOpen)
        {
            mixerPanelOpen = false;
            mixerButton.setToggleState(false, juce::dontSendNotification);
        }
        inspectorToggleButton.setVisible(true);
        resized();
        repaint();
    }

    void toggleMixerPanel()
    {
        if (currentView == View::midi)
            currentView = View::timeline;
        mixerPanelOpen = !mixerPanelOpen;
        timelineButton.setToggleState(true, juce::dontSendNotification);
        midiButton.setToggleState(false, juce::dontSendNotification);
        mixerButton.setToggleState(mixerPanelOpen, juce::dontSendNotification);
        mixerButton.setTooltip(mixerPanelOpen ? "Hide mixer (M)" : "Show mixer (M)");
        statusMessage = mixerPanelOpen ? "Mixer opened" : "Mixer closed";
        resized();
        repaint();
    }

    void timerCallback() override
    {
        if (transport.isPlaying() || audioEngine.getTrackPeak() > 0.001f) repaint();
    }

    ChromeState chromeState() const
    {
        return { getWidth(), getHeight(), titleBarHeight, contentTop, statusBarHeight,
                 transport.isClickTrackEnabled(), project.getMasterVolume(), statusMessage,
                 timeline.getVisibleBeats(), minimumVisibleBeats, maximumVisibleBeats };
    }

    juce::Rectangle<int> masterVolumeSliderBounds() const
    {
        const auto toolY = titleBarHeight + menuBarHeight + 14;
        return { getWidth() - 329, toolY + 8, 176, 32 };
    }

    juce::Rectangle<int> statusZoomSliderBounds() const
    {
        return { getWidth() - 190, getHeight() - statusBarHeight,
                 180, statusBarHeight };
    }

    juce::Rectangle<int> instrumentInspectorArea() const
    {
        if (inspectorCollapsed || currentView != View::timeline) return {};
        const auto editor = timelineEditorArea();
        return { getWidth() - currentInspectorWidth(), editor.getY(),
                 currentInspectorWidth(), editor.getHeight() };
    }

    juce::Rectangle<int> timelineArea() const
    {
        const auto editor = timelineEditorArea();
        return { leftSidebarWidth, editor.getY(),
                 getWidth() - leftSidebarWidth - currentInspectorWidth()
                     - (timelineVerticalNeeded ? scrollbarThickness : 0),
                 editor.getHeight()
                     - (timelineHorizontalNeeded ? scrollbarThickness : 0) };
    }

    juce::Rectangle<int> editorContentArea() const
    {
        return { 0, contentTop, getWidth(), getHeight() - contentTop - statusBarHeight };
    }

    juce::Rectangle<int> timelineEditorArea() const
    {
        auto area = editorContentArea();
        if (mixerPanelOpen)
            area.removeFromBottom(effectiveMixerPanelHeight());
        return area;
    }

    juce::Rectangle<int> mixerPanelArea() const
    {
        auto area = editorContentArea();
        return mixerPanelOpen ? area.removeFromBottom(effectiveMixerPanelHeight())
                              : juce::Rectangle<int>();
    }

    int effectiveMixerPanelHeight() const
    {
        const auto totalHeight = editorContentArea().getHeight();
        if (totalHeight <= 0) return 0;
        const auto minimumTimelineHeight = juce::jmin(180, totalHeight / 2);
        const auto minimumMixerHeight = juce::jmin(390, totalHeight - minimumTimelineHeight);
        const auto maximumMixerHeight = juce::jmax(minimumMixerHeight,
                                                   totalHeight - minimumTimelineHeight);
        const auto requestedHeight = mixerPanelHeight > 0 ? mixerPanelHeight
                                                          : totalHeight / 2;
        return juce::jlimit(minimumMixerHeight, maximumMixerHeight, requestedHeight);
    }

    juce::Rectangle<int> mixerPanelDividerBounds() const
    {
        const auto panel = mixerPanelArea();
        return panel.isEmpty()
            ? juce::Rectangle<int>()
            : juce::Rectangle<int>(panel.getX(), panel.getY() - 3,
                                   panel.getWidth(), 10);
    }

    int mixerStripResizeHandleAt(juce::Point<int> position) const
    {
        if (!mixerPanelOpen) return -1;
        const auto console = mixerPanelArea().withTrimmedTop(4);
        for (size_t channel = 0; channel < mixer.channelCount(); ++channel)
            if (mixerPresenter.stripResizeHandleArea(
                    console, 0, static_cast<int>(channel)).contains(position))
                return static_cast<int>(channel);
        return -1;
    }

    juce::Rectangle<int> leftSidebarDividerBounds() const
    {
        return { leftSidebarWidth - 4, contentTop, 8, timelineEditorArea().getHeight() };
    }

    juce::Rectangle<int> trackVolumeSliderBounds() const
    {
        const auto compact = leftSidebarWidth < 205;
        const auto controlsX = compact ? 64 : 108;
        const auto controlsRight = leftSidebarWidth - (compact ? 8 : 24);
        const auto sliderX = controlsX + 3;
        const auto sliderY = contentTop + timelineRulerHeight + (compact ? 116 : 126);
        return { sliderX, sliderY - 8, juce::jmax(24, controlsRight - sliderX), 23 };
    }

    juce::Rectangle<int> timelineMuteButtonBounds() const
    {
        const auto compact = leftSidebarWidth < 205;
        const auto controlsX = compact ? 64 : 108;
        const auto width = compact ? 30 : 39;
        return { controlsX, contentTop + timelineRulerHeight + 62, width, 36 };
    }

    juce::Rectangle<int> timelineSoloButtonBounds() const
    {
        const auto compact = leftSidebarWidth < 205;
        const auto mute = timelineMuteButtonBounds();
        return mute.translated(mute.getWidth() + (compact ? 5 : 11), 0);
    }

    juce::Rectangle<int> timelinePanBounds() const
    {
        const auto compact = leftSidebarWidth < 205;
        const auto controlsX = compact ? 64 : 108;
        const auto right = leftSidebarWidth - (compact ? 8 : 24);
        return { controlsX, contentTop + timelineRulerHeight + 101,
                 juce::jmax(24, right - controlsX), 17 };
    }

    juce::Rectangle<int> trackMeterBounds() const
    {
        const auto slider = trackVolumeSliderBounds();
        return { slider.getX(), slider.getBottom() + 1, slider.getWidth(), 12 };
    }

    void setTrackVolumeFromX(int x)
    {
        const auto slider = trackVolumeSliderBounds();
        const auto volume = juce::jlimit(
            0.0f, 1.0f,
            static_cast<float>(x - slider.getX())
                / static_cast<float>(juce::jmax(1, slider.getWidth())));
        project.setTrackVolume(volume);
        statusMessage = "MIDI Piano volume: "
                        + juce::String(juce::roundToInt(volume * 100.0f)) + "%";
        repaint();
    }

    void setTrackPanFromX(int x)
    {
        const auto bounds = timelinePanBounds();
        const auto pan = juce::jlimit(-1.0f, 1.0f,
            2.0f * static_cast<float>(x - bounds.getX())
                / static_cast<float>(juce::jmax(1, bounds.getWidth())) - 1.0f);
        mixer.setChannelPan(0, pan);
        statusMessage = "MIDI Piano pan: " + juce::String(juce::roundToInt(pan * 100.0f));
        repaint();
    }

    void setMixerVolumeFromY(int y, int channel)
    {
        const auto volume = mixerPresenter.volumeForY(
            mixerPanelArea().withTrimmedTop(4), 0, y, channel);
        mixer.setChannelVolume(static_cast<size_t>(channel), volume);
        statusMessage = mixer.channel(static_cast<size_t>(channel)).name + " volume: "
                        + juce::String(juce::roundToInt(volume * 100.0f)) + "%";
        repaint();
    }

    void setMixerPanFromX(int x, int channel)
    {
        const auto pan = mixerPresenter.panForX(
            mixerPanelArea().withTrimmedTop(4), 0, x, channel);
        mixer.setChannelPan(static_cast<size_t>(channel), pan);
        statusMessage = mixer.channel(static_cast<size_t>(channel)).name + " pan: "
                        + juce::String(juce::roundToInt(pan * 100.0f));
        repaint();
    }

    int currentInspectorWidth() const
    {
        if (inspectorCollapsed) return collapsedInspectorWidth;
        switch (currentView)
        {
            case View::timeline: return timelineInspectorWidth;
            case View::midi:     return midiInspectorWidth;
        }
        return timelineInspectorWidth;
    }

    void syncScrollbars()
    {
        double timelineContentEnd = 0.0;
        int highestTrack = 0;
        for (const auto& clip : clips)
        {
            timelineContentEnd = juce::jmax(timelineContentEnd,
                                             clip.startBeat + clip.lengthBeats);
            highestTrack = juce::jmax(highestTrack, clip.track);
        }
        timelineHorizontalNeeded = timeline.getVisibleBeats() + 0.001 < timelineContentEnd;
        const auto timelineViewportHeight = juce::jmax(
            0, timelineEditorArea().getHeight()
                   - (timelineHorizontalNeeded ? scrollbarThickness : 0));
        const auto timelineContentHeight = timelineRulerHeight
                                           + (highestTrack + 1) * timelineLaneHeight();
        timelineVerticalNeeded = timelineContentHeight > timelineViewportHeight;
        if (!timelineHorizontalNeeded && timeline.getStartBeat() != 0.0)
            timeline.setViewport(0.0, timeline.getVisibleBeats());

        const auto timelineRangeEnd = juce::jmax(timelineContentEnd,
                                                  timeline.getVisibleBeats());
        timelineHorizontalScroll.setRangeLimits(0.0, timelineRangeEnd);
        timelineHorizontalScroll.setCurrentRange(timeline.getStartBeat(),
                                                  timeline.getVisibleBeats(),
                                                  juce::dontSendNotification);
        timelineVerticalScroll.setRangeLimits(0.0, static_cast<double>(timelineContentHeight));
        timelineVerticalScroll.setCurrentRange(0.0, static_cast<double>(timelineViewportHeight),
                                                juce::dontSendNotification);

        double midiContentEnd = 0.0;
        int lowestNote = 127;
        int highestNote = 0;
        for (const auto& note : notes)
        {
            midiContentEnd = juce::jmax(midiContentEnd, note.beat + note.length);
            lowestNote = juce::jmin(lowestNote, note.note);
            highestNote = juce::jmax(highestNote, note.note);
        }
        midiHorizontalNeeded = midiEditor.getVisibleBeats() + 0.001 < midiContentEnd;
        const auto noteSpan = notes.empty() ? 0.0
                                            : static_cast<double>(highestNote - lowestNote + 1);
        midiVerticalNeeded = midiEditor.getVisibleNoteRows() + 0.001 < noteSpan;
        if (!midiHorizontalNeeded && midiEditor.getStartBeat() != 0.0)
            midiEditor.setViewport(0.0, midiEditor.getVisibleBeats(),
                                   midiEditor.getTopNote(), midiEditor.getVisibleNoteRows());

        const auto midiRangeEnd = juce::jmax(midiContentEnd, midiEditor.getVisibleBeats());
        midiHorizontalScroll.setRangeLimits(0.0, midiRangeEnd);
        midiHorizontalScroll.setCurrentRange(midiEditor.getStartBeat(),
                                              midiEditor.getVisibleBeats(),
                                              juce::dontSendNotification);
        midiVerticalScroll.setRangeLimits(0.0, 127.0);
        midiVerticalScroll.setCurrentRange(127.0 - midiEditor.getTopNote(),
                                            midiEditor.getVisibleNoteRows(),
                                            juce::dontSendNotification);

        timelineHorizontalScroll.setVisible(currentView == View::timeline
                                             && timelineHorizontalNeeded);
        timelineVerticalScroll.setVisible(currentView == View::timeline
                                           && timelineVerticalNeeded);
        midiHorizontalScroll.setVisible(currentView == View::midi
                                         && midiHorizontalNeeded);
        midiVerticalScroll.setVisible(currentView == View::midi
                                       && midiVerticalNeeded);
    }

    juce::Rectangle<int> metronomeButtonBounds() const
    {
        const auto toolY = titleBarHeight + menuBarHeight + 14;
        return { getWidth() - 375, toolY, 40, 46 };
    }

    int timelineLaneHeight() const
    {
        const auto baseHeight = juce::jlimit(126, 164, juce::roundToInt(getHeight() * 0.165));
        return juce::roundToInt(baseHeight * timeline.getTrackHeightScale());
    }

    juce::Rectangle<int> arrangementArea() const
    {
        auto area = timelineArea();
        area.removeFromTop(timelineRulerHeight);
        return area;
    }

    juce::Rectangle<int> timelineRulerArea() const
    {
        return timelineArea().withHeight(timelineRulerHeight);
    }

    juce::Rectangle<int> midiGridArea() const
    {
        auto area = juce::Rectangle<int>(0, contentTop, getWidth(),
                                         getHeight() - contentTop - statusBarHeight);
        area.removeFromRight(currentInspectorWidth()
                             + (midiVerticalNeeded ? scrollbarThickness : 0));
        if (midiHorizontalNeeded) area.removeFromBottom(scrollbarThickness);
        area.removeFromBottom(150);
        area.removeFromLeft(midiKeyboardWidth);
        return area.withTrimmedTop(midiHeaderHeight + midiRulerHeight);
    }

    juce::Rectangle<int> midiRulerArea() const
    {
        auto area = juce::Rectangle<int>(0, contentTop, getWidth(),
                                         getHeight() - contentTop - statusBarHeight);
        area.removeFromRight(currentInspectorWidth()
                             + (midiVerticalNeeded ? scrollbarThickness : 0));
        if (midiHorizontalNeeded) area.removeFromBottom(scrollbarThickness);
        area.removeFromLeft(midiKeyboardWidth);
        area.removeFromTop(midiHeaderHeight);
        return area.removeFromTop(midiRulerHeight);
    }

    juce::Rectangle<int> midiEditorSeekArea() const
    {
        auto area = juce::Rectangle<int>(0, contentTop, getWidth(),
                                         getHeight() - contentTop - statusBarHeight);
        area.removeFromRight(currentInspectorWidth()
                             + (midiVerticalNeeded ? scrollbarThickness : 0));
        if (midiHorizontalNeeded) area.removeFromBottom(scrollbarThickness);
        area.removeFromLeft(midiKeyboardWidth);
        area.removeFromTop(midiHeaderHeight + midiRulerHeight);
        return area;
    }

    juce::Rectangle<int> midiNoteBounds(int index, juce::Rectangle<int> grid) const
    {
        const auto& note = notes[static_cast<size_t>(index)];
        const auto rowHeight = static_cast<double>(grid.getHeight()) / midiEditor.getVisibleNoteRows();
        const auto x = grid.getX() + juce::roundToInt(
            midiEditor.ratioForBeat(note.beat) * grid.getWidth());
        const auto width = juce::jmax(5, juce::roundToInt(
            note.length / midiEditor.getVisibleBeats() * grid.getWidth()));
        const auto y = grid.getY() + juce::roundToInt(midiEditor.rowForNote(note.note) * rowHeight) + 2;
        const auto height = juce::jmax(4, juce::roundToInt(rowHeight) - 4);
        return { x, y, width, height };
    }

    double beatAtX(int x) const
    {
        const auto area = arrangementArea();
        const auto normalised = juce::jlimit(0.0, 1.0,
            static_cast<double>(x - area.getX()) / static_cast<double>(juce::jmax(1, area.getWidth())));
        return timeline.beatAtRatio(normalised);
    }

    void setTimelinePlayheadFromX(int x)
    {
        transport.setPlayheadBeats(timeline.snapToGrid(beatAtX(x)));
        statusMessage = "Playhead at beat "
                        + juce::String(transport.getPlayheadBeats() + 1.0, 2);
        repaint();
    }

    void setMidiPlayheadFromX(int x)
    {
        const auto grid = midiGridArea();
        const auto ratio = juce::jlimit(0.0, 1.0,
            static_cast<double>(x - grid.getX())
            / static_cast<double>(juce::jmax(1, grid.getWidth())));
        const auto sourceBeat = juce::jlimit(
            0.0, projectLengthBeats - project.getMidiClipStart(),
            midiEditor.snapToGrid(midiEditor.beatAtRatio(ratio)));
        transport.setPlayheadBeats(project.getMidiClipStart() + sourceBeat);
        statusMessage = "Playhead at beat "
                        + juce::String(project.getMidiClipStart() + sourceBeat + 1.0, 2);
        repaint();
    }

    void beginMarquee(MarqueeTarget target, juce::Point<int> start)
    {
        marqueeTarget = target;
        marqueeStart = start;
        marqueeBounds = {};
        marqueeActive = false;
    }

    void updateMarquee(juce::Point<int> current)
    {
        constexpr int dragThreshold = 4;
        if (!marqueeActive && marqueeStart.getDistanceFrom(current) < dragThreshold)
            return;

        marqueeActive = true;
        marqueeBounds = juce::Rectangle<int>::leftTopRightBottom(
            juce::jmin(marqueeStart.x, current.x), juce::jmin(marqueeStart.y, current.y),
            juce::jmax(marqueeStart.x, current.x), juce::jmax(marqueeStart.y, current.y));

        std::set<size_t> selection;
        if (marqueeTarget == MarqueeTarget::midi)
        {
            const auto grid = midiGridArea();
            marqueeBounds = marqueeBounds.getIntersection(midiEditorSeekArea());
            for (size_t i = 0; i < notes.size(); ++i)
                if (marqueeBounds.intersects(midiNoteBounds(static_cast<int>(i), grid)))
                    selection.insert(i);
            midiEditor.setSelection(std::move(selection));
            statusMessage = juce::String(midiEditor.selectionSize()) + " MIDI note(s) selected";
        }
        else
        {
            marqueeBounds = marqueeBounds.getIntersection(timelineArea());
            for (size_t i = 0; i < clips.size(); ++i)
                if (marqueeBounds.intersects(clipBounds(static_cast<int>(i))))
                    selection.insert(i);
            timeline.setSelection(std::move(selection));
            statusMessage = juce::String(timeline.selectionSize()) + " timeline clip(s) selected";
        }
        repaint();
    }

    void drawMarquee(juce::Graphics& g, MarqueeTarget target)
    {
        if (!marqueeActive || marqueeTarget != target || marqueeBounds.isEmpty())
            return;
        g.setColour(palette::violet.withAlpha(0.16f));
        g.fillRect(marqueeBounds);
        g.setColour(palette::violet.brighter(0.18f));
        g.drawRect(marqueeBounds, 1);
    }

    juce::Rectangle<int> clipBounds(int index) const
    {
        const auto& clip = clips[static_cast<size_t>(index)];
        const auto area = arrangementArea();
        const auto laneHeight = timelineLaneHeight();
        const auto x = area.getX() + juce::roundToInt(
            timeline.ratioForBeat(clip.startBeat) * area.getWidth());
        const auto width = juce::jmax(10, juce::roundToInt(
            clip.lengthBeats / timeline.getVisibleBeats() * area.getWidth()));
        return { x, area.getY() + clip.track * laneHeight + 16, width, laneHeight - 32 };
    }

    void syncMidiClipPosition(int index)
    {
        if (index == 0)
            project.setMidiClipStart(clips[0].startBeat);
    }


    void paintTimeline(juce::Graphics& g, juce::Rectangle<int> area)
    {
        timelinePresenter.paint(g,
            { area, trackVolumeSliderBounds(), trackMeterBounds(), marqueeBounds,
              leftSidebarWidth, currentInspectorWidth(), scrollbarThickness,
              timelineRulerHeight, timelineLaneHeight(),
              timelineVerticalNeeded, timelineHorizontalNeeded, inspectorCollapsed,
              marqueeActive && marqueeTarget == MarqueeTarget::timeline,
              projectLengthBeats, firstMidiClipLength, audioEngine.getTrackPeak(),
              project, transport, timeline, clips, notes });
    }
    void paintMixer(juce::Graphics& g, juce::Rectangle<int> area)
    {
        mixerPresenter.paint(g, area, 0, true,
                             mixer, audioEngine.getTrackPeak(), audioEngine.getMasterPeak());
    }

    void paintMidiEditor(juce::Graphics& g, juce::Rectangle<int> area)
    {
        midiPresenter.paint(g,
            { area, marqueeBounds, currentInspectorWidth(), scrollbarThickness,
              midiKeyboardWidth, midiHeaderHeight, midiRulerHeight,
              midiVerticalNeeded, midiHorizontalNeeded, inspectorCollapsed,
              marqueeActive && marqueeTarget == MarqueeTarget::midi,
              project, transport, midiEditor, notes });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

std::unique_ptr<juce::Component> createMainComponent(PreviewMode preview)
{
    auto component = std::make_unique<MainComponent>();
    component->setSize(scaledWindowWidth, scaledWindowHeight);
    component->resized();

    switch (preview)
    {
        case PreviewMode::zoomed:         component->setTimelineViewportForPreview(8.0, 8.0); break;
        case PreviewMode::zoomedOut:      component->setTimelineViewportForPreview(0.0, 128.0); break;
        case PreviewMode::verticalZoomed: component->setTimelineVerticalZoomForPreview(1.45); break;
        case PreviewMode::clickActive:    component->setClickTrackForPreview(true); break;
        case PreviewMode::midiEditor:     component->showMidiEditorForPreview(); break;
        case PreviewMode::mixer:          component->showMixerForPreview(); break;
        case PreviewMode::narrowSidebar:  component->showNarrowSidebarForPreview(); break;
        case PreviewMode::normal:         break;
    }

    return component;
}
}
