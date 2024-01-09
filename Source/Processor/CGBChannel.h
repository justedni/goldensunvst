#pragma once

#include "Instrument.h"

#include <cstdint>
#include <cstddef>
#include <memory>

#include "Types.h"
#include "Resampler.h"

#define INVALID_TRACK_IDX 0xFF

#define NOISE_SAMPLING_FREQ 65536.0f


namespace GSVST {

enum class WaveDuty : int { D12 = 0, D25, D50, D75 };

class CGBChannel : public Instrument
{
public: 
    CGBChannel(const Note& note, bool useStairstep = false);
    CGBChannel(const CGBChannel&) = delete;
    CGBChannel& operator=(const CGBChannel&) = delete;
    virtual ~CGBChannel() = default;

protected:
    void stepEnvelope() override;
    void updateVolFade() override;
    void applyVol();
    VolumeFade getVol() const override;

    static float timer2freq(float timer);
    static float freq2timer(float freq);

    std::unique_ptr<Resampler> rs;
    enum class Pan { LEFT, CENTER, RIGHT };
    uint32_t pos = 0;
    const bool useStairstep;

    /* all of these values have pairs of new and old value to allow smooth fades */
    uint8_t envPeak = 0;
    uint8_t envSustain = 0;
    uint8_t envFrameCount = 0;
    uint8_t envGradientFrame = 0;
    float envGradient = 0.0f;
    Pan panCur = Pan::CENTER;
    Pan panPrev = Pan::CENTER;
};

class SquareChannel : public CGBChannel
{
public:
    SquareChannel(WaveDuty wd, const Note& in_note, uint8_t sweep);

    EDSPType getType() const final { return EDSPType::Square; }
    int getMidCFreq() const override { return 3520; }

    void updatePitch() override;

    void process(sample* buffer, size_t numSamples, const MixingArgs& args) override;
    void updateArgs(const MixingArgs& args) override;
private:
    static bool sampleFetchCallback(std::vector<sample>& fetchBuffer, size_t samplesRequired, void* cbdata);

    static bool isSweepEnabled(uint8_t sweep);
    static bool isSweepAscending(uint8_t sweep);
    static float sweep2coeff(uint8_t sweep);
    static float sweep2convergence(uint8_t sweep);
    static uint8_t sweepTime(uint8_t sweep);

    const float *pat = nullptr;
    int16_t sweepStartCount = -1;
    const uint8_t sweep;
    const bool sweepEnabled;
    const float sweepConvergence;
    const float sweepCoeff;
    /* sweepTimer is emulated with float instead of hardware int to get sub frame accuracy */
    float sweepTimer = 1.0f;
};

}