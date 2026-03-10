#include "ui/MainComponentFactory.h"
#include "ui/MainWindow.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace resona
{
namespace
{
PreviewMode previewModeFor(const juce::String& commandLine)
{
    if (commandLine.contains("--screenshot-narrow-sidebar")) return PreviewMode::narrowSidebar;
    if (commandLine.contains("--screenshot-midi-editor"))     return PreviewMode::midiEditor;
    if (commandLine.contains("--screenshot-mixer"))          return PreviewMode::mixer;
    if (commandLine.contains("--screenshot-click-active"))    return PreviewMode::clickActive;
    if (commandLine.contains("--screenshot-zoomed-out"))      return PreviewMode::zoomedOut;
    if (commandLine.contains("--screenshot-vertical-zoomed")) return PreviewMode::verticalZoomed;
    if (commandLine.contains("--screenshot-zoomed"))          return PreviewMode::zoomed;
    return PreviewMode::normal;
}

juce::String previewFileName(PreviewMode mode)
{
    switch (mode)
    {
        case PreviewMode::narrowSidebar:  return "Resona-preview-narrow-sidebar.png";
        case PreviewMode::midiEditor:     return "Resona-preview-midi-editor.png";
        case PreviewMode::mixer:          return "Resona-preview-mixer.png";
        case PreviewMode::clickActive:    return "Resona-preview-click-active.png";
        case PreviewMode::zoomedOut:      return "Resona-preview-zoomed-out.png";
        case PreviewMode::verticalZoomed: return "Resona-preview-vertical-zoomed.png";
        case PreviewMode::zoomed:         return "Resona-preview-zoomed.png";
        case PreviewMode::normal:         return "Resona-preview.png";
    }
    return "Resona-preview.png";
}
}

class ResonaApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Resona"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        juce::Desktop::getInstance().setGlobalScaleFactor(uiScale);

        if (commandLine.contains("--screenshot"))
        {
            const auto mode = previewModeFor(commandLine);
            auto preview = createMainComponent(mode);
            const auto image = preview->createComponentSnapshot(preview->getLocalBounds(), true, uiScale);
            const auto output = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                                    .getSiblingFile(previewFileName(mode));
            output.deleteFile();
            if (auto stream = output.createOutputStream())
            {
                juce::PNGImageFormat format;
                format.writeImageToStream(image, *stream);
            }
            juce::MessageManager::callAsync([this] { quit(); });
            return;
        }

        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override { mainWindow.reset(); }
    void systemRequestedQuit() override { quit(); }

private:
    std::unique_ptr<MainWindow> mainWindow;
};
}

START_JUCE_APPLICATION(resona::ResonaApplication)
