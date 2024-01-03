#include "MainWindow.h"

#include "Processor/Processor.h"
#include "Processor/ReverbEffect.h"

#include "SettingsWindow.h"
#include "ControlsTab.h"
#include "OverviewTab.h"
#include "CustomLookAndFeel.h"

namespace GSVST {

MainWindow::MainWindow (Processor& p)
    : AudioProcessorEditor (&p)
    , m_tabbedComponent(new juce::TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop))
    , m_mainTab(new ControlsTab(p, *this))
    , m_globalViewTab(new GlobalViewTab(p))
    , m_settingsWindow(new SettingsWindow(p, *this))
    , m_settingsButton(new juce::TextButton())
    , m_audioProcessor (p)
{
    setSize (600, 300);

    startTimer(400);

    m_customLookAndFeel.reset(new CustomLookAndFeel());
    juce::LookAndFeel::setDefaultLookAndFeel(m_customLookAndFeel.get());

    addAndMakeVisible(m_tabbedComponent.get());
    m_tabbedComponent->setOrientation(juce::TabbedButtonBar::TabsAtBottom);
    m_tabbedComponent->addTab("Controls", juce::Colours::black, m_mainTab.get(), true);
    m_tabbedComponent->addTab("Overview", juce::Colours::black, m_globalViewTab.get(), true);

    addAndMakeVisible(*m_settingsButton.get());
    m_settingsButton->setButtonText("Settings");
    m_settingsButton->onClick = [this] { openSettingsWindow(); };

    refresh(true);
}


MainWindow::~MainWindow()
{
    if (m_window)
    {
        m_window.deleteAndZero();
    }

    stopTimer();
}

//==============================================================================
void MainWindow::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(14);
    g.drawText("Golden Sun VST", getWidth() / 2, getHeight() - 38, (getWidth() / 2) - 20, 20, juce::Justification::centredRight);
    g.setFont(10);
    g.drawText("Vincent Dortel", getWidth() / 2, getHeight() - 20, (getWidth() / 2) - 20, 20, juce::Justification::centredRight);
}

void MainWindow::resized()
{
    m_tabbedComponent->setBounds(10, 10, getWidth() - 20, getHeight() - 20);

    m_settingsButton->setBounds(245, 265, 90, 20);
    m_settingsWindow->setBounds(0, 0, 400, 200);
}

void MainWindow::timerCallback()
{
    refresh();
}

void MainWindow::refresh(bool bForce)
{
    // Refresh timer
    if (bForce || m_audioProcessor.dataRefreshRequired())
    {
        m_mainTab->refresh();
        m_globalViewTab->refresh();
    }

    if (bForce || m_audioProcessor.presetsRefreshRequired())
    {
        m_mainTab->refreshPresets();
        m_globalViewTab->refreshPresets();
    }
}

void MainWindow::openSettingsWindow()
{
    if (m_window)
    {
        m_window->toFront(true);
    }
    else
    {
        m_window = new juce::DocumentWindow("Settings", juce::Colour(80, 80, 80), false, false);
        m_window->setUsingNativeTitleBar(true);
        addAndMakeVisible(m_window);
        m_settingsWindow->refresh(false);
        m_window->setContentNonOwned(m_settingsWindow.get(), true);
        m_window->centreWithSize(400, 200);
        m_window->setVisible(true);
    }
}

void MainWindow::closeSettingsWindow()
{
    m_window.deleteAndZero();
}

void MainWindow::refreshMainTab()
{
    m_mainTab->refreshPresets();
}

void MainWindow::refreshGlobalTab()
{
    m_globalViewTab->refresh();
}

}
