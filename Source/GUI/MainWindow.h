#pragma once

#include <JuceHeader.h>

namespace GSVST {

class Processor;

class ControlsTab;
class GlobalViewTab;
class SettingsWindow;
class AboutWindow;
class CustomLookAndFeel;

class MainWindow  : public juce::AudioProcessorEditor
    , public juce::Timer
{
public:
    MainWindow (Processor&);
    ~MainWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    enum class EPopup : uint8_t { Settings, About };
    void openPopupWindow(EPopup popup);
    void closePopupWindow();

    void refreshMainTab();
    void refreshGlobalTab(bool bRefreshPresets = true);

private:
    void refresh(bool bForce = false);

    std::unique_ptr<juce::TabbedComponent> m_tabbedComponent;

    std::unique_ptr<ControlsTab> m_mainTab;
    std::unique_ptr<GlobalViewTab> m_globalViewTab;
    std::unique_ptr<SettingsWindow> m_settingsWindow;
    std::unique_ptr<AboutWindow> m_aboutWindow;

    std::unique_ptr<juce::TextButton> m_settingsButton;
    std::unique_ptr<juce::TextButton> m_aboutButton;
    juce::Component::SafePointer<juce::DocumentWindow> m_window;

    std::unique_ptr<CustomLookAndFeel> m_customLookAndFeel;

    Processor& m_audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

}