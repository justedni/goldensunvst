#include "SampleInstrument.h"

namespace GSVST {

SoundfontSampleInstrument::SoundfontSampleInstrument(SoundfontSampleInfo* in_info, const Note& in_note)
    : SampleInstrument(std::move(in_info), in_note)
    , m_fixed(in_info->fixed)
    , m_fixedModeRate(in_info->fixedSampleRate)
{
}

void SoundfontSampleInstrument::updateArgs(const MixingArgs& args)
{
    Instrument::updateArgs(args);

    if (m_fixed)
        cargs.interStep = float(m_fixedModeRate) * args.sampleRateInv;
    else
        cargs.interStep = freq * args.sampleRateInv;
}


SampleInstrument::SampleInstrument(SampleInfo* in_info, const Note& in_note)
    : Instrument(in_note)
    , m_inNumChannels(in_info->numChannels)
    , m_info(std::move(in_info))
{
    m_resampler = std::make_unique<LinearResampler>();
}

void SampleInstrument::updateArgs(const MixingArgs& args)
{
    Instrument::updateArgs(args);

    cargs.interStep = freq * args.sampleRateInv;
}

void SampleInstrument::process(sample* buffer, size_t numSamples, const MixingArgs& args)
{
    int debugSampleCount = 0;

    if (!cargs.bInitialized)
    {
        // First init
        updateArgs(args);
        cargs.bInitialized = true;
    }

    if (numSamples == 0)
        return;
    sample* outBuffer = new sample[numSamples];

    bool running = m_resampler->Process(outBuffer, numSamples, cargs.interStep, sampleFetchCallback, this);

    size_t i = 0;
    do {
        processStart(args);

        buffer->left += outBuffer[i].left * cargs.lVol;
        buffer->right += outBuffer[i].right * cargs.rVol;
        i++;
        buffer++;
        cargs.lVol += cargs.lVolStep;
        cargs.rVol += cargs.rVolStep;

        debugSampleCount++;
        processEnd(args);
    } while (--numSamples > 0);

    if (!running)
        kill();

    delete[] outBuffer;
}

bool SampleInstrument::sampleFetchCallback(std::vector<sample>& fetchBuffer, size_t samplesRequired, void* cbdata)
{
    if (fetchBuffer.size() >= samplesRequired)
        return true;
    SampleInstrument* _this = static_cast<SampleInstrument*>(cbdata);
    size_t samplesToFetch = samplesRequired - fetchBuffer.size();
    size_t i = fetchBuffer.size();
    fetchBuffer.resize(samplesRequired);

    auto* sampleInfo = static_cast<SampleInfo*>(_this->m_info.get());

    do {
        size_t samplesTilLoop = sampleInfo->endPos - _this->pos;
        size_t thisFetch = std::min(samplesTilLoop, samplesToFetch);

        samplesToFetch -= thisFetch;
        do {
            sampleInfo->fillSample(_this->pos, fetchBuffer[i].left, fetchBuffer[i].right);

            _this->pos++;
            i++;
        } while (--thisFetch > 0);

        if (_this->pos >= sampleInfo->endPos)
        {
            if (sampleInfo->loopEnabled) {
                _this->pos = sampleInfo->loopPos;
            }
            else {
                std::fill(fetchBuffer.begin() + i, fetchBuffer.end(), sample());
                return false;
            }
        }
    } while (samplesToFetch > 0);
    return true;
}

}
