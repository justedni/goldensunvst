#include "ChannelState.h"

#include "ReverbEffect.h"
#include "GS/GSReverb.h"
#include "Instrument.h"

#include "Presets/PresetsHandler.h"
#include "Presets/Presets.h"
#include "RPNHandler.h"


namespace GSVST {

ChannelState::ChannelState()
{
    m_rpnHanlder.reset(new RPNHandler());
}

ChannelState::~ChannelState()
{
    outputBuffers.clear();
}

void ChannelState::init(double in_sampleRate, int in_samplesPerBlock, int in_samplesPerBlockComputation)
{
    if (in_sampleRate != m_sampleRate
        || in_samplesPerBlock != m_samplesPerBlock
        || in_samplesPerBlockComputation != m_samplesPerBlockComputation)
    {
        m_sampleRate = in_sampleRate;
        m_samplesPerBlock = in_samplesPerBlock;
        m_samplesPerBlockComputation = in_samplesPerBlockComputation;

        outputBuffers.resize(in_samplesPerBlock);

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
    case EReverbType::MGAT:
        revdsp = std::make_unique<ReverbGS2>(
            reverbIntensity, m_samplesPerBlockComputation, uint8_t(revBufSize / (maxFixedModeRate / AGB_FPS)),
            0.25f, -0.046875f);
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

void ChannelState::setPreset(int bankId, int programId, const PresetsHandler& presets)
{
    for (auto* preset : presets.m_presets)
    {
        if (preset->bankid == bankId && preset->programid == programId)
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

std::pair<int, int> ChannelState::getCurrentPreset() const
{
    return m_preset ? std::make_pair(m_preset->bankid, m_preset->programid) : std::make_pair(0, -1);
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

        if (m_rpnHanlder->hasValue(RPN::Param::PitchBendRange))
            newInstance->setPitchBendRange(m_rpnHanlder->getValue(RPN::Param::PitchBendRange));

        newInstance->setPitchWheel(pitchWheel);

        if (m_rpnHanlder->hasValue(RPN::Param::Detune))
            newInstance->setTune(m_rpnHanlder->getValue(RPN::Param::Detune));

        if (m_rpnHanlder->hasValue(RPN::Param::LfoSpeed))
        {
            auto lfoSpeedVal = m_rpnHanlder->getValue(RPN::Param::LfoSpeed);
            if (lfoSpeedVal != 0 && modWheel != 0)
                newInstance->setLfo(static_cast<uint8_t>(lfoSpeedVal), modWheel);
        }


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
        if (soundChannel->getMidiNote() == noteNumber && !soundChannel->isStopping())
        {
            soundChannel->release();
            break;
        }
    }
}

void ChannelState::allNotesOff()
{
    for (auto* soundChannel : m_playingInstruments)
    {
        soundChannel->release();
    }

    m_rpnHanlder->resetAllRPNs(); // just to be safe
}

bool ChannelState::handleMidiMsg(const juce::MidiMessage& msg, const PresetsHandler& presets, bool bIgnorePrgChg, bool bIsPlaying)
{
    bool bRefreshRequired = false;

    if (msg.isControllerOfType(0)) // Bank change
    {
        m_currentBankId = msg.getControllerValue();
    }
    else if (msg.isProgramChange() && !bIgnorePrgChg)
    {
        auto programId = msg.getProgramChangeNumber();
        setPreset(m_currentBankId, programId, presets);
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

        auto lfoSpeedVal = m_rpnHanlder->hasValue(RPN::Param::LfoSpeed) ? m_rpnHanlder->getValue(RPN::Param::LfoSpeed) : 0;
        if (lfoSpeedVal != 0 && modWheel != 0)
        {
            for (auto& soundChannel : m_playingInstruments)
            {
                soundChannel->setLfo(static_cast<uint8_t>(lfoSpeedVal), modWheel);
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
    else if (msg.isController()) // All other controllers
    {
        auto ccId = msg.getControllerNumber();
        auto val = msg.getControllerValue();
        auto timestamp = msg.getTimeStamp();
        switch (ccId)
        {
        case 101: // RPN MSB
            m_rpnHanlder->setRPN_MSB(RPN::Type::RPN, val, timestamp);
            break;
        case 100: // RPN LSB
            m_rpnHanlder->setRPN_LSB(RPN::Type::RPN, val, timestamp);
            break;
        case 99: // NRPN MSB
            m_rpnHanlder->setRPN_MSB(RPN::Type::NRPN, val, timestamp);
            break;
        case 98: // NRPN LSB
            m_rpnHanlder->setRPN_LSB(RPN::Type::NRPN, val, timestamp);
            break;
        case 6: // Data entry
        {
            if (bIsPlaying)
            {
                auto param = m_rpnHanlder->setDataEntry(val, timestamp);

                if (param != RPN::Param::None)
                {
                    auto convertedVal = m_rpnHanlder->getValue(param);
                    for (auto& soundChannel : m_playingInstruments)
                    {
                        switch (param)
                        {
                        case RPN::Param::Detune:
                            soundChannel->setTune(static_cast<int16_t>(convertedVal));
                            break;
                        case RPN::Param::PitchBendRange:
                            soundChannel->setPitchBendRange(static_cast<int16_t>(convertedVal));
                            break;
                        case RPN::Param::LfoSpeed:
                            soundChannel->setLfo(static_cast<uint8_t>(convertedVal), modWheel);
                            break;
                        }
                    }

                    bRefreshRequired = true;
                }
            }
            break;
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

int16_t ChannelState::getDetune() const
{
    return m_rpnHanlder->getValue(RPN::Param::Detune);
}

int16_t ChannelState::getPitchBendRange() const
{
    return m_rpnHanlder->getValue(RPN::Param::PitchBendRange);
}

int16_t ChannelState::getLfoSpeed() const
{
    return m_rpnHanlder->getValue(RPN::Param::LfoSpeed);
}

void ChannelState::resetAllRPNs()
{
    m_rpnHanlder->resetAllRPNs();
}

}