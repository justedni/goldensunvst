#pragma once

#include "Processor/SampleInstrument.h"

#include <string>

template <class T>
class AudioFile;

struct tsf;

namespace GSVST {

class Instrument;
struct PresetsHandler;

enum class EPresetType : uint8_t
{
    Sample,
    Soundfont,
    Synth,
    PWMSynth
};

class Preset
{
public:
    Preset(int in_id, EPresetType in_type, std::string&& in_name)
        : programid(in_id)
        , type(in_type)
        , name(std::move(in_name))
    {}
    virtual ~Preset() {}

    virtual Instrument* createPlayingInstance(const Note& note) const = 0;
    virtual EDSPType getDSPType() const = 0;
    virtual const ADSR& getADSR() const = 0;
    virtual void getPWMData(PWMData&) const {}

    const int programid;
    const EPresetType type;
    const std::string name;
};

class SynthPreset : public Preset
{
public:
    SynthPreset(int in_id, std::string&& in_name, EDSPType in_synthType, ADSR&& in_adsr)
        : Preset(in_id, EPresetType::Synth, std::move(in_name))
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
    PWMSynthPreset(int in_id, std::string&& in_name, ADSR&& in_adsr, PWMData&& in_data)
        : Preset(in_id, EPresetType::PWMSynth, std::move(in_name))
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
    SamplePreset(int in_id, std::string&& in_name, std::string&& in_filepath, ADSR&& in_adsr, SampleInfo&& in_info);

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final { return EDSPType::PCM; }
    const ADSR& getADSR() const final { return info.adsr; }

    bool loadFile();

private:
    const std::string filepath;
    SampleInfo info;

    std::unique_ptr<AudioFile<float>> audioFile;
};

class SoundfontPreset : public Preset
{
public:
    SoundfontPreset(int in_id, std::string&& in_name, const PresetsHandler& in_presetsHandler, std::vector<SoundfontSampleInfo>&& in_samples)
        : Preset(in_id, EPresetType::Soundfont, std::move(in_name))
        , m_presetsHandler(in_presetsHandler)
        , samples(std::move(in_samples))
    {}

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final;
    const ADSR& getADSR() const final;

private:
    const SoundfontSampleInfo* findSample(int note) const
    {
        auto found = std::find_if(samples.begin(), samples.end(), [note](auto& s) { return note >= s.keyRange.first && note <= s.keyRange.second; });
        if (found != samples.end())
            return &(*found);

        return nullptr;
    }

    const PresetsHandler& m_presetsHandler;
    std::vector<SoundfontSampleInfo> samples;

    static const ADSR m_emptyADSR;
};

}