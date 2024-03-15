#include "Instrument.h"

#include <assert.h>
#include <algorithm>

namespace GSVST {

#define BPM_PER_FRAME 150

Instrument::VolumeFade Instrument::getVol() const
{
    float envBase = float(envLevelPrev);
    float envDelta = (float(envLevelCur) - envBase) / float(INTERFRAMES);
    float finalFromEnv = envBase + envDelta * float(envInterStep);
    float finalToEnv = envBase + envDelta * float(envInterStep + 1);

    VolumeFade retval;
    retval.fromVolLeft = float(leftVolPrev) * finalFromEnv * (1.0f / 65536.0f);
    retval.fromVolRight = float(rightVolPrev) * finalFromEnv * (1.0f / 65536.0f);
    retval.toVolLeft = float(leftVolCur) * finalToEnv * (1.0f / 65536.0f);
    retval.toVolRight = float(rightVolCur) * finalToEnv * (1.0f / 65536.0f);
    return retval;
}

void Instrument::processCommon(sample* buffer, size_t numSamples, const MixingArgs& args)
{
    if (isDead())
        return;

    if (numSamples == 0)
        return;

    process(buffer, numSamples, args);

    updateVolFade();
}

void Instrument::processStart(const MixingArgs& args)
{
    if (envSampleCount == 0)
    {
        stepEnvelope();
        updateArgs(args);
        updateBPMStack();
    }
}

void Instrument::processEnd(const MixingArgs& args)
{
    updateIncrements(args);
}

void Instrument::setVol(uint8_t in_vol)
{
    vol = in_vol;
    updateVolAndPan();
}

void Instrument::setPan(int8_t in_pan)
{
    pan = in_pan;
    updateVolAndPan();
}

void Instrument::updateVolAndPan()
{
    if (!stop) {
        int combinedPan = std::clamp(pan + note.rhythmPan, -64, +63);
        this->leftVolCur = uint8_t(note.velocity * vol * (-combinedPan + 64) / 8192);
        this->rightVolCur = uint8_t(note.velocity * vol * (combinedPan + 64) / 8192);
    }
}

void Instrument::updateADSR(const ADSR& in_adsr)
{
    env.att = in_adsr.att;
    env.dec = in_adsr.dec;
    env.sus = in_adsr.sus;
    env.rel = in_adsr.rel;
}

void Instrument::release()
{
    stop = true;
}

bool Instrument::isStopping() const
{
    return stop;
}

void Instrument::kill()
{
    envState = EnvState::DEAD;
    envInterStep = 0;
}

void Instrument::tickLfo()
{
    if (lfoSpeed != 0 && modWheel != 0)
    {
        lfoPhase += lfoSpeed;
        int lfoPoint;
        if (static_cast<int8_t>(lfoPhase - 64) >= 0)
            lfoPoint = 128 - lfoPhase;
        else
            lfoPoint = static_cast<int8_t>(lfoPhase);
        lfoPoint *= modWheel;
        lfoPoint >>= 6;

        if (lfoValue != lfoPoint)
        {
            lfoValue = static_cast<int8_t>(lfoPoint);
            updatePitch();
        }
    }
}

void Instrument::setLfo(uint8_t speed, uint8_t value)
{
    lfoSpeed = speed;
    modWheel = value;
    updatePitch();
}

void Instrument::setPitchWheel(int16_t pitch)
{
    pitchWheel = pitch;
    updatePitch();
}

void Instrument::setPitchBendRange(int16_t range)
{
    pitchBendRange = range;
    updatePitch();
}

void Instrument::setTune(int16_t in_tune)
{
    tune = in_tune;
    updatePitch();
}

void Instrument::updatePitch()
{
    auto pitch = tune + pitchWheel * pitchBendRange + (lfoValue * 4);
    freq = getMidCFreq() * powf(2.0f, float(getMidiKeyPitch() - 60) * (1.0f / 12.0f) + float(pitch) * (1.0f / 768.0f));
}

void Instrument::stepEnvelope()
{
    if (envState == EnvState::INIT) {
        if (stop) {
            envState = EnvState::DEAD;
            return;
        }
        /* it's important to initialize the volume ramp here because in the constructor
         * the initial volume is not yet known (i.e. 0) */
        updateVolFade();

        /* Because we are smoothly fading all our amplitude changes, we avoid the case
         * where the fastest attack value will still cause a 16.6ms ramp instead of being
         * instant maximum amplitude. */
        if (env.att == 0xFF)
            envLevelPrev = 0xFF;
        else
            envLevelPrev = 0x0;

        envLevelCur = 0;
        envInterStep = 0;
        envState = EnvState::ATK;
    }
    else {
        /* On GBA, envelopes update every frame but because we do a multiple of updates per frame
         * (to increase timing accuracy of Note ONs), only every so many sub-frames we actually update
         * the envelope state. */
        if (++envInterStep < INTERFRAMES)
            return;
        envLevelPrev = envLevelCur;
        envInterStep = 0;
    }

    if (envState == EnvState::PSEUDO_ECHO) {
        assert(note.pseudoEchoLen != 0);
        if (--note.pseudoEchoLen == 0) {
            envState = EnvState::DIE;
            envLevelCur = 0;
        }
    }
    else if (stop) {
        if (envState == EnvState::DIE) {
            /* This is really just a transitional state that is supposed to be the last GBA frame
             * of fadeout. Because we smoothly ramp out envelopes, out envelopes are actually one
             * frame longer than on hardware- As soon as this state is reached the channel is disabled */
            envState = EnvState::DEAD;
        }
        else {
            envLevelCur = static_cast<uint8_t>((envLevelCur * env.rel) >> 8);
            /* ORIGINAL "BUG":
             * Even when pseudo echo has no length, the following condition will kick in and may cause
             * an earlier then intended note release */
            if (envLevelCur <= note.pseudoEchoVol) {
            release:
                if (note.pseudoEchoVol == 0 || note.pseudoEchoLen == 0) {
                    envState = EnvState::DIE;
                    envLevelCur = 0;
                }
                else {
                    envState = EnvState::PSEUDO_ECHO;
                    envLevelCur = note.pseudoEchoVol;
                }
            }
        }
    }
    else {
        if (envState == EnvState::DEC) {
            envLevelCur = static_cast<uint8_t>((envLevelCur * env.dec) >> 8);
            if (envLevelCur <= env.sus) {
                envLevelCur = env.sus;
                if (envLevelCur == 0)
                    goto release;
                envState = EnvState::SUS;
            }
        }
        else if (envState == EnvState::ATK) {
            uint32_t newLevel = envLevelCur + env.att;
            if (newLevel >= 0xFF) {
                envLevelCur = 0xFF;
                envState = EnvState::DEC;
            }
            else {
                envLevelCur = static_cast<uint8_t>(newLevel);
            }
        }
    }
}

void Instrument::updateVolFade()
{
    leftVolPrev = leftVolCur;
    rightVolPrev = rightVolCur;
}


void Instrument::updateIncrements(const MixingArgs& args)
{
    envSampleCount++;
    if (envSampleCount == args.samplesPerBufferForComputation)
    {
        envSampleCount = 0;
    }
}

void Instrument::updateBPMStack()
{
    const auto speedFactor = 1.0f;

    bpmStack += uint32_t(bpmRefresh * speedFactor);
    if (bpmStack >= BPM_PER_FRAME * INTERFRAMES)
    {
        tickLfo();
        bpmStack -= BPM_PER_FRAME * INTERFRAMES;
    }
}

void Instrument::updateArgs(const MixingArgs& args)
{
    VolumeFade volFade = getVol();
    volFade.fromVolLeft *= args.vol;
    volFade.fromVolRight *= args.vol;
    volFade.toVolLeft *= args.vol;
    volFade.toVolRight *= args.vol;

    cargs.lVolStep = (volFade.toVolLeft - volFade.fromVolLeft) * args.samplesPerBufferInv;
    cargs.rVolStep = (volFade.toVolRight - volFade.fromVolRight) * args.samplesPerBufferInv;
    cargs.lVol = volFade.fromVolLeft;
    cargs.rVol = volFade.fromVolRight;
}

}