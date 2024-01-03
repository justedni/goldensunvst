#pragma once

#include "Processor/ReverbEffect.h"

namespace GSVST {

class ReverbGS1 : public ReverbEffect
{
public:
    ReverbGS1(uint8_t intensity, size_t samplesPerBufferForComputation, uint8_t numAgbBuffers);
    ~ReverbGS1() override;
protected:
    size_t processInternal(sample *buffer, const size_t numSamples, const size_t numSamplesForCount, bool bRecalculate) override;
    size_t getBlocksPerGsBuffer() const;
    std::vector<sample> gsBuffer;

    bool resetGS = false;
};

class ReverbGS2 : public ReverbEffect
{
public:
    ReverbGS2(uint8_t intesity, size_t samplesPerBufferForComputation, uint8_t numAgbBuffers,
            float rPrimFac, float rSecFac);
    ~ReverbGS2() override;
protected:
    size_t processInternal(sample *buffer, const size_t numSamples, const size_t numSamplesForCount, bool bRecalculate) override;
    std::vector<sample> gs2Buffer;
    size_t gs2Pos;
    float rPrimFac, rSecFac;

    bool reset2 = false;
    bool resetgs2 = false;
};

}
