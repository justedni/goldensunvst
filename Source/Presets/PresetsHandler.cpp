#include "PresetsHandler.h"
#include "Presets.h"
#include "CGBSynthPresets.h"

#include "Processor/SampleInstrument.h"
#include "Processor/CGBChannel.h"

#include "GS/GSPresets.h"

#define TSF_IMPLEMENTATION
#include "External/tinysoundfont/tsf.h"

#include <map>
#include <cmath>
#include <algorithm>
#include <assert.h>

#include <JuceHeader.h>

namespace GSVST {

PresetsHandler::PresetsHandler()
{
    m_formatManager.reset(new juce::AudioFormatManager());
    m_formatManager->registerBasicFormats();
}

PresetsHandler::~PresetsHandler()
{
    clear();
}

void PresetsHandler::sort()
{
    std::sort(m_presets.begin(), m_presets.end(), [](auto& l, auto& r) { return l->programid < r->programid; });
}

const ProgramInfo* PresetsHandler::findProgramInfo(const std::list<ProgramInfo>& list, int bankid, int programid) const
{
    const auto& it = std::find_if(list.begin(), list.end(), [&](const auto& e) {
        return e.bankid == bankid && e.programid == programid; });

    if (it != list.end())
        return &(*it);

    return nullptr;
}

const std::list<ProgramInfo>& PresetsHandler::getProgramInfo() const
{
    switch (m_programNameMode)
    {
    default:
    case EProgramNameMode::Sf2:
    {
        return m_emptyList;
    }
    case EProgramNameMode::GS:
    {
        return getCustomProgramList();
    }
    }
}

void PresetsHandler::setSoundfont(const std::string& path)
{
    soundFontPath = path;

    clearPresetsOfType(EPresetType::Synth);
    clearPresetsOfType(EPresetType::Soundfont);

    if (juce::File(path).exists())
    {
        addSoundFontPresets();
        sort();
    }
    else
    {
        addSynthsPresets();
    }
}

float* PresetsHandler::getSoundfontBuffer(unsigned int offset) const
{
    return &(soundFont->fontSamples[offset]);
}

void PresetsHandler::setAutoReplaceGSSynths(bool bEnable)
{
    if (m_bAutoReplaceGSSynthsEnabled != bEnable)
    {
        m_bAutoReplaceGSSynthsEnabled = bEnable;

        if (!bEnable)
            clearPresetsOfType(EPresetType::Synth);

        addSoundFontPresets();
        sort();
    }
}

void PresetsHandler::setAutoReplaceGBSynths(bool bEnable)
{
    if (m_bAutoReplaceGBSynthsEnabled != bEnable)
    {
        m_bAutoReplaceGBSynthsEnabled = bEnable;

        addSoundFontPresets();
        sort();
    }
}

void PresetsHandler::setProgramNameMode(EProgramNameMode mode)
{
    if (m_programNameMode != mode)
    {
        m_programNameMode = mode;

        addSoundFontPresets();
        sort();
    }
}

void PresetsHandler::clear()
{
    for (auto* preset : m_presets)
    {
        delete preset;
    }

    m_presets.clear();
}

void PresetsHandler::clearPresetsOfType(EPresetType type)
{
    for (auto it = m_presets.begin(); it != m_presets.end(); )
    {
        auto* preset = (*it);
        if (preset->type == type)
        {
            delete preset;
            it = m_presets.erase(it);
        }
        else
            ++it;
    }
}

void PresetsHandler::addSamplePreset(int bankid, int programid, std::string&& name, std::string&& filepath, ADSR&& adsr,
    int pitch_correction, int original_pitch, int sample_rate)
{
    auto* newPreset = new SamplePreset(bankid, programid, std::move(name), std::move(filepath), std::move(adsr), SampleInfo(calculateMidCFreq(pitch_correction, original_pitch, sample_rate)));
    if (newPreset->loadFile(*m_formatManager.get()))
    {
        m_presets.push_back(newPreset);
    }
    else
    {
        delete newPreset;
    }
}

void PresetsHandler::addSamplePresetLooping(int bankid, int programid, std::string&& name, std::string&& filepath, ADSR&& adsr,
    int pitch_correction, int original_pitch, int sample_rate, uint32_t in_loopPos, uint32_t in_endPos)
{
    auto* newPreset = new SamplePreset(bankid, programid, std::move(name), std::move(filepath), std::move(adsr),
        SampleInfo(calculateMidCFreq(pitch_correction, original_pitch, sample_rate), true, in_loopPos, in_endPos));

    if (newPreset->loadFile(*m_formatManager.get()))
    {
        m_presets.push_back(newPreset);
    }
    else
    {
        delete newPreset;
    }
}

bool PresetsHandler::isGSSynth(const std::string& presetName)
{
    if (presetName.find("Square @0x") != std::string::npos
        || presetName.find("Saw @0x") != std::string::npos
        || presetName.find("Triangle @0x") != std::string::npos)
        return true;

    return false;
}

bool PresetsHandler::isGBSynth(const std::string& presetName)
{
    if (presetName.find("square ") != std::string::npos)
        return true;

    return false;
}

bool PresetsHandler::isSynth(const tsf_preset& preset)
{
    if (preset.regionNum == 0)
        return false;

    auto& region = preset.regions[0];

    if (isGBSynth(region.sampleName) || isGSSynth(region.sampleName))
        return true;

    return false;
}

ADSR PresetsHandler::getSoundfontADSR(const tsf_region& region)
{
    auto& adsr = region.ampenv;

    if (isGBSynth(region.sampleName))
    {
        // GB instrument ADSR
        uint8_t attack = 15;
        uint8_t decay = 0;
        uint8_t sustain = 15;
        uint8_t release = 0;

        if (adsr.attack > 0.0f)
        {
            attack = static_cast<uint8_t>(std::round((adsr.attack * 5.0)));
        }

        if (adsr.decay > 0.0f)
        {
            decay = static_cast<uint8_t>(std::round(((adsr.decay - 1) * 5.0)));
        }

        if (adsr.sustain < 1.0f)
        {
            auto writtenSf2Val = std::round(log10(adsr.sustain) / -0.005f);
            sustain = static_cast<uint8_t>(15.0 / exp(writtenSf2Val / 100.0));
        }

        if (adsr.release > 0.0f)
        {
            release = static_cast<uint8_t>(std::round((adsr.release * 5.0)));
        }

        return ADSR(attack, decay, sustain, release);
    }
    else // Sample ADSR
    {
        uint8_t attack = 255;
        uint8_t decay = 0;
        uint8_t sustain = 255;
        uint8_t release = 0;

        if (adsr.attack > 0.0f)
        {
            attack = static_cast<uint8_t>(std::round(((256 / 60.0) / adsr.attack)));
        }

        if (adsr.decay > 0.0f)
        {
            decay = static_cast<uint8_t>(exp(log(256) - (log(256.0) / (adsr.decay * log(256) * 6.0))));
        }

        if (adsr.sustain < 1.0f)
        {
            auto writtenSf2Val = std::round(log10(adsr.sustain) / -0.005f);
            sustain = static_cast<uint8_t>(256.0 / exp(writtenSf2Val / 100.0));
        }

        if (adsr.release > 0.0f)
        {
            release = static_cast<uint8_t>(std::floor(exp(log(256) - (log(256.0) / 60.0 / adsr.release))));
        }

        return ADSR(attack, decay, sustain, release);
    }
}

void PresetsHandler::addSoundFontPresets()
{
    if (soundFontPath.empty())
        return;

    clearPresetsOfType(EPresetType::Soundfont);
    clearPresetsOfType(EPresetType::Synth);

    cleanupSoundfont();
    soundFont = tsf_load_filename(soundFontPath.c_str());

    if (!soundFont)
        return;

    for (int i = 0; i < tsf_get_presetcount(soundFont); i++)
    {
        const auto& preset = soundFont->presets[i];
        auto bankId = preset.bank;
        auto presetId = preset.preset;
        auto name = tsf_get_presetname(soundFont, i);

        if (std::find_if(m_presets.begin(), m_presets.end(), [&](auto* p) { return p->programid == presetId && p->bankid == bankId; }) != m_presets.end())
            continue; // Don't add presets if there's already a Sample or Synth version of it

        const auto& friendlyNames = getProgramInfo();
        const auto* foundFriendlyName = findProgramInfo(friendlyNames, bankId, presetId);
        if (!friendlyNames.empty() && !foundFriendlyName)
            continue;

        {
            std::string friendlyName = (foundFriendlyName ? foundFriendlyName->name : "");
            auto* newPreset = buildSoundfontPreset(preset, name, friendlyName);
            assert(newPreset);
            if (newPreset)
            {
                m_presets.push_back(newPreset);
            }
        }
    }
}

Preset* PresetsHandler::buildSoundfontPreset(const tsf_preset& preset, const std::string& name, const std::string& friendlyName)
{
    Preset* newPreset = nullptr;

    bool bIsEveryKeySplit = (std::string(name).find("Type 128") != std::string::npos);
    auto bankId = preset.bank;
    auto presetId = preset.preset;

    if (isSynth(preset) && !bIsEveryKeySplit && preset.regionNum > 0)
    {
        auto firstAdsr = getSoundfontADSR(preset.regions[0]);
        auto synthName = std::string(preset.regions[0].sampleName);

        if (m_bAutoReplaceGSSynthsEnabled && isGSSynth(synthName))
        {
            if (auto* synthPreset = buildCustomSynthPreset(presetId, synthName, firstAdsr))
                newPreset = synthPreset;
        }
        else if (m_bAutoReplaceGBSynthsEnabled && isGBSynth(synthName))
        {
            auto dutyCycle = WaveDuty::D12;
            if (synthName.find("square 12.5%") != std::string::npos)
                dutyCycle = WaveDuty::D12;
            else if (synthName.find("square 25%") != std::string::npos)
                dutyCycle = WaveDuty::D25;
            else if (synthName.find("square 50%") != std::string::npos)
                dutyCycle = WaveDuty::D50;
            else if (synthName.find("square 75%") != std::string::npos) // Requires a modified version of GBA Mus Ripper
                dutyCycle = WaveDuty::D75;

            newPreset = new SquareSynthPreset(bankId, presetId, std::move(synthName), std::move(firstAdsr), dutyCycle);
        }
    }

    if (!newPreset)
    {
        std::vector<SoundfontSampleInfo> samples;

        for (int j = 0; j < preset.regionNum; j++)
        {
            auto& region = preset.regions[j];

            if (region.hivel == 0)
                continue;

            bool bLoopEnabled = (region.loop_mode == 1);
            auto loopStart = (bLoopEnabled ? region.loop_start - region.offset : 0);
            auto loopEnd = (bLoopEnabled ? (region.loop_end - region.offset) + 1 : region.end - region.offset);
            auto offset = region.offset;

            bool fixed = (region.pitch_keytrack == 0);

            // Calculate mid-c freq
            int calculated_mid_c = calculateMidCFreq(region.tune, region.pitch_keycenter, region.sample_rate);

            int8_t rhythmPan = 0;

            if (bIsEveryKeySplit && region.pan)
            {
                rhythmPan = static_cast<int8_t>(std::round((region.pan * 256)));
            }

            auto sampleInfo = SoundfontSampleInfo(calculated_mid_c, fixed, region.sample_rate, bLoopEnabled, loopStart, loopEnd);
            sampleInfo.adsr = getSoundfontADSR(region);
            sampleInfo.offset = offset;
            sampleInfo.keyRange = { region.lokey, region.hikey };
            sampleInfo.rhythmPan = rhythmPan;
            sampleInfo.notePitch = static_cast<uint8_t>(region.pitch_keycenter);

            samples.push_back(sampleInfo);
        }

        const auto& label = !friendlyName.empty() ? friendlyName : name;
        newPreset = new SoundfontPreset(bankId, presetId, std::string(label), *this, std::move(samples));
    }

    return newPreset;
}

void PresetsHandler::cleanupSoundfont()
{
    if (soundFont)
    {
        tsf_close(const_cast<tsf*>(soundFont));
        //deletion handled by tsf
        soundFont = nullptr;
    }
}

int PresetsHandler::calculateMidCFreq(int pitch_correction, int original_pitch, int sample_rate)
{
    int calculated_mid_c = 22050;

    int int_delta_note = original_pitch - 60;
    float delta_note = int_delta_note - (pitch_correction / 100.0f);

    calculated_mid_c = static_cast<int>(std::floor(sample_rate / pow(2, delta_note / 12)));

    return calculated_mid_c;
}

}
