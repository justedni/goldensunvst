#pragma once

#include "Types.h"

#include <JuceHeader.h>

namespace GSVST {

class ReverbEffect;
class Instrument;

class Preset;
struct PresetsHandler;
enum class EDSPType : uint8_t;

struct ChannelState
{
public:
    ChannelState() {}
    ~ChannelState()
    {
        outputBuffers.clear();
    }

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

    void setPreset(int presetId, const PresetsHandler& presets);
    void resetPreset();
    int getCurrentPreset() const;

    void updateADSR(const ADSR& in_adsr);
    const ADSR& getADSR() const { return m_envelope; }

    void updatePWMData(const PWMData& in_data);
    const PWMData& getPWMData() const { return m_pwmData; }

    Instrument* handleNoteOn(uint8_t noteNumber, int8_t velocity, int bpm);
    void handleNoteOff(int noteNumber);
    bool handleMidiMsg(const juce::MidiMessage& msg, const PresetsHandler& presets, bool bIgnorePrgChg);
    void allNotesOff();

    std::vector<sample>& getOutBuffer() { return outputBuffers; }

    float getAverageLevel() const;

private:
    static void allocateBuffersIfNecessary(std::vector<sample>& io_buffers, int numSamples);
    static void zeroBuffers(std::vector<sample>& io_buffers);

    void allocateReverb();

    double m_sampleRate = 0.0;
    int m_samplesPerBlock = 0;
    int m_samplesPerBlockComputation = 0;

    uint8_t volume = 77;
    uint8_t pan = 64;
    int16_t pitchWheel = 0;
    uint8_t modWheel = 0;
    const uint8_t lfoSpeed = 40; // 0x28

    int m_detectedBPM = -1;

    Preset* m_preset = nullptr;

    ADSR m_envelope;
    PWMData m_pwmData;

    EReverbType reverbType = EReverbType::None;
    int reverbLevel = 0;

    EDSPType m_type;
    std::list<Instrument*> m_playingInstruments;
    std::vector<sample> outputBuffers;

    std::unique_ptr<ReverbEffect> revdsp;

    struct RPN
    {
        bool validMsb = false;
        bool validLsb = false;

        bool hasValue() const { return bValueSet; }
        int16_t getValue() const { return value; }
        void setValue(int16_t in_val) { value = in_val; }

        bool isValid() const { return (validMsb && validLsb); }

        void reset()
        {
            validMsb = false;
            validLsb = false;
        }

    private:
        int16_t value = 0;
        bool bValueSet = false;
    };

    RPN detune;
    RPN pitchBendRange;

    void resetAllRPNs()
    {
        detune.reset();
        pitchBendRange.reset();
    }
};

}