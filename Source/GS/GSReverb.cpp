#include "GSReverb.h"

#include <assert.h>

namespace GSVST {

ReverbGS1::ReverbGS1(uint8_t intensity, size_t samplesPerBufferForComputation, uint8_t numAgbBuffers)
    : ReverbEffect(intensity, samplesPerBufferForComputation, numAgbBuffers)
    , gsBuffer(samplesPerBufferForComputation * INTERFRAMES, sample{ 0.0f, 0.0f })
{
    bufferPos2 = 0;
}

ReverbGS1::~ReverbGS1()
{
}

size_t ReverbGS1::getBlocksPerGsBuffer() const
{
    return gsBuffer.size();
}

size_t ReverbGS1::processInternal(sample* buffer, const size_t numSamples, const size_t numSamplesForCount, bool bRecalculate)
{
    auto requestedNumSamples = numSamples;

    std::vector<sample>& rbuf = reverbBuffer;

    const size_t bPerBuf = getBlocksPerBuffer();
    const size_t bPerGsBuf = getBlocksPerGsBuffer();

    auto recalculate = [&]()
    {
        if (resetGS)
        {
            bufferPos2 = 0;
            resetGS = false;
        }

        if (reset)
        {
            bufferPos = 0;
            reset = false;
        }

        count = std::min(std::min(bPerBuf - bufferPos, bPerGsBuf - bufferPos2), numSamplesForCount);

        if (count == bPerBuf - bufferPos)
            reset = true;
        if (count == bPerGsBuf - bufferPos2)
            resetGS = true;
    };

    if (bRecalculate)
    {
        recalculate();
    }

    size_t c = std::min(count, numSamples);
    size_t processedSamples = c;

    do {
        float mixL = buffer->left  + gsBuffer[bufferPos2].left;
        float mixR = buffer->right + gsBuffer[bufferPos2].right;

        float lA = rbuf[bufferPos].left;
        float rA = rbuf[bufferPos].right;

        buffer->left  = rbuf[bufferPos].left  = mixL;
        buffer->right = rbuf[bufferPos].right = mixR;

        float lRMix = 0.25f * mixL + 0.25f * rA;
        float rRMix = 0.25f * mixR + 0.25f * lA;

        gsBuffer[bufferPos2].left  = lRMix;
        gsBuffer[bufferPos2].right = rRMix;

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


ReverbGS2::ReverbGS2(uint8_t intensity, size_t samplesPerBufferForComputation, uint8_t numAgbBuffers,
    float rPrimFac, float rSecFac)
    : ReverbEffect(intensity, samplesPerBufferForComputation, numAgbBuffers)
    , gs2Buffer(samplesPerBufferForComputation* INTERFRAMES, sample{ 0.0f, 0.0f })
{
    // equivalent to the offset of -0xB0 samples for a 0x210 buffer size
    bufferPos2 = (getBlocksPerBuffer() * 176) / 528;
    gs2Pos = 0;
    this->rPrimFac = rPrimFac;
    this->rSecFac = rSecFac;
}

ReverbGS2::~ReverbGS2()
{
}

size_t ReverbGS2::processInternal(sample *buffer, const size_t numSamples, const size_t numSamplesForCount, bool bRecalculate)
{
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
        if (resetgs2)
        {
            gs2Pos = 0;
            resetgs2 = false;
        }

        count = std::min(
            std::min(blocksPerBuffer - bufferPos2, blocksPerBuffer - bufferPos),
            std::min(numSamplesForCount, gs2Buffer.size() - gs2Pos)
        );

        if (blocksPerBuffer - bufferPos2 == count) {
            reset2 = true;
        }
        if (blocksPerBuffer - bufferPos == count) {
            reset = true;
        }
        if ((gs2Buffer.size() / 2) - gs2Pos == count) {
            resetgs2 = true;
        }
    };

    if (bRecalculate)
    {
        recalculate();
    }

    size_t c = std::min(count, numSamples);

    size_t processedSamples = c;

    do {
        float mixL = buffer->left  + gs2Buffer[gs2Pos].left;
        float mixR = buffer->right + gs2Buffer[gs2Pos].right;

        float lA = rbuf[bufferPos].left;
        float rA = rbuf[bufferPos].right;

        buffer->left  = rbuf[bufferPos].left  = mixL;
        buffer->right = rbuf[bufferPos].right = mixR;

        float lRMix = lA * rPrimFac + rA * rSecFac;
        float rRMix = rA * rPrimFac + lA * rSecFac;

        float lB = rbuf[bufferPos2].right * 0.25f;
        float rB = mixR * 0.25f;

        gs2Buffer[gs2Pos].left  = lRMix + lB;
        gs2Buffer[gs2Pos].right = rRMix + rB;

        buffer++;

        bufferPos++;
        bufferPos2++;
        gs2Pos++;
    } while (--c > 0);

    if (numSamples != numSamplesForCount && count > numSamples)
    {
        count -= numSamples;
        left = count;
    }
    else
    {
        left = numSamplesForCount - count;
    }

    return processedSamples;
}

}
