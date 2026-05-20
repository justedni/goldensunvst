#pragma once

#include <JuceHeader.h>
#include "Processor/Types.h"
#include "LevelMeter.h"

namespace GSVST {

class Processor;
class MainWindow;
class PresetCombo;
struct ProgramInfo;
enum EUITheme : uint8_t;

struct ChannelDesc
{
    std::unique_ptr<PresetCombo> m_presetCombo;
    std::unique_ptr<LevelMeter> m_meter;
    std::string m_deviceName;
    juce::Colour m_deviceColour;

    void updateTheme(EUITheme theme);
};

class GlobalViewTab : public juce::Component,
    public juce::Timer
{
public:
    GlobalViewTab(Processor& p, MainWindow& e);

    void paint(juce::Graphics&) override;
    void resized() override;

    void presetComboChanged(int channel);

    void refresh();
    void refreshPresets();

    void timerCallback() override;

    static juce::Colour getDeviceColour(const std::string& name);

    void updateTheme(EUITheme theme);

private:
    ChannelDesc m_channelDescs[MAX_MIDI_CHANNELS];

    Processor& m_audioProcessor;
    MainWindow& m_mainWindow;

    std::map<int, ProgramInfo> m_overrideProgramInfo;
};

}
