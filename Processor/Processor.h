#pragma once

#include <JuceHeader.h>
#include "Types.h"
#include "ChannelState.h"

struct PresetsHandler;

namespace GSVST {

class Processor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    Processor();
    ~Processor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    const auto& getPresets() const { return *(m_presets.get()); }
    auto& getPresets() { return *(m_presets.get()); }

    double getDetectedBPM() const { return detectedBPM; }

    ChannelState& GetChannelState(int midiChannel) { return *(m_channels[midiChannel]); }

    void applyReverbToAllChannels(EReverbType type);

    bool dataRefreshRequired();
    bool presetsRefreshRequired();
    void setPresetsRefresh();

    void setSoundfont(const std::string& path);
    void setGSMode(bool bEnable);

    void setIgnoreProgramChange(bool in_ignore) { bIgnoreProgramChange = in_ignore; }

private:
    int getNumSamplesForComputation(double sampleRate);

    template<typename T>
    void ForEachMidiChannel(T func)
    {
        for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
        {
            func(*m_channels[i]);
        }
    }

    int detectedBPM = 120;
    ChannelState* m_channels[MAX_MIDI_CHANNELS];

    std::unique_ptr<PresetsHandler> m_presets;

    volatile bool bRefreshUIRequired = false;
    volatile bool bRefreshPresetsRequired = false;
    
    bool bIgnoreProgramChange = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

}