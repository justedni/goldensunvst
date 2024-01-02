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
    auto addButton = [&](auto& button, auto&& text)
    {
        button.setButtonText(text);
        button.addListener(this);
        addAndMakeVisible(button);
    };

    addButton(m_browseSoundfontButton, "Browse soundfont");
    addButton(m_clearSoundfontButton, "Clear");

    addAndMakeVisible(m_labelSoundfont);

    addAndMakeVisible(m_gsModeToggleButton);
    m_gsModeToggleButton.setButtonText("GS mode (enable synths and program names)");
    m_gsModeToggleButton.onClick = [this] { toggleButtonStateChanged(); };

    addAndMakeVisible(m_closeButton);
    m_closeButton.setButtonText("Close");
    m_closeButton.addListener(this);
}

void SettingsWindow::paint(juce::Graphics& g)
{
    CustomLookAndFeel::drawGSBox(g, 0, 0, getWidth(), getHeight());

    g.setFont(14);
    g.setColour(juce::Colours::white);
    g.drawText("General", 10, 5, getWidth() - 20, 20, juce::Justification::centredLeft);
    g.drawText("Soundfont", 10, 48, getWidth() - 20, 20, juce::Justification::centredLeft);

    auto width = getWidth() - 15;
    g.setFont(12);
    g.drawText("Massive thanks to:", 0, 110, width, 20, juce::Justification::centredRight);
    g.setFont(9);
    g.drawText("ipatix: creator of agbplay", 0, 130, width, 20, juce::Justification::centredRight);
    g.drawText("FreeJusticeHere: feedback and sample wisdom", 0, 140, width, 20, juce::Justification::centredRight);
}

void SettingsWindow::resized()
{
    m_gsModeToggleButton.setBounds(10, 25, getWidth(), 20);

    m_browseSoundfontButton.setBounds(10, 70, 120, 20);
    m_labelSoundfont.setBounds(140, 70, getWidth() - 150, 20);
    m_clearSoundfontButton.setBounds(10, 90, 60, 20);

    m_closeButton.setBounds(30, getHeight() - 30, 60, 20);
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

    m_gsModeToggleButton.setToggleState(presets.getGSModeEnabled(), juce::dontSendNotification);
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
        m_mainWindow.closeSettingsWindow();
    }
    else if (button == &m_clearSoundfontButton)
    {
        m_audioProcessor.setSoundfont("");
        refresh();
        m_mainWindow.refreshMainTab();
    }
}

void SettingsWindow::toggleButtonStateChanged()
{
    m_audioProcessor.setGSMode(m_gsModeToggleButton.getToggleState());

    m_mainWindow.refreshMainTab();
}

}
