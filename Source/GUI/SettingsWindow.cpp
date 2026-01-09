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

    const auto& handler = p.getPresets();
    const auto& gamesList = handler.getGamesProgramList();
    const auto& selectedGame = handler.getSelectedGame();
    int selectedGameId = 1;

    m_comboProgramNameMode.addItem("SF2", 1);
    int id = 2;
    for (auto& game : gamesList)
    {
        m_id_to_gamename[id] = game.first;
        m_comboProgramNameMode.addItem(game.first, id);

        if (selectedGame == game.first)
            selectedGameId = id;

        id++;
    }

    m_comboProgramNameMode.onChange = [this] { comboChangedProgramNameMode(); };
    m_comboProgramNameMode.setSelectedId(selectedGameId);

    auto addButton = [&](auto& button, auto&& text)
    {
        button.setButtonText(text);
        button.addListener(this);
        addAndMakeVisible(button);
    };

    addButton(m_browseSoundfontButton, "Browse");
    addButton(m_clearSoundfontButton, "Clear");

    m_labelProgramNameMode.setText("Program names:", juce::dontSendNotification);
    m_labelTheme.setText("UI:", juce::dontSendNotification);
    m_labelAutoReplaceSynths.setText("Auto-replace synths with better ones:", juce::dontSendNotification);

    addAndMakeVisible(m_labelSoundfont);
    addAndMakeVisible(m_labelProgramNameMode);
    addAndMakeVisible(m_labelTheme);
    addAndMakeVisible(m_labelAutoReplaceSynths);

    addAndMakeVisible(m_gsSynthModeToggleButton);
    m_gsSynthModeToggleButton.setButtonText("GS (PWM, Tri, Saw)");
    m_gsSynthModeToggleButton.onClick = [this] { toggleButtonStateChanged(&m_gsSynthModeToggleButton); };

    addAndMakeVisible(m_gbSynthModeToggleButton);
    m_gbSynthModeToggleButton.setButtonText("GB (Square)");
    m_gbSynthModeToggleButton.onClick = [this] { toggleButtonStateChanged(&m_gbSynthModeToggleButton); };

    addAndMakeVisible(m_hideUnknownPresetsButton);
    m_hideUnknownPresetsButton.setButtonText("Hide unknown instruments");
    m_hideUnknownPresetsButton.onClick = [this] { toggleButtonStateChanged(&m_hideUnknownPresetsButton); };

    {
        m_comboTheme.addItem("GS", EUITheme::GS);
        m_comboTheme.addItem("CotM", EUITheme::CoTM);
        m_comboTheme.onChange = [this] { comboChangedTheme(); };
        addAndMakeVisible(m_comboTheme);
    }

    m_lookAndFeel.reset(new ComboLookAndFeel(&e));
    getLookAndFeel().setDefaultSansSerifTypeface(m_lookAndFeel->getTypeface());

    m_comboProgramNameMode.setLookAndFeel(m_lookAndFeel.get());
    m_comboTheme.setLookAndFeel(m_lookAndFeel.get());
}

SettingsWindow::~SettingsWindow()
{
    m_comboProgramNameMode.setLookAndFeel(nullptr);
    m_comboTheme.setLookAndFeel(nullptr);
}

void SettingsWindow::paint(juce::Graphics& g)
{
    CustomLookAndFeel::drawGSBox(g, 0, 0, getWidth(), getHeight());
}

void SettingsWindow::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    m_labelSoundfont.setBounds(bounds.removeFromTop(20));

    auto buttonArea = bounds.removeFromTop(20);
    m_browseSoundfontButton.setBounds(buttonArea.removeFromLeft(60).withWidth(60));
    m_clearSoundfontButton.setBounds(buttonArea.withWidth(60));

    auto getIdealWidth = [](auto& label)
    {
        juce::Font font = label.getFont();
        return font.getStringWidth(label.getText());
    };

    auto programModeArea = bounds.removeFromTop(20);
    m_labelProgramNameMode.setBounds(programModeArea.removeFromLeft(150));
    m_comboProgramNameMode.setBounds(programModeArea.withWidth(180));

    auto themeArea = bounds.removeFromTop(20);
    m_labelTheme.setBounds(themeArea.removeFromLeft(35));
    m_comboTheme.setBounds(themeArea.withWidth(100));

    bounds.removeFromTop(10);

    m_labelAutoReplaceSynths.setBounds(bounds.removeFromTop(20));
    auto firstRow = bounds.removeFromTop(20);
    const auto halfWidth = getWidth() / 2;
    m_gsSynthModeToggleButton.setBounds(firstRow.removeFromLeft(halfWidth));
    m_gbSynthModeToggleButton.setBounds(firstRow);

    auto secondRow = bounds.removeFromTop(20);
    m_hideUnknownPresetsButton.setBounds(secondRow);
}

void SettingsWindow::refresh(bool /*bForce*/)
{
    const auto& presets = m_audioProcessor.getPresets();

    auto soundFontPath = presets.getSoundFontPath();
    juce::File file(soundFontPath);
    juce::String text = "Soundfont: ";
    if (!soundFontPath.empty() && file.exists())
        text += file.getFileName();
    else
        text += "none";

    m_labelSoundfont.setText(text, juce::dontSendNotification);

    m_gsSynthModeToggleButton.setToggleState(presets.getAutoReplaceGSSynths(), juce::dontSendNotification);
    m_gbSynthModeToggleButton.setToggleState(presets.getAutoReplaceGBSynths(), juce::dontSendNotification);
    m_hideUnknownPresetsButton.setToggleState(presets.getHideUnknownInstruments(), juce::dontSendNotification);
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
    else if (button == &m_hideUnknownPresetsButton)
        m_audioProcessor.setHideUnknownInstruments(button->getToggleState());

    m_mainWindow.refreshMainTab();
    m_mainWindow.refreshGlobalTab();
}

void SettingsWindow::comboChangedProgramNameMode()
{
    auto gameName = m_id_to_gamename[m_comboProgramNameMode.getSelectedId()];
    m_audioProcessor.setSelectedGame(gameName);

    m_mainWindow.refreshMainTab();
    m_mainWindow.refreshGlobalTab();
}

void SettingsWindow::comboChangedTheme()
{
    auto theme = m_comboTheme.getSelectedId();
    m_mainWindow.setSelectedTheme(static_cast<EUITheme>(theme));
    sendLookAndFeelChange();
    resized();
}

}
