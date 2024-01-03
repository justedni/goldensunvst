#include "Presets.h"
#include "PresetsHandler.h"

#include "Processor/SampleInstrument.h"
#include "GS/GSSynths.h"

#include <map>
#include <cmath>
#include <algorithm>

#include <JuceHeader.h>
#include "External/Audiofile/AudioFile.h"

namespace GSVST {

const ADSR SoundfontPreset::m_emptyADSR;

//-----------------------------------------------------------------------------
Instrument* SynthPreset::createPlayingInstance(const Note& note)
{
    assert(synthType == EDSPType::Saw || synthType == EDSPType::Tri);
    return GSSynth::createSynth(synthType, note);
}

//-----------------------------------------------------------------------------
Instrument* PWMSynthPreset::createPlayingInstance(const Note& note)
{
    return GSPWMSynth::createPWMSynth(pwmdata, note);
}

//-----------------------------------------------------------------------------
SamplePreset::SamplePreset(int in_id, std::string&& in_name, std::string&& in_filepath, ADSR&& in_adsr, SampleInfo&& in_info)
    : Preset(in_id, EPresetType::Sample, std::move(in_name))
    , filepath(std::move(in_filepath))
    , info(std::move(in_info))
{
    info.adsr = std::move(in_adsr);
    audioFile.reset(new AudioFile<float>());
}


Instrument* SamplePreset::createPlayingInstance(const Note& note)
{
    auto* sampleInfo = new SampleInfo(info);
    sampleInfo->numChannels = static_cast<uint8_t>(audioFile->getNumChannels());
    sampleInfo->samplePtr = &audioFile->samples;

    if (!sampleInfo->loopEnabled)
        sampleInfo->endPos = static_cast<uint32_t>(audioFile->samples[0].size());

    return new SampleInstrument(std::move(sampleInfo), note);
}

bool SamplePreset::loadFile()
{
    audioFile->load(filepath);

    if (audioFile->samples.empty() || audioFile->samples[0].empty())
        return false;

    return true;
}

//-----------------------------------------------------------------------------
Instrument* SoundfontPreset::createPlayingInstance(const Note& note)
{
    auto noteNum = note.midiKeyPitch;
    auto found = std::find_if(samples.begin(), samples.end(), [noteNum](auto& s) { return noteNum >= s.keyRange.first && noteNum <= s.keyRange.second; });
    if (found == samples.end())
        return nullptr;

    auto* sample = &(*found);

    auto* sampleInfo = new SoundfontSampleInfo(*sample);
    sampleInfo->numChannels = 1;
    sampleInfo->soundFontSamplePtr = m_presetsHandler.getSoundfontBuffer(sample->offset);

    Note noteToUse = note;
    noteToUse.rhythmPan = sampleInfo->rhythmPan;
    noteToUse.midiKeyPitch = sampleInfo->fixed ? sampleInfo->notePitch : note.midiKeyPitch;

    auto* newInstance = new SoundfontSampleInstrument(std::move(sampleInfo), noteToUse);
    newInstance->useTrackADSR(!sampleInfo->fixed);
    if (sampleInfo->fixed) // Forcing preset ADSR, not the track one
    {
        newInstance->updateADSR(sample->adsr);
    }

    return newInstance;
}

EDSPType SoundfontPreset::getDSPType() const
{
    if (samples.size() > 0)
        return samples[0].fixed ? EDSPType::PCMFixed : EDSPType::PCM;

    return EDSPType::PCM;
}

const ADSR& SoundfontPreset::getADSR() const
{
    return samples.size() > 0 ? samples[0].adsr : m_emptyADSR;
}

}
