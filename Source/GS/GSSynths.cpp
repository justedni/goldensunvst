#include "GSSynths.h"

#include <assert.h>

namespace GSVST {

GSSynth* GSSynth::createSynth(EDSPType type, const Note& in_note)
{
    switch (type)
    {
    case EDSPType::PCM:
    case EDSPType::PCMFixed:
    case EDSPType::ModPulse:
    default:
        assert(false);
        break;
    case EDSPType::Saw:
        return new GSSawSynth(in_note);
    case EDSPType::Tri:
        return new GSTriangleSynth(in_note);
    }

    return nullptr;
}

GSPWMSynth* GSPWMSynth::createPWMSynth(const PWMData& pwmdata, const Note& in_note)
{
    return new GSPWMSynth(pwmdata, in_note);
}

void GSSynth::updateArgs(const MixingArgs& args)
{
    Instrument::updateArgs(args);

    cargs.interStep = freq * args.sampleRateInv;
    cargs.interStep /= 64.f; // different scale for GS synths
}

void GSPWMSynth::calculateModPulseThreshold(float nBlocksReciprocal)
{
    uint32_t fromPos;

    if (envInterStep == 0)
        fromPos = pos += (m_data.dutyStep << 24);
    else
        fromPos = pos;

    uint32_t toPos = fromPos + (m_data.dutyStep << 24);

    auto calcThresh = [](uint32_t val, const auto& data) {
        uint32_t iThreshold = uint32_t(data.initDuty << 24) + val;
        iThreshold = int32_t(iThreshold) < 0 ? ~iThreshold >> 8 : iThreshold >> 8;
        iThreshold = iThreshold * data.depth + uint32_t(data.dutyBase << 24);
        return float(iThreshold) / float(0x100000000);
    };

    float fromThresh = calcThresh(fromPos, m_data);
    float toThresh = calcThresh(toPos, m_data);

    m_deltaThresh = toThresh - fromThresh;
    m_baseThresh = fromThresh + (m_deltaThresh * (float(envInterStep) * (1.0f / float(INTERFRAMES))));

    m_threshStep = m_deltaThresh * (1.0f / float(INTERFRAMES)) * nBlocksReciprocal;
    m_fThreshold = m_baseThresh;
}

void GSPWMSynth::processStart(const MixingArgs& args)
{
    if (envSampleCount == 0)
    {
        stepEnvelope();
        updateArgs(args);
        calculateModPulseThreshold(args.samplesPerBufferInv);
        updateBPMStack();
    }
}

void GSPWMSynth::process(sample* buffer, size_t numSamples, const MixingArgs& args)
{
    do {
        processStart(args);

        float baseSamp = interPos < m_fThreshold ? 0.5f : -0.5f;
        // correct dc offset
        baseSamp += 0.5f - m_fThreshold;
        m_fThreshold += m_threshStep;
        buffer->left += baseSamp * cargs.lVol;
        buffer->right += baseSamp * cargs.rVol;
        buffer++;

        cargs.lVol += cargs.lVolStep;
        cargs.rVol += cargs.rVolStep;

        interPos += cargs.interStep;
        // this below might glitch for too high frequencies, which usually shouldn't be used anyway
        if (interPos >= 1.0f) interPos -= 1.0f;

        processEnd(args);
    } while (--numSamples > 0);
}

void GSSawSynth::process(sample* buffer, size_t numSamples, const MixingArgs& args)
{
    const uint32_t fix = 0x70;

    do {
        processStart(args);

        /*
         * Sorry that the baseSamp calculation looks ugly.
         * For accuracy it's a 1 to 1 translation of the original assembly code
         * Could probably be reimplemented easier. Not sure if it's a perfect saw wave
         */
        interPos += cargs.interStep;
        if (interPos >= 1.0f) interPos -= 1.0f;
        uint32_t var1 = uint32_t(interPos * 256) - fix;
        uint32_t var2 = uint32_t(interPos * 65536.0f) << 17;
        uint32_t var3 = var1 - (var2 >> 27);
        pos = var3 + uint32_t(int32_t(pos) >> 1);

        float baseSamp = float((int32_t)pos) / 256.0f;

        buffer->left += baseSamp * cargs.lVol;
        buffer->right += baseSamp * cargs.rVol;
        buffer++;

        cargs.lVol += cargs.lVolStep;
        cargs.rVol += cargs.rVolStep;

        processEnd(args);
    } while (--numSamples > 0);
}

void GSTriangleSynth::process(sample* buffer, size_t numSamples, const MixingArgs& args)
{
    do {
        processStart(args);

        interPos += cargs.interStep;
        if (interPos >= 1.0f) interPos -= 1.0f;
        float baseSamp;
        if (interPos < 0.5f) {
            baseSamp = (4.0f * interPos) - 1.0f;
        }
        else {
            baseSamp = 3.0f - (4.0f * interPos);
        }

        buffer->left += baseSamp * cargs.lVol;
        buffer->right += baseSamp * cargs.rVol;
        buffer++;

        cargs.lVol += cargs.lVolStep;
        cargs.rVol += cargs.rVolStep;

        processEnd(args);
    } while (--numSamples > 0);
}

}
