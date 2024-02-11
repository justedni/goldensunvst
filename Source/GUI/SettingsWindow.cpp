#include "SettingsWindow.h"
#include "CustomLookAndFeel.h"
#include "MainWindow.h"

#include "Processor/Processor.h"
#include "Processor/ReverbEffect.h"
#include "Presets/PresetsHandler.h"

namespace GSVST {

SettingsWindow::SettingsWindow(Processor& p, MainWindow& e)
 : m_audioProcessor(p)
 , m_mainWindow(e)
{
    addAndMakeVisible(m_comboProgramNameMode);
    m_comboProgramNameMode.addItem("SF2", 1);
    m_comboProgramNameMode.addItem("GS", 2);

    m_comboProgramNameMode.onChange = [this] { comboChangedProgramNameMode(); };
    auto mode = static_cast<int>(p.getPresets().getProgramNameMode()) + 1;
    m_comboProgramNameMode.setSelectedId(mode);

    auto addButton = [&](auto& button, auto&& text)
    {
        button.setButtonText(text);
        button.addListener(this);
        addAndMakeVisible(button);
    };

    addButton(m_browseSoundfontButton, "Browse");
    addButton(m_clearSoundfontButton, "Clear");

    addAndMakeVisible(m_labelSoundfont);

    addAndMakeVisible(m_gsSynthModeToggleButton);
    m_gsSynthModeToggleButton.setButtonText("GS (PWM, Tri, Saw)");
    m_gsSynthModeToggleButton.onClick = [this] { toggleButtonStateChanged(&m_gsSynthModeToggleButton); };

    addAndMakeVisible(m_gbSynthModeToggleButton);
    m_gbSynthModeToggleButton.setButtonText("GB (Square)");
    m_gbSynthModeToggleButton.onClick = [this] { toggleButtonStateChanged(&m_gbSynthModeToggleButton); };

    addAndMakeVisible(m_closeButton);
    m_closeButton.setButtonText("Close");
    m_closeButton.addListener(this);
}

void SettingsWindow::paint(juce::Graphics& g)
{
    CustomLookAndFeel::drawGSBox(g, 0, 0, getWidth(), getHeight());

    g.setColour(juce::Colours::white);
    g.setFont(14);
    g.drawText("Soundfont:", 10, 10, 130, 20, juce::Justification::centredLeft);

    g.setFont(10);
    g.drawText("Use program names from:", 10, 60, 180, 20, juce::Justification::centredLeft);
    g.drawText("Auto-replace synths with better ones:", 10, 80, 300, 20, juce::Justification::centredLeft);
}

void SettingsWindow::resized()
{
    m_labelSoundfont.setBounds(140, 10, getWidth() - 150, 20);

    m_browseSoundfontButton.setBounds(10, 35, 60, 20);
    m_clearSoundfontButton.setBounds(90, 35, 60, 20);

    m_comboProgramNameMode.setBounds(210, 60, 70, 20);

    m_gsSynthModeToggleButton.setBounds(10, 100, getWidth() / 2, 20);
    m_gbSynthModeToggleButton.setBounds(getWidth() / 2, 100, getWidth() / 2, 20);

    m_closeButton.setBounds(getWidth() / 2 - 30, getHeight() - 30, 60, 20);
}

void SettingsWindow::refresh(bool /*bForce*/)
{
    const auto& presets = m_audioProcessor.getPresets();

    auto soundFontPath = presets.getSoundFontPath();
    juce::File file(soundFontPath);
    juce::String text = "No soundfont";
    if (!soundFontPath.empty() && file.exists())
        text = file.getFileName();

    m_labelSoundfont.setText(text, juce::dontSendNotification);

    m_gsSynthModeToggleButton.setToggleState(presets.getAutoReplaceGSSynths(), juce::dontSendNotification);
    m_gbSynthModeToggleButton.setToggleState(presets.getAutoReplaceGBSynths(), juce::dontSendNotification);
}

void SettingsWindow::buttonClicked(juce::Button* button)
{
    if (button == &m_browseSoundfontButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Please select a GS2 soundfont...",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.sf2");

        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(folderChooserFlags, [&](const juce::FileChooser& chooser)
        {
            juce::File sf2File(chooser.getResult());

            m_audioProcessor.setSoundfont(std::string(sf2File.getFullPathName().getCharPointer()));
            refresh();
            m_mainWindow.refreshMainTab();
        });
    }
    else if (button == &m_closeButton)
    {
        m_mainWindow.closePopupWindow();
    }
    else if (button == &m_clearSoundfontButton)
    {
        m_audioProcessor.setSoundfont("");
        refresh();
        m_mainWindow.refreshMainTab();
        m_mainWindow.refreshGlobalTab();
    }
}

void SettingsWindow::toggleButtonStateChanged(juce::ToggleButton* button)
{
    if (button == &m_gsSynthModeToggleButton)
        m_audioProcessor.setAutoReplaceGSSynths(button->getToggleState());
    else if (button == &m_gbSynthModeToggleButton)
        m_audioProcessor.setAutoReplaceGBSynths(button->getToggleState());

    m_mainWindow.refreshMainTab();
    m_mainWindow.refreshGlobalTab();
}

void SettingsWindow::comboChangedProgramNameMode()
{
    auto mode = static_cast<EProgramNameMode>(m_comboProgramNameMode.getSelectedId() - 1);
    m_audioProcessor.setProgramNameMode(mode);

    m_mainWindow.refreshMainTab();
    m_mainWindow.refreshGlobalTab();
}


}
