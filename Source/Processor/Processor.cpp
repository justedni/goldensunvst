#include "Processor.h"
#include "GUI/MainWindow.h"

#include "ReverbEffect.h"
#include "Instrument.h"

#include "GS/GSPresets.h"

#include <assert.h>

namespace GSVST {

Processor::Processor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : juce::AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
    #if ! JucePlugin_IsSynth
    .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
    #endif
    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
)
#endif
{
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
    {
        m_channels[i] = new ChannelState();
    }

    m_presets.reset(new GSPresets());
}

Processor::~Processor()
{
    m_presets->cleanupSoundfont();

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
    {
        delete m_channels[i];
        m_channels[i] = nullptr;
    }
}

void Processor::applyReverbToAllChannels(EReverbType type)
{
    ForEachMidiChannel([&](auto& state)
    {
        state.setReverbType(type);
    });
}

//==============================================================================
const juce::String Processor::getName() const
{
    return JucePlugin_Name;
}

bool Processor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Processor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Processor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Processor::getTailLengthSeconds() const
{
    return 0.0;
}

int Processor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Processor::getCurrentProgram()
{
    return 0;
}

void Processor::setCurrentProgram (int /*index*/)
{
}

const juce::String Processor::getProgramName (int  /*index*/)
{
    return {};
}

void Processor::changeProgramName (int  /*index*/, const juce::String& /*newName*/)
{
}

int Processor::getNumSamplesForComputation(double sampleRate)
{
    return static_cast<int>(std::round(sampleRate / (AGB_FPS * INTERFRAMES)));
}

//==============================================================================
void Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    ForEachMidiChannel([&](auto& state)
    {
        state.init(sampleRate, samplesPerBlock, getNumSamplesForComputation(sampleRate));
    });
}

void Processor::releaseResources()
{
    ForEachMidiChannel([](auto& state)
    {
        state.cleanup();
    });
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

int getChannelId(const juce::MidiMessage& msg)
{
    return msg.getChannel() - 1;
}

void Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    const auto& numSamples = buffer.getNumSamples();

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    bool bIsPlaying = true;

    auto positionInfo = getPlayHead()->getPosition();
    auto timeInSec = positionInfo->getTimeInSeconds();

    {
       
        bIsPlaying = positionInfo->getIsPlaying();
        
        if (auto bpm = positionInfo->getBpm(); bpm.hasValue())
        {
            auto newVal = *bpm;
            if (newVal != detectedBPM)
            {
                detectedBPM = static_cast<int>(std::round(*bpm));
                bRefreshUIRequired = true;
            }
        }

        if (timeInSec.hasValue())
        {
            if (std::abs(*timeInSec - currentTime) > 0.2)
            {
                // Jump detected: reset all RPNs to avoid weird bugs
                ForEachMidiChannel([&](auto& state)
                {
                    state.resetAllRPNs();
                });
            }

            currentTime = *timeInSec;
        }

        if (bRefreshUIRequired)
        {
            ForEachMidiChannel([&](auto& state)
            {
                state.setBPM(detectedBPM);
            });
        }
    }

    struct PendingNoteOn
    {
        PendingNoteOn(double in_time, uint8_t in_note, int in_chan, unsigned char in_vel)
            : timestamp(in_time)
            , noteNumber(in_note)
            , channel(in_chan)
            , velocity(in_vel)
        {}

        double timestamp;
        uint8_t noteNumber;
        int channel;
        unsigned char velocity;
    };

    std::vector<PendingNoteOn> pendingNotesOn;

    for (const auto& msgRaw : midiMessages)
    {
        auto msg = msgRaw.getMessage();
        auto& state = GetChannelState(getChannelId(msg));

        if (msg.isNoteOn())
        {
            pendingNotesOn.emplace_back(msg.getTimeStamp(), (uint8_t)msg.getNoteNumber(), getChannelId(msg), msg.getVelocity());
        }
        else if (msg.isAllNotesOff())
        {
            ForEachMidiChannel([](auto& state)
            {
                state.allNotesOff();
            });
        }
        else if (msg.isNoteOff())
        {
            // Ignore for now
        }
        else
        {
            bRefreshUIRequired |= state.handleMidiMsg(msg, *m_presets.get(), bIgnoreProgramChange, bIsPlaying);
        }
    }

    MixingArgs margs;
    margs.vol = 1.0f;
    margs.sampleRateInv = 1.0f / static_cast<float>(getSampleRate());

    margs.samplesPerBufferForComputation = getNumSamplesForComputation(getSampleRate());
    margs.samplesPerBufferInv = 1.0f / static_cast<float>(margs.samplesPerBufferForComputation);

    ForEachMidiChannel([&](auto& state)
    {
        state.process(numSamples, margs);
    });

    if (!pendingNotesOn.empty())
    {
        for (auto& noteOn : pendingNotesOn)
        {
            auto& state = GetChannelState(noteOn.channel);
            state.allocateIfNecessary(numSamples);

            if (auto* newChan = state.handleNoteOn(noteOn.noteNumber, noteOn.velocity, detectedBPM))
            {
                auto offset = (int)std::round(noteOn.timestamp);
                newChan->processCommon(state.getOutBuffer().data() + offset, numSamples - offset, margs);
            }
        }
        pendingNotesOn.clear();
    }

    ForEachMidiChannel([&](auto& state)
    {
        state.processReverb(numSamples, margs.samplesPerBufferForComputation, buffer);
    });

    // Processing "note off" messages after buffer process
    for (auto msgRaw : midiMessages)
    {
        auto msg = msgRaw.getMessage();
        auto& state = GetChannelState(getChannelId(msg));
        if (msg.isNoteOff())
        {
            auto noteNumber = msg.getNoteNumber();
            state.handleNoteOff(noteNumber);
        }
    }

    ForEachMidiChannel([](auto& state)
    {
        state.cleanupDeadInstruments();
    });
}

//==============================================================================
bool Processor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Processor::createEditor()
{
    return new MainWindow (*this);
}

bool Processor::dataRefreshRequired()
{
    if (bRefreshUIRequired)
    {
        bRefreshUIRequired = false;
        return true;
    }

    return false;
}

bool Processor::presetsRefreshRequired()
{
    if (bRefreshPresetsRequired)
    {
        bRefreshPresetsRequired = false;
        return true;
    }

    return false;
}

void Processor::setPresetsRefresh()
{
    bRefreshPresetsRequired = true;
}

void Processor::setSoundfont(const std::string& path)
{
    // Make sure nothing is already playing
    ForEachMidiChannel([&](auto& state)
    {
        state.killAllPlayingInstruments();
        state.resetPreset();
    });

    m_presets->setSoundfont(path);
    bRefreshPresetsRequired = true;
}

void  Processor::setGSMode(bool bEnable)
{
    if (!bEnable)
    {
        ForEachMidiChannel([&](auto& state)
        {
            state.killAllPlayingInstruments();
            state.resetPreset();
        });
    }

    m_presets->setEnableGSMode(bEnable);
}

//==============================================================================
void Processor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement root("plugin_data");
    root.setAttribute("version", 0.1);
    root.setAttribute("gsmode", m_presets->m_bGSModeEnabled);
    root.setAttribute("soundfont", m_presets->soundFontPath);

    copyXmlToBinary(root, destData);
}

void Processor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (!xmlState || !xmlState->hasAttribute("version"))
        return;

    auto version = xmlState->getDoubleAttribute("version");
    if (version < 0.1)
        return;

    if (xmlState->hasAttribute("gsmode"))
        m_presets->setEnableGSMode(xmlState->getBoolAttribute("gsmode"));

    auto path = std::string(xmlState->getStringAttribute("soundfont").getCharPointer());
    setSoundfont(path);

    bRefreshPresetsRequired = true;
}

} // namespace GSVST

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GSVST::Processor();
}