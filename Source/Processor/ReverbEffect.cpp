#include <algorithm>
#include <cassert>
#include <string>

#include "ReverbEffect.h"

namespace GSVST {

ReverbEffect::ReverbEffect(uint8_t intensity, size_t samplesPerBufferForComputation, uint8_t numAgbBuffers)
    : reverbBuffer(samplesPerBufferForComputation * INTERFRAMES * numAgbBuffers, sample{ 0.0f, 0.0f })
{
    this->intensity = intensity / 128.0f;
    this->numAgbBuffers = numAgbBuffers;

    size_t bufferLen = samplesPerBufferForComputation * INTERFRAMES;
    bufferPos = 0;
    bufferPos2 = bufferLen;
}

ReverbEffect::~ReverbEffect()
{
}

void ReverbEffect::ProcessData(sample *buffer, size_t numSamples, size_t samplesPerBufferForComputation)
{
    bool bRecalculate = !(left > 0);

    while (numSamples > 0)
    {
        size_t samplesToProcess = std::min(samplesPerBufferForComputation, numSamples);
        if (left > 0)
            samplesToProcess = std::min(left, samplesToProcess);

        const size_t samplesForCountCalculation = (left > 0) ? left : samplesPerBufferForComputation;
        auto processedSamples = processInternal(buffer, samplesToProcess, samplesForCountCalculation, bRecalculate);
        buffer += processedSamples;
        numSamples -= processedSamples;
        bRecalculate = true; // Always recalculate after first pass
    }
}

size_t ReverbEffect::getBlocksPerBuffer() const
{
    return reverbBuffer.size();
}

size_t ReverbEffect::processInternal(sample *buffer, const size_t numSamples, const size_t numSamplesForCount, bool bRecalculate)
{
    auto requestedNumSamples = numSamples;

    assert(numSamples > 0);
    std::vector<sample>& rbuf = reverbBuffer;

    const auto blocksPerBuffer = getBlocksPerBuffer();

    auto recalculate = [&]()
    {
        if (reset2)
        {
            bufferPos2 = 0;
            reset2 = false;
        }

        if (reset)
        {
            bufferPos = 0;
            reset = false;
        }

        count = std::min(std::min(blocksPerBuffer - bufferPos2, blocksPerBuffer - bufferPos), numSamplesForCount);

        if (blocksPerBuffer - bufferPos == count) {
            reset = true;
        }
        if (blocksPerBuffer - bufferPos2 == count) {
            reset2 = true;
        }
    };

    if (bRecalculate)
    {
        recalculate();
    }

    size_t c = std::min(count, numSamples);
    size_t processedSamples = c; // Returning number of samples that are actually going to be processed this frame

    do {
        float rev = (rbuf[bufferPos].left + rbuf[bufferPos].right + 
                rbuf[bufferPos2].left + rbuf[bufferPos2].right) * intensity * (1.0f / 4.0f);
        rbuf[bufferPos].left  = buffer->left  += rev;
        rbuf[bufferPos].right = buffer->right += rev;
        buffer++;
        bufferPos++;
        bufferPos2++;
    } while (--c > 0);

    if (requestedNumSamples != numSamplesForCount && count > requestedNumSamples)
    {
        count -= requestedNumSamples;
        left = count;
    }
    else
    {
        left = numSamplesForCount - count;
    }

    return processedSamples;
}

}
