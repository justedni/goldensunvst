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
    void comboChangedProgramNameMode();
    void buttonClicked(juce::Button* button) override;
    void toggleButtonStateChanged(juce::ToggleButton* button);

    juce::ComboBox m_comboProgramNameMode;
    juce::TextButton m_browseSoundfontButton;
    juce::TextButton m_clearSoundfontButton;
    juce::Label m_labelSoundfont;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::ToggleButton m_gsSynthModeToggleButton;
    juce::ToggleButton m_gbSynthModeToggleButton;
    juce::ToggleButton m_hideUnknownPresetsButton;

    juce::TextButton m_closeButton;

    std::map<int, std::string> m_id_to_gamename;

    Processor& m_audioProcessor;
    MainWindow& m_mainWindow;
};

}
