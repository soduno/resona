#include "MainWindow.h"
#include "MainComponentFactory.h"

namespace resona
{
MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name, juce::Colour(0xff14181c), DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(false);
    setTitleBarHeight(0);
    setDropShadowEnabled(true);
    setContentOwned(createMainComponent().release(), true);
    setResizable(true, true);
    setResizeLimits(1100, 720, 2800, 1800);
    centreWithSize(scaledWindowWidth, scaledWindowHeight);
    setVisible(true);
    if (auto* content = getContentComponent())
        content->grabKeyboardFocus();
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
}
