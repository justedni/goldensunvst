#pragma once

#include "Processor/Instrument.h"

namespace GSVST {

struct SynthInfo
{
    SynthInfo(EDSPType inType)
        : type(inType)
    {
    }

    EDSPType getType() const { return type; }

private:
    EDSPType type = EDSPType::ModPulse;
    float midCfreq = 16738;
};

class GSSynth : public Instrument
{
public:
    GSSynth(const Note& in_note)
        : Instrument(in_note)
    {}

    void updateArgs(const MixingArgs& args) override;
    int getMidCFreq() const final { return midCfreq; }

    static GSSynth* createSynth(EDSPType type, const Note& in_note);

protected:
    const int midCfreq = 16738;

    uint32_t pos = 0;
    float interPos = 0.0f;
};

class GSPWMSynth : public GSSynth
{
public:
    GSPWMSynth(const PWMData& pwmdata, const Note& in_note)
        : GSSynth(in_note)
        , m_data(pwmdata)
    {}

    EDSPType getType() const final { return EDSPType::ModPulse; }
    void processStart(const MixingArgs& args) final;
    void process(sample* buffer, size_t numSamples, const MixingArgs& args) final;

    void updatePWMData(const PWMData& in_data) final
    {
        m_data = PWMData(in_data);
    }

    static GSPWMSynth* createPWMSynth(const PWMData& pwmdata, const Note& in_note);

private:
    void calculateModPulseThreshold(float nBlocksReciprocal);

    float m_deltaThresh = 0.0f;
    float m_fThreshold = 0.0f;
    float m_baseThresh = 0.0f;
    float m_threshStep = 0.0f;

    PWMData m_data;
};

class GSSawSynth : public GSSynth
{
public:
    GSSawSynth(const Note& in_note)
        : GSSynth(in_note)
    {}

    EDSPType getType() const final { return EDSPType::Saw; }
    void process(sample* buffer, size_t numSamples, const MixingArgs& args) final;
};

class GSTriangleSynth : public GSSynth
{
public:
    GSTriangleSynth(const Note& in_note)
        : GSSynth(in_note)
    {}

    EDSPType getType() const final { return EDSPType::Tri; }
    void process(sample* buffer, size_t numSamples, const MixingArgs& args) final;
};

}