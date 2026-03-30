#pragma once

#include <cstdint>
#include <vector>

#include "Instrument.h"

namespace GSVST {

struct SampleInfo
{
    static constexpr int DEFAULT_MIDC_FREQ = 16738;

    SampleInfo()
        : midCfreq(DEFAULT_MIDC_FREQ)
        , loopPos(0)
        , endPos(0)
        , loopEnabled(false)
    {
    }

    SampleInfo(bool in_loopEnabled, uint32_t in_loopPos, uint32_t in_endPos)
        : midCfreq(DEFAULT_MIDC_FREQ)
        , loopPos(in_loopPos)
        , endPos(in_endPos)
        , loopEnabled(in_loopEnabled)
    {
    }

    SampleInfo(const SampleInfo& other) = default;

    void setMidCFreq(int pitch_correction, int original_pitch, int sample_rate);

    virtual void fillSample(uint32_t pos, float& out_left, float& out_right) const
    {
        out_left = sampleBuffer[0][pos];
        if (numChannels == 2)
            out_right = sampleBuffer[1][pos];
        else
            out_right = out_left;
    }

    virtual EDSPType getType() const { return EDSPType::PCM; }

    int rootNote = 0;
    int midCfreq = DEFAULT_MIDC_FREQ;
    const float* const* sampleBuffer = nullptr;
    uint32_t loopPos = 0;
    uint32_t endPos = 0;
    bool loopEnabled = false;
    uint8_t numChannels = 1;

    ADSR adsr;
    uint8_t notePitch = 0;
    int8_t rhythmPan = 0;
};


struct SoundfontSampleInfo : public SampleInfo
{
    SoundfontSampleInfo(bool in_fixed, uint32_t in_fixedSampleRate, bool in_loopEnabled, uint32_t in_loopPos, uint32_t in_endPos)
        : SampleInfo(in_loopEnabled, in_loopPos, in_endPos)
        , fixed(in_fixed)
        , fixedSampleRate(in_fixedSampleRate)
    {
    }

    SoundfontSampleInfo(const SoundfontSampleInfo& other)
        : SampleInfo(other)
        , keyRange(other.keyRange)
        , fixed(other.fixed)
        , fixedSampleRate(other.fixedSampleRate)
        , offset(other.offset)
    {
    }

    void fillSample(uint32_t pos, float& out_left, float& out_right) const override
    {
        out_left = out_right = soundFontSamplePtr[pos];
    }

    EDSPType getType() const override { return fixed ? EDSPType::PCMFixed : EDSPType::PCM; }

    float* soundFontSamplePtr = nullptr;

    std::pair<uint16_t, uint16_t> keyRange;

    bool fixed = false;
    uint32_t fixedSampleRate = 0;
    unsigned int offset = 0;
};

class SampleInstrument : public Instrument
{
public:
    SampleInstrument(SampleInfo* sInfo, const Note& note);

    EDSPType getType() const final { return m_info->getType(); }
    void process(sample* buffer, size_t numSamples, const MixingArgs& args) final;
    static bool sampleFetchCallback(std::vector<sample>& fetchBuffer, size_t samplesRequired, void* cbdata);

    void updateArgs(const MixingArgs& args) override;
    int getMidCFreq() const override { return m_info->midCfreq; }

private:
    const uint8_t m_inNumChannels;

    uint32_t pos = 0;

    std::unique_ptr<Resampler> m_resampler;
    std::unique_ptr<SampleInfo> m_info;
};

class SoundfontSampleInstrument : public SampleInstrument
{
public:
    SoundfontSampleInstrument(SoundfontSampleInfo* sInfo, const Note& note);

    void updateArgs(const MixingArgs& args) override;

private:
    const bool m_fixed;
    const uint32_t m_fixedModeRate = 0;
};

}
