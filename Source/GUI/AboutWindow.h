#pragma once

#include <JuceHeader.h>

namespace GSVST {

class MainWindow;

class AboutWindow
    : public juce::Component
    , public juce::Button::Listener
{
public:
    AboutWindow(MainWindow& mainTab);

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;

private:
    juce::TextButton m_closeButton;

    MainWindow& m_mainWindow;
};

}
