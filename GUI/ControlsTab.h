#pragma once

#include <JuceHeader.h>
#include "PropertyListener.h"
#include "SliderEx.h"

namespace GSVST {

class Processor;
class MainWindow;

class PresetCombo;
class SliderEx;

class ControlsTab : public juce::Component
    , public juce::Button::Listener
    , public PropertyListener
{
public:
    ControlsTab(Processor& p, MainWindow& e);

    void paint(juce::Graphics&) override;
    void resized() override;

    void channelComboChanged();
    void presetComboChanged();
    void updateCheckboxState();
    void buttonClicked(juce::Button* button) override;
    void reverbComboChanged();

    int getSelectedMidiChannel() const;

    void refresh();
    void refreshPresets();

private:
    void setPropertyVal(EProperty prop, uint8_t val) override;

    std::unique_ptr<juce::ComboBox> m_comboChannel;
    std::unique_ptr<PresetCombo> m_comboPreset;
    std::unique_ptr<juce::ToggleButton> m_tickBoxIgnorePrgChg;

    std::unique_ptr<SliderEx> m_sliderVolume;
    std::unique_ptr<SliderEx> m_sliderPan;
    std::unique_ptr<SliderEx> m_sliderReverb;

    std::unique_ptr<SliderEx> m_sliderAtt;
    std::unique_ptr<SliderEx> m_sliderDec;
    std::unique_ptr<SliderEx> m_sliderSus;
    std::unique_ptr<SliderEx> m_sliderRel;

    std::unique_ptr<SliderEx> m_sliderInitDuty;
    std::unique_ptr<SliderEx> m_sliderDutyBase;
    std::unique_ptr<SliderEx> m_sliderDutyStep;
    std::unique_ptr<SliderEx> m_sliderDepth;

    std::unique_ptr<juce::ComboBox> m_comboReverb;
    std::unique_ptr<juce::ToggleButton> m_changeReverbForAll;

    int m_currentMidiChannel = 0;

    bool m_isPWMSynth = false;
    bool m_reverbSliderEnabled = false;
    int m_detectedBPM = 120;

    Processor& m_audioProcessor;
    MainWindow& m_mainWindow;
};

}
