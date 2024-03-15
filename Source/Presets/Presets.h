#pragma once

#include "Processor/SampleInstrument.h"

#include <JuceHeader.h>

template <class T>
class AudioFile;

namespace juce
{
    template <typename Type>
    class AudioBuffer;
}

struct tsf;

namespace GSVST {

class Instrument;
struct PresetsHandler;

enum class EPresetType : uint8_t
{
    Sample,
    Soundfont,
    Synth
};

std::string EnumToString_EPresetType(EPresetType type);

class Preset
{
public:
    Preset(int in_bankid, int in_programid, EPresetType in_type, std::string&& in_name)
        : bankid(in_bankid)
        , programid(in_programid)
        , type(in_type)
        , name(std::move(in_name))
    {}
    virtual ~Preset() {}

    virtual Instrument* createPlayingInstance(const Note& note) const = 0;
    virtual EDSPType getDSPType() const = 0;
    virtual const ADSR& getADSR() const = 0;
    virtual void getPWMData(PWMData&) const {}

    const int bankid;
    const int programid;
    const EPresetType type;
    const std::string name;
};

class SynthPreset : public Preset
{
public:
    SynthPreset(int in_bankid, int in_programid, std::string&& in_name, EDSPType in_synthType, ADSR&& in_adsr)
        : Preset(in_bankid, in_programid, EPresetType::Synth, std::move(in_name))
        , synthType(in_synthType)
        , adsr(std::move(in_adsr))
    {}

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final { return synthType; }
    const ADSR& getADSR() const final { return adsr; }

    const EDSPType synthType;

private:
    const ADSR adsr;
};

class PWMSynthPreset : public Preset
{
public:
    PWMSynthPreset(int in_bankid, int in_programid, std::string&& in_name, ADSR&& in_adsr, PWMData&& in_data)
        : Preset(in_bankid, in_programid, EPresetType::Synth, std::move(in_name))
        , adsr(std::move(in_adsr))
        , pwmdata(std::move(in_data))
    {}

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final { return EDSPType::ModPulse; }
    const ADSR& getADSR() const final { return adsr; }
    void getPWMData(PWMData& out_data) const final { out_data = pwmdata; }

    const ADSR adsr;
    const PWMData pwmdata;
};

class SamplePreset : public Preset
{
public:
    SamplePreset(int in_bankid, int in_programid, std::string&& in_name, std::string&& in_filepath, ADSR&& in_adsr, SampleInfo&& in_info);

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final { return EDSPType::PCM; }
    const ADSR& getADSR() const final { return m_info.adsr; }

    bool loadFile(juce::AudioFormatManager& formatManager);

private:
    const std::string m_filepath;
    SampleInfo m_info;

    juce::AudioBuffer<float> m_audioBuffer;
    unsigned int m_numChannels = 0;
    int64_t m_lengthInSamples = 0;
};

class SampleMultiPreset : public Preset
{
public:
    struct MultiSample
    {
        MultiSample(std::string&& in_filePath, SampleInfo&& in_info, uint16_t lowRange, uint16_t upRange)
            : filePath(std::move(in_filePath))
            , info(std::move(in_info))
        {
            keyRange.first = lowRange;
            keyRange.second = upRange;
        }

        std::string filePath;
        SampleInfo info;
        std::pair<uint16_t, uint16_t> keyRange;

        juce::AudioBuffer<float> audioBuffer;
        unsigned int numChannels = 0;
        int64_t lengthInSamples = 0;

        int loopStart = 0;
        int loopEnd = 0;
    };

    SampleMultiPreset(int in_bankid, int in_programid, std::string&& in_name, ADSR&& in_adsr, std::vector<MultiSample>&& in_info);

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final { return EDSPType::PCM; }
    const ADSR& getADSR() const final { return adsr; }

    bool loadFiles(juce::AudioFormatManager& formatManager);

    static void getLoopTimesFromFile(juce::AudioFormatManager& formatManager, std::string filePath, int& loopStart, int& loopEnd);

private:
    std::vector<MultiSample> samples;
    ADSR adsr;
};

class SoundfontPreset : public Preset
{
public:
    SoundfontPreset(int in_bankid, int in_programid, std::string&& in_name, const PresetsHandler& in_presetsHandler, std::vector<SoundfontSampleInfo>&& in_samples)
        : Preset(in_bankid, in_programid, EPresetType::Soundfont, std::move(in_name))
        , m_presetsHandler(in_presetsHandler)
        , samples(std::move(in_samples))
    {}

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final;
    const ADSR& getADSR() const final;

private:

    const PresetsHandler& m_presetsHandler;
    std::vector<SoundfontSampleInfo> samples;

    static const ADSR m_emptyADSR;
};

}