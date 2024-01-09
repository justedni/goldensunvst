#include "CGBChannel.h"

#include <cmath>
#include <cassert>
#include <algorithm>

#include "CGBPatterns.h"

namespace GSVST {

/*
 * public CGBChannel
 */

CGBChannel::CGBChannel(const Note& in_note, bool useStairstep)
    : Instrument(in_note)
    , useStairstep(useStairstep)
{
    this->env.att &= 0x7;
    this->env.dec &= 0x7;
    this->env.sus &= 0xF;
    this->env.rel &= 0x7;
}

CGBChannel::VolumeFade CGBChannel::getVol() const
{
    float envBase = static_cast<float>(envLevelPrev);
    float finalFromEnv = envBase + envGradient * static_cast<float>(envGradientFrame * INTERFRAMES + envInterStep);
    float finalToEnv = finalFromEnv + envGradient;

    VolumeFade retval;
    retval.fromVolLeft = (panPrev == Pan::RIGHT) ? 0.0f : finalFromEnv * (1.0f / 32.0f);
    retval.fromVolRight = (panPrev == Pan::LEFT) ? 0.0f : finalFromEnv * (1.0f / 32.0f);
    retval.toVolLeft = (panCur == Pan::RIGHT) ? 0.0f : finalToEnv * (1.0f / 32.0f);
    retval.toVolRight = (panCur == Pan::LEFT) ? 0.0f : finalToEnv * (1.0f / 32.0f);
    return retval;
}

float CGBChannel::timer2freq(float timer)
{
    assert(timer >= 0.0f);
    assert(timer <= 2047.0f);
    return 131072.0f / static_cast<float>(2048.0f - timer);
}

float CGBChannel::freq2timer(float freq)
{
    assert(freq > 0.0f);
    return 2048.0f - 131072.0f / freq;
}

void CGBChannel::stepEnvelope()
{
    if (envState == EnvState::INIT) {
        if (stop) {
            envState = EnvState::DEAD;
            return;
        }

        applyVol();
        updateVolFade();

        envLevelCur = 0;
        envInterStep = 0;
        envState = EnvState::ATK;

        if (env.att > 0) {
            envLevelPrev = 0;
        } else if (env.dec > 0) {
            envLevelPrev = envPeak;
            envLevelCur = envPeak;
            if (envPeak > 0) {
                envState = EnvState::DEC;
                if (useStairstep) {
                    envFrameCount = env.dec;
                    return;
                }
            } else {
                envState = EnvState::SUS;
            }
        } else {
            envLevelPrev = envSustain;
            envLevelCur = envSustain;
            envState = EnvState::SUS;
        }
    } else {
        if (useStairstep) {
            envLevelPrev = envLevelCur;
            envGradient = 0.0f;
        }

        if (++envInterStep < INTERFRAMES)
            return;

        envInterStep = 0;

        assert(envFrameCount > 0);
        envFrameCount--;
        envGradientFrame++;
    }

    bool fromDecay;

    if (envState == EnvState::PSEUDO_ECHO) {
        assert(note.pseudoEchoLen != 0);
        if (--note.pseudoEchoLen == 0) {
            envState = EnvState::DIE;
            envLevelCur = 0;
        }
        envFrameCount = 1;
        envGradient = 0.0f;
    } else if (stop && envState < EnvState::REL) {
        envState = EnvState::REL;
        envFrameCount = env.rel;
        if (envLevelCur == 0 || envFrameCount == 0) {
            fromDecay = false;
            goto pseudo_echo_start;
        }
        envGradientFrame = 0;
        envLevelPrev = envLevelCur;
        goto release;

    } else if (envFrameCount == 0) {
        envGradientFrame = 0;
        envLevelPrev = envLevelCur;

        applyVol();

        if (envState == EnvState::REL) {
release:
            assert(envLevelCur > 0);
            envLevelCur--;
            assert((int8_t)envLevelCur >= 0);

            if (envLevelCur == 0) {
                fromDecay = false;
pseudo_echo_start:
                envLevelCur = static_cast<uint8_t>(((envPeak * note.pseudoEchoVol) + 0xFF) >> 8);
                if (envLevelCur != 0 && note.pseudoEchoLen != 0) {
                    envState = EnvState::PSEUDO_ECHO;
                    envFrameCount = 1;
                    envLevelPrev = envLevelCur;
                } else {
                    if (fromDecay) {
                        envState = EnvState::DIE;
                        envLevelCur = 0;
                        envFrameCount = env.dec;
                    } else if (env.rel == 0) {
                        envState = EnvState::DEAD;
                        return;
                    } else {
                        envState = EnvState::DIE;
                        envLevelCur = 0;
                        envFrameCount = env.rel;
                    }
                }
            } else {
                envFrameCount = env.rel;
                assert(env.rel != 0);
            }
        } else if (envState == EnvState::SUS) {
            envLevelCur = envSustain;
            envFrameCount = 1;
        } else if (envState == EnvState::DEC) {
            envLevelCur--;
            assert((int8_t)envLevelCur >= 0);

            if (envLevelCur <= envSustain) {
                if (env.sus == 0) {
                    fromDecay = true;
                    goto pseudo_echo_start;
                } else {
                    envState = EnvState::SUS;
                    envLevelCur = envSustain;
                }
            }
            envFrameCount = env.dec;
            assert(env.dec != 0);
        } else if (envState == EnvState::ATK) {
            envLevelCur++;

            if (envLevelCur >= envPeak) {
                envState = EnvState::DEC;
                envFrameCount = env.dec;
                if (envPeak == 0 || envFrameCount == 0) {
                    envState = EnvState::SUS;
                } else {
                    envLevelCur = envPeak;
                }
            }
            envFrameCount = env.att;
            assert(env.att != 0);
        } else if (envState == EnvState::DIE) {
            envState = EnvState::DEAD;
            return;
        }

        assert(envFrameCount != 0);
        if (useStairstep)
            envGradient = static_cast<float>(envLevelCur - envLevelPrev);
        else
            envGradient = static_cast<float>(envLevelCur - envLevelPrev) / static_cast<float>(envFrameCount * INTERFRAMES);
    }
}

void CGBChannel::updateVolFade()
{
    panPrev = panCur;
}

void CGBChannel::applyVol()
{
    int combinedPan = std::clamp(pan + note.rhythmPan, -64, +63);

    if (combinedPan < -21)
        this->panCur = Pan::LEFT;
    else if (combinedPan > 20)
        this->panCur = Pan::RIGHT;
    else
        this->panCur = Pan::CENTER;

    int volA = (128 * (vol << 1)) >> 8;
    int volB = (127 * (vol << 1)) >> 8;
    volA = (note.velocity * 128 * volA) >> 14;
    volB = (note.velocity * 127 * volB) >> 14;

    envPeak = static_cast<uint8_t>(std::clamp((volA + volB) >> 4, 0, 15));
    envSustain = static_cast<uint8_t>(std::clamp((envPeak * env.sus + 15) >> 4, 0, 15));
    // TODO is this if below right???
    if (envState == EnvState::SUS)
        envLevelCur = envSustain;
}

/*
 * public SquareChannel
 */

SquareChannel::SquareChannel(WaveDuty wd, const Note& in_note, uint8_t sweep)
    : CGBChannel(in_note)
      , sweep(sweep)
      , sweepEnabled(isSweepEnabled(sweep))
      , sweepConvergence(sweep2convergence(sweep))
      , sweepCoeff(sweep2coeff(sweep))
{
    static const float *patterns[4] = {
        CGBPatterns::pat_sq12,
        CGBPatterns::pat_sq25,
        CGBPatterns::pat_sq50,
        CGBPatterns::pat_sq75,
    };

    this->pat = patterns[static_cast<int>(wd)];
    this->rs = std::make_unique<BlepResampler>();
}

void SquareChannel::updatePitch()
{
    auto pitch = tune + pitchWheel * pitchBendRange + (lfoValue * 4);

    // non original quality improving behavior
    if (!stop || freq <= 0.0f)
        freq = 3520.0f * powf(2.0f, float(getMidiKeyPitch() - 69) * (1.0f / 12.0f) + float(pitch) * (1.0f / 768.0f));

    if (sweepEnabled && sweepStartCount < 0) {
        sweepTimer = freq2timer(freq / 8.0f);
        /* Because the initial frequency of a sweeped sound can only be set
         * in the beginning, we have to differentiate between the first and the other
         * SetPitch calls. */
        uint8_t time = sweepTime(sweep);
        assert(time != 0);
        sweepStartCount = static_cast<int16_t>(time * AGB_FPS * INTERFRAMES);
    }
}

void SquareChannel::updateArgs(const MixingArgs& args)
{
    VolumeFade volFade = getVol();
    assert(pat);
    cargs.lVolStep = (volFade.toVolLeft - volFade.fromVolLeft) * args.samplesPerBufferInv;
    cargs.rVolStep = (volFade.toVolRight - volFade.fromVolRight) * args.samplesPerBufferInv;
    cargs.lVol = volFade.fromVolLeft;
    cargs.rVol = volFade.fromVolRight;

    if (sweepEnabled)
    {
        cargs.interStep = 8.0f * timer2freq(sweepTimer) * args.sampleRateInv;
    }
    else
    {
        cargs.interStep = freq * args.sampleRateInv;
    }
}

void SquareChannel::process(sample* buffer, size_t numSamples, const MixingArgs& args)
{
    if (!cargs.bInitialized)
    {
        // First init
        updateArgs(args);
        cargs.bInitialized = true;
    }

    sample* outBuffer = new sample[numSamples];

    rs->Process(outBuffer, numSamples, cargs.interStep, sampleFetchCallback, this);

    size_t i = 0;
    do {
        processStart(args);

        buffer->left  += outBuffer[i].left * cargs.lVol;
        buffer->right += outBuffer[i].right * cargs.rVol;
        buffer++;
        i++;
        cargs.lVol += cargs.lVolStep;
        cargs.rVol += cargs.rVolStep;

        processEnd(args);
    } while (--numSamples > 0);

    if (sweepEnabled) {
        assert(sweepStartCount >= 0);
        if (sweepStartCount == 0) {
            sweepTimer *= sweepCoeff;
            if (isSweepAscending(sweep))
                sweepTimer = std::min(sweepTimer, sweepConvergence);
            else
                sweepTimer = std::max(sweepTimer, sweepConvergence);
        } else {
            sweepStartCount -= 128;
            if (sweepStartCount < 0)
                sweepStartCount = 0;
        }
    }

    delete[] outBuffer;
}

bool SquareChannel::sampleFetchCallback(std::vector<sample>& fetchBuffer, size_t samplesRequired, void* cbdata)
{
    if (fetchBuffer.size() >= samplesRequired)
        return true;
    SquareChannel *_this = static_cast<SquareChannel *>(cbdata);
    size_t samplesToFetch = samplesRequired - fetchBuffer.size();
    size_t i = fetchBuffer.size();
    fetchBuffer.resize(samplesRequired);

    do {
        fetchBuffer[i].left = fetchBuffer[i].right = _this->pat[_this->pos];
        _this->pos++;
        _this->pos %= 8;
        i++;
    } while (--samplesToFetch > 0);
    return true;
}

bool SquareChannel::isSweepEnabled(uint8_t sweep)
{
    if (sweep >= 0x80 || (sweep & 0x7) == 0)
        return false;
    else
        return true;
}

bool SquareChannel::isSweepAscending(uint8_t sweep)
{
    if (sweep & 8)
        return false;
    else
        return true;
}

float SquareChannel::sweep2coeff(uint8_t sweep)
{
    /* if sweep time is zero, don't change pitch */
    const int sweep_time = sweepTime(sweep);
    if (sweep_time == 0)
        return 1.0f;

    const int shifts = sweep & 7;
    float step_coeff;

    if (isSweepAscending(sweep)) {
        /* if ascending */
        step_coeff = static_cast<float>(128 + (128 >> shifts)) / 128.0f;
    } else {
        /* if descending */
        step_coeff = static_cast<float>(128 - (128 >> shifts)) / 128.0f;
    }

    /* convert the sweep pitch timer coefficient to the rate that agbplay runs at */
    const float hardware_sweep_rate = 128 / static_cast<float>(sweep_time);
    const float agbplay_sweep_rate = AGB_FPS * INTERFRAMES;
    const float coeff = powf(step_coeff, hardware_sweep_rate / agbplay_sweep_rate);

    return coeff;
}

float SquareChannel::sweep2convergence(uint8_t sweep)
{
    if (isSweepAscending(sweep)) {
        /* if ascending:
         *
         * Convergance is always at the maximum timer value to prevent hardware overflow */
        return 2047.0f;
    } else {
        /* if descending:
         *
         * Because hardware calculates sweep with:
         * timer -= timer >> shift
         * the timer converges to the value which timer >> shift always results zero. */
        const int shifts = sweep & 7;
        return static_cast<float>((1 << shifts) - 1);
    }
}

uint8_t SquareChannel::sweepTime(uint8_t sweep)
{
    return static_cast<uint8_t>((sweep & 0x70) >> 4);
}

}