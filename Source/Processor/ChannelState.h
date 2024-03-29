#pragma once

#include "Types.h"

#include <JuceHeader.h>

namespace GSVST {

class ReverbEffect;
class Instrument;

class Preset;
struct PresetsHandler;
struct RPNHandler;
enum class EDSPType : uint8_t;

struct ChannelState
{
public:
    ChannelState();
    ~ChannelState();

    void init(double in_sampleRate, int in_samplesPerBlock, int in_samplesPerBlockComputation);
    void cleanup();

    void process(size_t numSamples, const MixingArgs& args);
    void processReverb(size_t numSamples, size_t samplesPerBufferForComputation, juce::AudioBuffer<float>& buffer);

    void killAllPlayingInstruments();
    void cleanupDeadInstruments();

    void allocateIfNecessary(int numSamples);

    bool isActive() const;

    EDSPType getType() const { return m_type; }

    void setVolume(int val);
    int getVolume() const { return volume; }

    void setPan(int val);
    int getPan() const;

    void setBPM(int in_bpm);

    void setReverbType(EReverbType type);
    EReverbType getReverbType() const { return reverbType; }
    void setReverbLevel(int val);
    int getReverbLevel() const { return reverbLevel; }

    void setPreset(int bankId, int programId, const PresetsHandler& presets);
    void resetPreset();
    std::pair<int, int> getCurrentPreset() const;

    void updateADSR(const ADSR& in_adsr);
    const ADSR& getADSR() const { return m_envelope; }

    void updatePWMData(const PWMData& in_data);
    const PWMData& getPWMData() const { return m_pwmData; }

    Instrument* handleNoteOn(uint8_t noteNumber, int8_t velocity, int bpm);
    void handleNoteOff(int noteNumber);
    bool handleMidiMsg(const juce::MidiMessage& msg, const PresetsHandler& presets, bool bIgnorePrgChg, bool bIsPlaying);
    void allNotesOff();

    std::vector<sample>& getOutBuffer() { return outputBuffers; }

    float getAverageLevel() const;

    int16_t getDetune() const;
    int16_t getPitchBendRange() const;
    int16_t getLfoSpeed() const;
    void resetAllRPNs();

private:
    static void allocateBuffersIfNecessary(std::vector<sample>& io_buffers, int numSamples);
    static void zeroBuffers(std::vector<sample>& io_buffers);

    void allocateReverb();

    double m_sampleRate = 0.0;
    int m_samplesPerBlock = 0;
    int m_samplesPerBlockComputation = 0;

    uint8_t volume = 77;
    int8_t pan = 0;
    int16_t pitchWheel = 0;
    uint8_t modWheel = 0;

    int m_detectedBPM = -1;

    int m_currentBankId = 0;
    const Preset* m_preset = nullptr;

    ADSR m_envelope;
    PWMData m_pwmData;

    EReverbType reverbType = EReverbType::None;
    int reverbLevel = 0;

    EDSPType m_type;
    std::list<Instrument*> m_playingInstruments;
    std::vector<sample> outputBuffers;

    std::unique_ptr<ReverbEffect> revdsp;

    std::unique_ptr<RPNHandler> m_rpnHanlder;
};

}