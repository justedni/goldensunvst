#include "ControlsTab.h"

#include "Processor/Types.h"
#include "Processor/Instrument.h"
#include "Processor/Processor.h"
#include "Processor/ReverbEffect.h"

#include "PresetCombo.h"
#include "CustomLookAndFeel.h"
#include "MainWindow.h"

namespace GSVST {

ControlsTab::ControlsTab(Processor& p, MainWindow& e)
    : m_comboChannel(new juce::ComboBox())
    , m_comboPreset(new PresetCombo())
    , m_tickBoxIgnorePrgChg(new juce::ToggleButton())
    , m_comboReverb(new juce::ComboBox())
    , m_changeReverbForAll(new juce::ToggleButton())
    , m_audioProcessor(p)
    , m_mainWindow(e)
{
    addAndMakeVisible(*m_comboChannel.get());

    for (int i = 1; i <= MAX_MIDI_CHANNELS; i++)
    {
        m_comboChannel->addItem(std::to_string(i), i);
    }

    m_comboChannel->onChange = [this] { channelComboChanged(); };
    m_comboChannel->setSelectedId(1);

    addAndMakeVisible(*m_comboPreset.get());

    m_comboPreset->refresh(m_audioProcessor.getPresets());
    m_comboPreset->onChange = [this] { presetComboChanged(); };

    auto defaultSelection = p.GetChannelState(0).getCurrentPreset();
    m_comboPreset->setSelectedId(defaultSelection);

    addAndMakeVisible(*m_tickBoxIgnorePrgChg.get());
    m_tickBoxIgnorePrgChg->setButtonText("Ignore Program Change");
    m_tickBoxIgnorePrgChg->onClick = [this] { updateCheckboxState(); };

    auto addSlider = [&](auto& slider, auto&& name, auto prop, auto min, auto max, auto step)
    {
        slider.reset(new SliderEx(prop, std::move(name), this, min, max, step));
        addAndMakeVisible(slider.get());
    };

    addSlider(m_sliderVolume, "Volume", EProperty::Volume, 0.0, 127.0, 1.0);
    addSlider(m_sliderPan, "Pan", EProperty::Pan, 0.0, 127.0, 1.0);
    addSlider(m_sliderReverb, "", EProperty::Reverb, 0.0, 127.0, 1.0);

    addSlider(m_sliderAtt, "Att", EProperty::Attack, 0.0, 255.0, 1.0);
    addSlider(m_sliderDec, "Dec", EProperty::Decay, 0.0, 255.0, 1.0);
    addSlider(m_sliderSus, "Sus", EProperty::Sustain, 0.0, 255.0, 1.0);
    addSlider(m_sliderRel, "Rel", EProperty::Release, 0.0, 255.0, 1.0);

    addSlider(m_sliderInitDuty, "Duty Init", EProperty::InitDuty, 0.0, 256.0, 32.0); // 64
    addSlider(m_sliderDutyBase, "Base", EProperty::DutyBase, 0.0, 128.0, 16.0);
    addSlider(m_sliderDutyStep, "Step", EProperty::DutyStep, -64.0, 64.0, 16.0);
    addSlider(m_sliderDepth, "Depth", EProperty::Depth, -0.0, 224, 8.0); // 64

    addAndMakeVisible(*m_comboReverb.get());

    m_comboReverb->addItem("No Reverb", 1);
    m_comboReverb->addItem("Default Reverb", 2);
    m_comboReverb->addItem("GS1 Reverb", 3);
    m_comboReverb->addItem("GS2 Reverb", 4);

    m_comboReverb->onChange = [this] { reverbComboChanged(); };
    m_comboReverb->setSelectedId(2);

    m_changeReverbForAll->setButtonText("Apply to all (1-16)");
    addAndMakeVisible(*m_changeReverbForAll.get());
    m_changeReverbForAll->setToggleState(true, juce::dontSendNotification);
    m_changeReverbForAll->addListener(this);
}

void ControlsTab::paint(juce::Graphics& g)
{
    CustomLookAndFeel::drawGSBox(g, 0, 0, getWidth(), 50);
    CustomLookAndFeel::drawGSBox(g, 370, 50, getWidth() - 370, 40);

    g.setColour(juce::Colours::white);
    g.setFont(12);
    g.drawText("Midi channel / Program", 8, 5, getWidth() - 20, 20, juce::Justification::centredLeft);

    g.drawText(juce::String::formatted("BPM used for LFO: %d", m_detectedBPM), 355, 50, getWidth() - 370, 40, juce::Justification::centredRight);

    CustomLookAndFeel::drawGSBox(g, 0, 51, 290, 99);
    CustomLookAndFeel::drawGSBox(g, 0, 151, 290, 100);

    if (m_isPWMSynth)
        CustomLookAndFeel::drawGSBox(g, 295, 151, getWidth() - 295, 100);
}

void ControlsTab::resized()
{
    auto currentX = 10;

    m_tickBoxIgnorePrgChg->setBounds(340, 25, 250, 20);

    m_comboChannel->setBounds(currentX, 25, 60, 20);
    auto offsetX = 80;
    m_comboPreset->setBounds(offsetX, 25, 250, 20);

    const auto sliderWidth = 70;
    const auto sliderHeight = 90;
    const auto firstRowY = 52;
    const auto spacing = 70;

    currentX = 5;
    m_sliderVolume->setBounds(currentX, firstRowY, sliderWidth, sliderHeight);
    currentX += spacing;
    m_sliderPan->setBounds(currentX, firstRowY, sliderWidth, sliderHeight);
    currentX += spacing;

    m_comboReverb->setBounds(currentX, 56, 140, 20);
    m_changeReverbForAll->setBounds(currentX, 85, 80, 45);

    currentX += spacing;
    m_sliderReverb->setBounds(currentX, firstRowY, sliderWidth, sliderHeight);

    const auto secondRowY = 152;
    currentX = 5;
    m_sliderAtt->setBounds(currentX, secondRowY, sliderWidth, sliderHeight);
    currentX += spacing;
    m_sliderDec->setBounds(currentX, secondRowY, sliderWidth, sliderHeight);
    currentX += spacing;
    m_sliderSus->setBounds(currentX, secondRowY, sliderWidth, sliderHeight);
    currentX += spacing;
    m_sliderRel->setBounds(currentX, secondRowY, sliderWidth, sliderHeight);

    currentX = 305;
    m_sliderInitDuty->setBounds(currentX, secondRowY, sliderWidth + 5, sliderHeight);
    currentX += 60;
    m_sliderDutyBase->setBounds(currentX, secondRowY, sliderWidth, sliderHeight);
    currentX += 60;
    m_sliderDutyStep->setBounds(currentX, secondRowY, sliderWidth, sliderHeight);
    currentX += (spacing + 15);
    m_sliderDepth->setBounds(currentX, secondRowY, sliderWidth, sliderHeight);
}


void ControlsTab::channelComboChanged()
{
    auto selectedChannel = m_comboChannel->getSelectedId();

    auto& channelState = m_audioProcessor.GetChannelState(selectedChannel - 1);
    auto currentChannelPreset = channelState.getCurrentPreset();
    m_comboPreset->setSelectedId(currentChannelPreset);

    refresh();
}

void ControlsTab::presetComboChanged()
{
    auto selectedChannel = m_comboChannel->getSelectedId();
    auto selectedId = m_comboPreset->getSelectedId();

    auto& channelState = m_audioProcessor.GetChannelState(selectedChannel - 1);
    channelState.setPreset(selectedId, m_audioProcessor.getPresets());

    refresh();
    m_mainWindow.refreshGlobalTab();
}

void ControlsTab::updateCheckboxState()
{
    m_audioProcessor.setIgnoreProgramChange(m_tickBoxIgnorePrgChg->getToggleState());
}

void ControlsTab::setPropertyVal(EProperty prop, uint8_t val)
{
    auto& channelState = m_audioProcessor.GetChannelState(m_currentMidiChannel);
    auto adsr = channelState.getADSR();
    auto pwmData = channelState.getPWMData();

    switch (prop)
    {
    case EProperty::Volume:
        channelState.setVolume(val);
        break;
    case EProperty::Pan:
        channelState.setPan(val);
        break;
    case EProperty::Reverb:
        channelState.setReverbLevel(val);
        break;
    case EProperty::Attack:   adsr.att = val; break;
    case EProperty::Decay:    adsr.dec = val; break;
    case EProperty::Sustain:  adsr.sus = val; break;
    case EProperty::Release:  adsr.rel = val; break;
    case EProperty::InitDuty: pwmData.initDuty = val; break;
    case EProperty::DutyBase: pwmData.dutyBase = val; break;
    case EProperty::DutyStep: pwmData.dutyStep = val; break;
    case EProperty::Depth:    pwmData.depth = val; break;
    }

    auto adsrChanged = [&prop]() {
        return (prop == EProperty::Attack || prop == EProperty::Decay
            || prop == EProperty::Sustain || prop == EProperty::Release);
    };

    auto pwmChanged = [&prop]() {
        return (prop == EProperty::InitDuty || prop == EProperty::DutyBase
            || prop == EProperty::DutyStep || prop == EProperty::Depth);
    };

    if (adsrChanged())
        channelState.updateADSR(adsr);

    if (pwmChanged())
        channelState.updatePWMData(pwmData);
}


void ControlsTab::buttonClicked(juce::Button* button)
{
    if (button == m_changeReverbForAll.get())
    {
        if (m_changeReverbForAll->getToggleState())
        {
            auto selectedReverb = m_comboReverb->getSelectedId();
            m_audioProcessor.applyReverbToAllChannels(static_cast<EReverbType>(selectedReverb - 1));
        }
    }
}

void ControlsTab::reverbComboChanged()
{
    auto selectedReverb = m_comboReverb->getSelectedId();
    auto reverbType = static_cast<EReverbType>(selectedReverb - 1);
 
    if (m_changeReverbForAll->getToggleState())
    {
        m_audioProcessor.applyReverbToAllChannels(reverbType);
    }
    else // Apply to curent channel
    {
        m_audioProcessor.GetChannelState(m_currentMidiChannel).setReverbType(reverbType);
    }

    m_reverbSliderEnabled = (reverbType == EReverbType::Default);
    m_sliderReverb->setVisible(m_reverbSliderEnabled);
    repaint();
}

int ControlsTab::getSelectedMidiChannel() const
{
    return m_comboChannel->getSelectedId();
}

void ControlsTab::refresh()
{
    auto selectedComboChannel = m_comboChannel->getSelectedId();
    m_currentMidiChannel = selectedComboChannel - 1;

    const auto& channelState = m_audioProcessor.GetChannelState(m_currentMidiChannel);

    auto currentChannelPreset = channelState.getCurrentPreset();
    m_comboPreset->setSelectedId(currentChannelPreset);

    m_sliderVolume->setValue(channelState.getVolume());
    m_sliderPan->setValue(channelState.getPan());
    m_sliderReverb->setValue(channelState.getReverbLevel());

    const auto& adsr = channelState.getADSR();
    m_sliderAtt->setValue(adsr.att);
    m_sliderDec->setValue(adsr.dec);
    m_sliderSus->setValue(adsr.sus);
    m_sliderRel->setValue(adsr.rel);

    auto bIsPWM = (channelState.getType() == EDSPType::ModPulse);

    m_sliderInitDuty->setVisible(bIsPWM);
    m_sliderDutyBase->setVisible(bIsPWM);
    m_sliderDutyStep->setVisible(bIsPWM);
    m_sliderDepth->setVisible(bIsPWM);

    if (bIsPWM)
    {
        const auto& pwmData = channelState.getPWMData();
        m_sliderInitDuty->setValue(pwmData.initDuty);
        m_sliderDutyBase->setValue(pwmData.dutyBase);
        m_sliderDutyStep->setValue(pwmData.dutyStep);
        m_sliderDepth->setValue(pwmData.depth);
    }

    auto reverbType = channelState.getReverbType();
    m_comboReverb->setSelectedId((int)reverbType + 1, juce::dontSendNotification);

    m_reverbSliderEnabled = (reverbType == EReverbType::Default);
    m_sliderReverb->setVisible(m_reverbSliderEnabled);

    auto detectedBPM = static_cast<int>(std::round(m_audioProcessor.getDetectedBPM()));

    if (bIsPWM != m_isPWMSynth)
    {
        m_isPWMSynth = bIsPWM;
        repaint();
    }

    if (detectedBPM != m_detectedBPM)
    {
        m_detectedBPM = detectedBPM;
        repaint();
    }
}

void ControlsTab::refreshPresets()
{
    m_comboPreset->refresh(m_audioProcessor.getPresets());
}

}
