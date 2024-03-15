#include "OverviewTab.h"
#include "CustomLookAndFeel.h"
#include "PresetCombo.h"
#include "LevelMeter.h"

#include "Processor/Processor.h"
#include "Processor/ReverbEffect.h"
#include "Presets/PresetsHandler.h"
#include "Presets/Presets.h"

#define DISPLAYED_MIDI_CHANNELS 10

namespace GSVST {

GlobalViewTab::GlobalViewTab(Processor& p)
    : m_audioProcessor(p)
{
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
    {
        auto& combo = m_channelDescs[i].m_presetCombo;
        combo.reset(new PresetCombo());
        addAndMakeVisible(*combo.get());
        combo->refresh(p.getPresets());
        combo->onChange = [this, id=i] { presetComboChanged(id); };

        auto [bankId, programId] = p.GetChannelState(i).getCurrentPreset();
        combo->setSelectedProgram(bankId, programId);
    }

    for (int i = 0; i < DISPLAYED_MIDI_CHANNELS; i++)
    {
        m_channelDescs[i].m_meter.reset(new LevelMeter());
        addAndMakeVisible(*m_channelDescs[i].m_meter.get());
    }
   
    startTimerHz(20);
}

void GlobalViewTab::timerCallback()
{
    // Only showing 10 channels for now (TODO: add <-> selector)
    for (int i = 0; i < DISPLAYED_MIDI_CHANNELS; i++)
    {
        auto& channelState = m_audioProcessor.GetChannelState(i);
        float level = 0.0f;
        if (channelState.isActive())
            level = channelState.getAverageLevel();

        m_channelDescs[i].m_meter->setLevel(level);
        m_channelDescs[i].m_meter->repaint();
    }
}

void GlobalViewTab::paint(juce::Graphics& g)
{
    CustomLookAndFeel::drawGSBox(g, 0, 0, getWidth(), getHeight());

    auto currentY = 10;

    for (int i = 0; i < DISPLAYED_MIDI_CHANNELS; i++)
    {
        g.setFont(12);
        g.setColour(juce::Colours::white);
        g.drawText(std::to_string(i+1), 6, currentY, 20, 20, juce::Justification::centred);

        g.setFont(10);
        g.setColour(m_channelDescs[i].m_deviceColour);
        g.drawText(m_channelDescs[i].m_deviceName, 240, currentY, 60, 20, juce::Justification::centredLeft);
        currentY += 22;
    }

    //getLookAndFeel().drawLevelMeter(g, 250, 100, (float)std::exp(std::log(level) / 3.0));
}

void GlobalViewTab::resized()
{
    auto currentY = 10;

    for (int i = 0; i < DISPLAYED_MIDI_CHANNELS; i++)
    {
        m_channelDescs[i].m_presetCombo->setBounds(30, currentY, 200, 20);
        m_channelDescs[i].m_meter->setBounds(300, currentY, 100, 20);
        currentY += 22;
    }
}

void GlobalViewTab::presetComboChanged(int channel)
{
    auto [bankId, programId] = m_channelDescs[channel].m_presetCombo->getSelectedProgramId();
    auto& channelState = m_audioProcessor.GetChannelState(channel);
    channelState.setPreset(bankId, programId, m_audioProcessor.getPresets());

    refresh();
}

void GlobalViewTab::refresh()
{
    const auto& presetsHandler = m_audioProcessor.getPresets();
    const auto& programInfo = presetsHandler.getProgramInfo();

    for (int i = 0; i < DISPLAYED_MIDI_CHANNELS; i++)
    {
        auto& desc = m_channelDescs[i];
        auto [bankId, programId] = m_audioProcessor.GetChannelState(i).getCurrentPreset();
        desc.m_presetCombo->setSelectedProgram(bankId, programId);

        if (auto* info = presetsHandler.findProgramInfo(programInfo, bankId, programId))
        {
            desc.m_deviceName = info->device;
            auto colour = getDeviceColour(info->device);
            desc.m_deviceColour = colour;
            desc.m_meter->setColour(colour);
        }
    }

    repaint();
}

juce::Colour GlobalViewTab::getDeviceColour(const std::string& name)
{
    if (name == "SC-88")
        return juce::Colour(230, 127, 22);
    else if (name == "Synth")
        return juce::Colour(202, 0, 0);
    else if (name == "JV-1080")
        return juce::Colour(113, 196, 229);

    return juce::Colour(66, 162, 200);
}

void GlobalViewTab::refreshPresets()
{
    const auto& presets = m_audioProcessor.getPresets();
    
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
    {
        m_channelDescs[i].m_presetCombo->refresh(presets);
    }
}

}
