#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace resona
{
class MainWindow final : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name);
    void closeButtonPressed() override;
};
}
