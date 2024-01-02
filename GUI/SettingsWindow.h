#pragma once

#include <JuceHeader.h>

namespace GSVST {

class MainWindow;
class Processor;

class SettingsWindow
    : public juce::Component
    , public juce::Button::Listener
{
public:
    SettingsWindow(Processor& p, MainWindow& mainTab);

    void paint(juce::Graphics&) override;
    void resized() override;

    void refresh(bool bForce = false);

private:
    void buttonClicked(juce::Button* button) override;
    void toggleButtonStateChanged();

    juce::TextButton m_browseSoundfontButton;
    juce::TextButton m_clearSoundfontButton;
    juce::Label m_labelSoundfont;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::ToggleButton m_gsModeToggleButton;

    juce::TextButton m_closeButton;

    Processor& m_audioProcessor;
    MainWindow& m_mainWindow;
};

}
