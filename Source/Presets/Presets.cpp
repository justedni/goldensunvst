#include "Presets.h"
#include "PresetsHandler.h"

#include "Processor/SampleInstrument.h"
#include "GS/GSSynths.h"

#include <map>
#include <cmath>
#include <algorithm>
#include <assert.h>

#include <JuceHeader.h>

namespace GSVST {

const ADSR SoundfontPreset::m_emptyADSR;

std::string EnumToString_EPresetType(EPresetType type)
{
    switch (type)
    {
    case EPresetType::Sample: return "Sample";
    case EPresetType::Soundfont: return "Sf2";
    case EPresetType::Synth: return "Synth";
    }

    return "";
}

//-----------------------------------------------------------------------------
Instrument* SynthPreset::createPlayingInstance(const Note& note) const
{
    assert(synthType == EDSPType::Saw || synthType == EDSPType::Tri);
    return GSSynth::createSynth(synthType, note);
}

//-----------------------------------------------------------------------------
Instrument* PWMSynthPreset::createPlayingInstance(const Note& note) const
{
    return GSPWMSynth::createPWMSynth(pwmdata, note);
}

//-----------------------------------------------------------------------------
SamplePreset::SamplePreset(int in_id, std::string&& in_name, std::string&& in_filepath, ADSR&& in_adsr, SampleInfo&& in_info)
    : Preset(in_id, EPresetType::Sample, std::move(in_name))
    , m_filepath(std::move(in_filepath))
    , m_info(std::move(in_info))
{
    m_info.adsr = std::move(in_adsr);
}


Instrument* SamplePreset::createPlayingInstance(const Note& note) const
{
    auto* sampleInfo = new SampleInfo(m_info);
    sampleInfo->numChannels = m_numChannels;
    sampleInfo->sampleBuffer = m_audioBuffer.getArrayOfReadPointers();

    if (!sampleInfo->loopEnabled)
    {
        sampleInfo->endPos = m_lengthInSamples;
    }

    return new SampleInstrument(std::move(sampleInfo), note);
}

bool SamplePreset::loadFile(juce::AudioFormatManager& formatManager)
{
    auto file = juce::File(m_filepath);
    if (!file.exists())
        return false;

    auto* reader = formatManager.createReaderFor(file);
    m_audioBuffer.setSize(reader->numChannels, reader->lengthInSamples);
    reader->read(&m_audioBuffer, 0, reader->lengthInSamples, 0, true, true);
    m_numChannels = reader->numChannels;
    m_lengthInSamples = reader->lengthInSamples;

    delete reader;

    return true;
}

//-----------------------------------------------------------------------------
SampleMultiPreset::SampleMultiPreset(int in_id, std::string&& in_name, ADSR&& in_adsr, std::vector<MultiSample>&& in_info)
    : Preset(in_id, EPresetType::Sample, std::move(in_name))
    , samples(std::move(in_info))
    , adsr(std::move(in_adsr))

{
}

Instrument* SampleMultiPreset::createPlayingInstance(const Note& note) const
{
    auto noteNum = note.midiKeyPitch;
    auto found = std::find_if(samples.begin(), samples.end(), [noteNum](auto& s) { return noteNum >= s.keyRange.first && noteNum <= s.keyRange.second; });
    if (found == samples.end())
        return nullptr;

    auto* sample = &(*found);

    auto* sampleInfo = new SampleInfo(sample->info);
    sampleInfo->numChannels = sample->numChannels;
    sampleInfo->sampleBuffer = sample->audioBuffer.getArrayOfReadPointers();

    sampleInfo->loopEnabled = true;
    sampleInfo->loopPos = sample->loopStart;
    sampleInfo->endPos = sample->loopEnd;

    return new SampleInstrument(std::move(sampleInfo), note);
}

void SampleMultiPreset::getLoopTimesFromFile(juce::AudioFormatManager& formatManager, std::string filePath, int& loopStart, int& loopEnd)
{
    auto file = juce::File(filePath);
    if (!file.exists())
        return;

    auto* reader = formatManager.createReaderFor(file);

    juce::String defaultVal = "";
    auto startVal = reader->metadataValues.getValue("Cue0Offset", defaultVal);
    auto endVal = reader->metadataValues.getValue("Cue1Offset", defaultVal);

    loopStart = std::stoi(std::string(startVal.getCharPointer()));
    loopEnd = std::stoi(std::string(endVal.getCharPointer()));

    delete reader;
}

bool SampleMultiPreset::loadFiles(juce::AudioFormatManager& formatManager)
{
    for (auto& sample : samples)
    {
        auto file = juce::File(sample.filePath);
        if (!file.exists())
            continue;

        auto* reader = formatManager.createReaderFor(file);
        sample.audioBuffer.setSize(reader->numChannels, reader->lengthInSamples);
        reader->read(&sample.audioBuffer, 0, reader->lengthInSamples, 0, true, true);
        sample.numChannels = reader->numChannels;
        sample.lengthInSamples = reader->lengthInSamples;

        juce::String defaultVal = "";
        auto startVal = reader->metadataValues.getValue("Cue0Offset", defaultVal);
        auto endVal = reader->metadataValues.getValue("Cue1Offset", defaultVal);

        sample.loopStart = std::stoi(std::string(startVal.getCharPointer()));
        sample.loopEnd = std::stoi(std::string(endVal.getCharPointer()));

        delete reader;
    }

    return true;
}

//-----------------------------------------------------------------------------
Instrument* SoundfontPreset::createPlayingInstance(const Note& note) const
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
