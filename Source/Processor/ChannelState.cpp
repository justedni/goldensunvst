#include "ChannelState.h"

#include "ReverbEffect.h"
#include "GS/GSReverb.h"
#include "Instrument.h"

#include "Presets/PresetsHandler.h"
#include "Presets/Presets.h"

namespace GSVST {

void ChannelState::init(double in_sampleRate, int in_samplesPerBlock, int in_samplesPerBlockComputation)
{
    if (in_sampleRate != m_sampleRate
        || in_samplesPerBlock != m_samplesPerBlock
        || in_samplesPerBlockComputation != m_samplesPerBlockComputation)
    {
        m_sampleRate = in_sampleRate;
        m_samplesPerBlock = in_samplesPerBlock;
        m_samplesPerBlockComputation = in_samplesPerBlockComputation;

        allocateReverb();
    }
}

bool ChannelState::isActive() const
{
    return !m_playingInstruments.empty();
}

void ChannelState::cleanup()
{
    for (auto* instr : m_playingInstruments)
        delete instr;

    m_playingInstruments.clear();
}

void ChannelState::process(size_t numSamples, const MixingArgs& margs)
{
    if (isActive())
    {
        zeroBuffers(outputBuffers);

        for (auto* instr : m_playingInstruments)
        {
            instr->processCommon(outputBuffers.data(), numSamples, margs);
        }
    }
}

void ChannelState::processReverb(size_t numSamples, size_t samplesPerBufferForComputation, juce::AudioBuffer<float>& buffer)
{
    if (isActive())
    {
        if (revdsp)
            revdsp->ProcessData(outputBuffers.data(), numSamples, samplesPerBufferForComputation);

        for (auto iSample = 0; iSample < numSamples; iSample++)
        {
            auto& sample = outputBuffers[iSample];
            buffer.addSample(0, iSample, sample.left);
            buffer.addSample(1, iSample, sample.right);
        }
    }
}

void ChannelState::killAllPlayingInstruments()
{
    if (isActive())
    {
        for (auto* soundChannel : m_playingInstruments)
            soundChannel->kill();
    }
}

void ChannelState::cleanupDeadInstruments()
{
    auto it = m_playingInstruments.begin();
    while (it != m_playingInstruments.end())
    {
        auto* instr = (*it);
        if (instr->isDead())
        {
            delete instr;
            it = m_playingInstruments.erase(it);
        }
        else
            it++;
    }
}

void ChannelState::allocateIfNecessary(int numSamples)
{
    if (!isActive())
    {
        zeroBuffers(outputBuffers);
        allocateBuffersIfNecessary(outputBuffers, numSamples);
    }

}

void ChannelState::allocateBuffersIfNecessary(std::vector<sample>& io_buffers, int numSamples)
{
    if (io_buffers.size() != numSamples)
    {
        io_buffers.clear();
        for (int i = 0; i < numSamples; i++)
        {
            io_buffers.push_back(sample());
        }
    }
}

void ChannelState::zeroBuffers(std::vector<sample>& io_buffers)
{
    for (auto& sample : io_buffers)
    {
        sample.left = 0.0f;
        sample.right = 0.0f;
    }
}

void ChannelState::setBPM(int in_bpm)
{
    m_detectedBPM = in_bpm;

    for (auto* soundChannel : m_playingInstruments)
    {
        soundChannel->setBPM(in_bpm);
    }
}

void ChannelState::setReverbType(EReverbType type)
{
    if (reverbType == type)
        return;

    if (reverbType != type)
    {
        reverbType = type;
        allocateReverb();
    }
}

void ChannelState::allocateReverb()
{
    const uint8_t reverbIntensity = 79;
    const auto revBufSize = 0x630;
    const auto maxFixedModeRate = 31536;

    revdsp.reset();

    switch (reverbType)
    {
    case EReverbType::None:
        break;
    case EReverbType::Default:
        revdsp = std::make_unique<ReverbEffect>(
            reverbIntensity, m_samplesPerBlockComputation, uint8_t(revBufSize / (maxFixedModeRate / AGB_FPS)));
        break;
    case EReverbType::GS1:
        revdsp = std::make_unique<ReverbGS1>(
            reverbIntensity, m_samplesPerBlockComputation, uint8_t(revBufSize / (maxFixedModeRate / AGB_FPS)));
        break;
    case EReverbType::GS2:
        revdsp = std::make_unique<ReverbGS2>(
            reverbIntensity, m_samplesPerBlockComputation, uint8_t(revBufSize / (maxFixedModeRate / AGB_FPS)),
            0.4140625f, -0.0625f);
        break;
    }
}

void ChannelState::setReverbLevel(int val)
{
    reverbLevel = val;

    if (revdsp)
        revdsp->SetIntensity(val);
}

void ChannelState::setVolume(int val)
{
    volume = static_cast<uint8_t>(std::round(val * val / 127.0f));

    for (auto* soundChannel : m_playingInstruments)
    {
        soundChannel->setVol(volume);
    }
}

int ChannelState::getPan() const
{
    auto val = ((pan * 2) >> 1) + 0x40;
    return val;
}

void ChannelState::setPan(int val)
{
    int8_t convertedVal = static_cast<int8_t>(std::clamp(val, 0, 127) - 0x40);
    convertedVal = (convertedVal << 1) / 2;

    pan = convertedVal;

    for (auto& soundChannel : m_playingInstruments)
    {
        soundChannel->setPan(convertedVal);
    }
}

void ChannelState::setPreset(int presetId, const PresetsHandler& presets)
{
    for (auto* preset : presets.m_presets)
    {
        if (preset->programid == presetId)
        {
            m_preset = preset;
            m_envelope = ADSR(preset->getADSR());
            m_type = m_preset->getDSPType();
            m_preset->getPWMData(m_pwmData);
            break;
        }
    }
}

void ChannelState::resetPreset()
{
    m_preset = nullptr;
}

int ChannelState::getCurrentPreset() const
{
    return m_preset ? m_preset->programid : -1;
}

void ChannelState::updateADSR(const ADSR& in_adsr)
{
    m_envelope.att = in_adsr.att;
    m_envelope.dec = in_adsr.dec;
    m_envelope.sus = in_adsr.sus;
    m_envelope.rel = in_adsr.rel;

    for (auto* soundChannel : m_playingInstruments)
    {
        soundChannel->updateADSR(in_adsr);
    }
}

void ChannelState::updatePWMData(const PWMData& in_data)
{
    m_pwmData = PWMData(in_data);

    for (auto* soundChannel : m_playingInstruments)
    {
        soundChannel->updatePWMData(in_data);
    }
}

Instrument* ChannelState::handleNoteOn(uint8_t noteNumber, int8_t velocity, int bpm)
{
    if (!m_preset)
        return nullptr;

    Note note;
    note.midiKeyTrackData = noteNumber;
    note.midiKeyPitch = noteNumber;
    note.velocity = velocity;

    Instrument* newInstance = m_preset->createPlayingInstance(note);

    if (newInstance)
    {
        newInstance->setBPM(bpm);

        if (newInstance->shouldUseTrackADSR())
            newInstance->updateADSR(m_envelope);

        if (pitchBendRange.hasValue())
            newInstance->setPitchBendRange(pitchBendRange.getValue());

        newInstance->setPitchWheel(pitchWheel);

        if (detune.hasValue())
            newInstance->setTune(detune.getValue());

        if (lfoSpeed != 0 && modWheel != 0)
            newInstance->setLfo(lfoSpeed, modWheel);


        newInstance->setVol(volume);
        newInstance->setPan(pan);

        newInstance->updatePWMData(m_pwmData);

        m_playingInstruments.push_back(newInstance);
    }

    return newInstance;
}

void ChannelState::handleNoteOff(int noteNumber)
{
    for (auto& soundChannel : m_playingInstruments)
    {
        if (soundChannel->getMidiNote() == noteNumber)
        {
            soundChannel->release();
        }
    }
}

void ChannelState::allNotesOff()
{
    for (auto* soundChannel : m_playingInstruments)
    {
        soundChannel->release();
    }
}

bool ChannelState::handleMidiMsg(const juce::MidiMessage& msg, const PresetsHandler& presets, bool bIgnorePrgChg)
{
    bool bRefreshRequired = false;

    if (msg.isProgramChange() && !bIgnorePrgChg)
    {
        auto prgChg = msg.getProgramChangeNumber();
        setPreset(prgChg, presets);
        bRefreshRequired = true;
    }
    else if (msg.isPitchWheel())
    {
        auto val = msg.getPitchWheelValue();
        val = (val >> 7) - 0x40;
        pitchWheel = static_cast<int16_t>(val);

        for (auto& soundChannel : m_playingInstruments)
        {
            soundChannel->setPitchWheel(pitchWheel);
        }
    }
    else if (msg.isControllerOfType(1)) // Mod Wheel
    {
        modWheel = static_cast<uint8_t>(msg.getControllerValue() / 10);

        if (lfoSpeed != 0 && modWheel != 0)
        {
            for (auto& soundChannel : m_playingInstruments)
            {
                soundChannel->setLfo(lfoSpeed, modWheel);
            }
        }
    }
    else if (msg.isControllerOfType(7)) // Volume MSB
    {
        setVolume(msg.getControllerValue());
        bRefreshRequired = true;
    }
    else if (msg.isControllerOfType(10)) // Pan Position MSB
    {
        setPan(msg.getControllerValue());
        bRefreshRequired = true;
    }
    else if (msg.isControllerOfType(91)) // Effect level
    {
        setReverbLevel(msg.getControllerValue());
        bRefreshRequired = true;
    }
    else if (msg.isControllerOfType(101)) // RPN MSB
    {
        resetAllRPNs();

        auto msb = msg.getControllerValue();
        if (msb == 0)
        {
            detune.validMsb = true;
            pitchBendRange.validMsb = true;
        }
    }
    else if (msg.isControllerOfType(100)) // RPN LSB
    {
        auto lsb = msg.getControllerValue();
        if (lsb == 0)
        {
            pitchBendRange.validLsb = true;
            detune.validLsb = false;
        }
        if (lsb == 1)
        {
            detune.validLsb = true;
            pitchBendRange.validLsb = false;

        }
    }
    else if (msg.isControllerOfType(6)) // Data entry
    {
        if (detune.isValid())
        {
            int16_t val = static_cast<int16_t>(msg.getControllerValue() - 0x40);
            detune.setValue(val);
            for (auto& soundChannel : m_playingInstruments)
            {
                soundChannel->setTune(val);
            }
        }
        else if (pitchBendRange.isValid())
        {
            int16_t val = static_cast<int16_t>(msg.getControllerValue());
            pitchBendRange.setValue(val);
            for (auto& soundChannel : m_playingInstruments)
            {
                soundChannel->setPitchBendRange(val);
            }
        }
    }

    return bRefreshRequired;
}

float ChannelState::getAverageLevel() const
{
    if (outputBuffers.size() > 0)
    {
        auto result = std::accumulate(outputBuffers.begin(), outputBuffers.end(), 0.0f,
            [](float total, sample elem)
            {
                return total + elem.left * elem.left;
            });

        return std::sqrt(result / static_cast<float>(outputBuffers.size()));
    }

    return 0.0f;
}

}