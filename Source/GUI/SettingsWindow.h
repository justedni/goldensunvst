#pragma once

#include <JuceHeader.h>

namespace GSVST {

class MainWindow;
class Processor;
class ComboLookAndFeel;

class SettingsWindow
    : public juce::Component
    , public juce::Button::Listener
{
public:
    SettingsWindow(Processor& p, MainWindow& mainTab);
    ~SettingsWindow();

    void paint(juce::Graphics&) override;
    void resized() override;

    void refresh(bool bForce = false);

private:
    void comboChangedProgramNameMode();
    void comboChangedTheme();
    void buttonClicked(juce::Button* button) override;
    void toggleButtonStateChanged(juce::ToggleButton* button);

    juce::Label m_labelSoundfont;
    juce::TextButton m_browseSoundfontButton;
    juce::TextButton m_clearSoundfontButton;

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::Label m_labelProgramNameMode;
    juce::ComboBox m_comboProgramNameMode;

    juce::Label m_labelTheme;
    juce::ComboBox m_comboTheme;

    juce::Label m_labelAutoReplaceSynths;
    juce::ToggleButton m_gsSynthModeToggleButton;
    juce::ToggleButton m_gbSynthModeToggleButton;
    juce::ToggleButton m_hideUnknownPresetsButton;

    std::unique_ptr<ComboLookAndFeel> m_lookAndFeel;

    std::map<int, std::string> m_id_to_gamename;

    Processor& m_audioProcessor;
    MainWindow& m_mainWindow;
};

}
