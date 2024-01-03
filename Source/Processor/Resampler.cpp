#include "Resampler.h"

namespace GSVST {

Resampler::~Resampler()
{
}

LinearResampler::LinearResampler()
{
    Reset();
}

LinearResampler::~LinearResampler()
{
}

void LinearResampler::Reset()
{
    fetchBuffer.clear();
    phase = 0.0f;
}

bool LinearResampler::Process(sample *outData, size_t numBlocks, float phaseInc, res_data_fetch_cb cbPtr, void *cbdata)
{
    if (numBlocks == 0)
        return true;

    size_t samplesRequired = static_cast<size_t>(
            phase + phaseInc * static_cast<float>(numBlocks));
    // be sure and fetch one more sample in case of odd rounding errors
    samplesRequired += 1;
    // fetch one more for linear interpolation
    samplesRequired += 1;
    bool result = cbPtr(fetchBuffer, samplesRequired, cbdata);

    auto getSample = [&](float& a, float& b)
    {
        return (a + phase * (b - a));
    };

    int i = 0;
    do {
        float sampleLeft = getSample(fetchBuffer[i].left, fetchBuffer[i + 1].left);
        float sampleRight = getSample(fetchBuffer[i].right, fetchBuffer[i + 1].right);

        phase += phaseInc;
        int istep = static_cast<int>(phase);
        phase -= static_cast<float>(istep);
        i += istep;

        outData->left = sampleLeft;
        outData->right = sampleRight;
        outData++;
    } while (--numBlocks > 0);

    // remove first i elements from the fetch buffer since they are no longer needed
    fetchBuffer.erase(fetchBuffer.begin(), fetchBuffer.begin() + i);

    return result;
}

}
