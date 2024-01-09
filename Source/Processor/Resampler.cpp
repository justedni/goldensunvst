#include "Resampler.h"

namespace GSVST {

#define SINC_WINDOW_SIZE 16
#define SINC_FILT_THRESH 0.85f
#define M_PI 3.14159265358979323846

// anything higher than 256 LUT size seems to be indistinguishable
#define LUT_SIZE 256

#define INTEGRAL_RESOLUTION 256


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


BlepResampler::BlepResampler()
{
    Reset();
}

BlepResampler::~BlepResampler()
{
}


void BlepResampler::Reset()
{
    fetchBuffer.clear();
    fetchBuffer.resize(SINC_WINDOW_SIZE, { 0.0f, 0.0f });
    phase = 0.0f;
}

bool BlepResampler::Process(sample* outData, size_t numBlocks, float phaseInc, res_data_fetch_cb cbPtr, void *cbdata)
{
    if (numBlocks == 0)
        return true;

    size_t samplesRequired = static_cast<size_t>(
            phase + phaseInc * static_cast<float>(numBlocks));
    // be sure and fetch one more sample in case of odd rounding errors
    samplesRequired += 1;
    // fetch a few more for complete windowed sinc interpolation
    samplesRequired += SINC_WINDOW_SIZE * 2;
    bool result = cbPtr(fetchBuffer, samplesRequired, cbdata);

    float sincStep = SINC_FILT_THRESH / phaseInc;

    int i = 0;
    do {
        float leftSampleSum = 0.0f;
        float rightSampleSum = 0.0f;
        float kernelSum = 0.0f;
        for (int wi = -SINC_WINDOW_SIZE + 1; wi <= SINC_WINDOW_SIZE; wi++) {
            float SiIndexLeft = (float(wi) - phase - 0.5f) * sincStep;
            float SiIndexRight = (float(wi) - phase + 0.5f) * sincStep;
            float sl = fast_Si(SiIndexLeft);
            float sr = fast_Si(SiIndexRight);
            float kernel = sr - sl;
            leftSampleSum += kernel * fetchBuffer[i + wi + SINC_WINDOW_SIZE - 1].left;
            rightSampleSum += kernel * fetchBuffer[i + wi + SINC_WINDOW_SIZE - 1].right;
            kernelSum += kernel;
        }
        phase += phaseInc;
        int istep = static_cast<int>(phase);
        phase -= static_cast<float>(istep);
        i += istep;

        outData->left = leftSampleSum / kernelSum;
        outData->right = rightSampleSum / kernelSum;
        outData++;
        //*outData++ = sampleSum;
        //_print_debug("kernel sum: %f\n", kernelSum);
    } while (--numBlocks > 0);
    // remove first i elements from the fetch buffer since they are no longer needed
    fetchBuffer.erase(fetchBuffer.begin(), fetchBuffer.begin() + i);

    return result;
}

float sinc_pi(const float x)
{
    return (sin(x) / x);
}

static const std::vector<float> Si_lut = []() {
    std::vector<float> l(LUT_SIZE+2);
    double acc = 0.0;
    double step_per_index = double(SINC_WINDOW_SIZE) / double(LUT_SIZE);
    double integration_inc = step_per_index / double(INTEGRAL_RESOLUTION);
    double index = 0.0;
    double prev_value = 1.0;

    for (size_t i = 0; i < LUT_SIZE+1; i++) {
        double convergence_level = 0.5 - 0.5 * cos(double(i) * M_PI / double(LUT_SIZE));
        double integral_level = 1.0 - convergence_level;
        l[i] = static_cast<float>(acc * integral_level + 0.5 * convergence_level);
        for (size_t j = 0; j < INTEGRAL_RESOLUTION; j++) {
            index += integration_inc;
            double new_value = sinc_pi(M_PI * index);
            acc += (new_value + prev_value) * integration_inc * 0.5;
            prev_value = new_value;
        }
    }
    l[LUT_SIZE+1] = 0.5f;
    l.shrink_to_fit();
    return l;
}();

float BlepResampler::fast_Si(float t)
{
    float signed_t = t;
    t = fabs(t);
    t = std::min(t, float(SINC_WINDOW_SIZE));
    t *= float(double(LUT_SIZE) / double(SINC_WINDOW_SIZE));
    uint32_t left_index = static_cast<uint32_t>(t);
    float fraction = t - static_cast<float>(left_index);
    uint32_t right_index = left_index + 1;
    float retval = Si_lut[left_index] + fraction * (Si_lut[right_index] - Si_lut[left_index]);
    return copysignf(retval, signed_t);
}

}
