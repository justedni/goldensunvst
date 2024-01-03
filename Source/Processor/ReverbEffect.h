#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <fstream>

#include "Types.h"

namespace GSVST {

class ReverbEffect
{
public:
    ReverbEffect(uint8_t intensity, size_t samplesPerBufferForComputation, uint8_t numAgbBuffers);
    virtual ~ReverbEffect();
    void ProcessData(sample *buffer, size_t numSamples, size_t samplesPerBufferForComputation);

    void SetDebugFile(std::fstream* in_file) { debug_file = in_file; }

    void SetIntensity(int val) { intensity = val / 128.0f; }
protected:
    virtual size_t processInternal(sample *buffer, const size_t numSamples, const size_t numSamplesForCount, bool bRecalculate);
    size_t getBlocksPerBuffer() const;
    float intensity;
    uint8_t numAgbBuffers;

    std::vector<sample> reverbBuffer;
    size_t bufferPos;
    size_t bufferPos2;

    size_t left = 0;
    size_t count = 0;

    bool reset = false;
    bool reset2 = false;

    std::fstream* debug_file = nullptr;
};

}
