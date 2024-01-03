#include "PresetsHandler.h"
#include "Presets.h"

#include "Processor/SampleInstrument.h"

#define TSF_IMPLEMENTATION
#include "External/tinysoundfont/tsf.h"

#include <map>
#include <cmath>
#include <algorithm>

#include <JuceHeader.h>

namespace GSVST {

PresetsHandler::~PresetsHandler()
{
    clear();
}

void PresetsHandler::sort()
{
    std::sort(m_presets.begin(), m_presets.end(), [](auto& l, auto& r) { return l->id < r->id; });
}

void PresetsHandler::setSoundfont(const std::string& path)
{
    soundFontPath = path;

    if (juce::File(path).exists())
    {
        addSoundFontPresets();
        sort();
    }
    else
    {
        clearSoundfontPresets();
    }
}

float* PresetsHandler::getSoundfontBuffer(unsigned int offset) const
{
    return &(soundFont->fontSamples[offset]);
}

void PresetsHandler::setEnableGSMode(bool bEnable)
{
    if (m_bGSModeEnabled != bEnable)
    {
        m_bGSModeEnabled = bEnable;

        if (bEnable)
        {
            addSynthsPresets();
            sort();
        }
        else
        {
            clearSynthPresets();
        }
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

void PresetsHandler::clearSynthPresets()
{
    for (auto it = m_presets.begin(); it != m_presets.end(); )
    {
        auto* preset = (*it);
        if (preset->type == EPresetType::Synth || preset->type == EPresetType::PWMSynth)
        {
            delete preset;
            it = m_presets.erase(it);
        }
        else
            ++it;
    }
}

void PresetsHandler::clearSoundfontPresets()
{
    for (auto it = m_presets.begin(); it != m_presets.end(); )
    {
        auto* preset = (*it);
        if (preset->type == EPresetType::Soundfont)
        {
            delete preset;
            it = m_presets.erase(it);
        }
        else
            ++it;
    }
}

void PresetsHandler::addSamplePreset(int id, std::string&& name, std::string&& filepath, ADSR&& adsr, int pitch_correction, int original_pitch, int sample_rate)
{
    auto* newPreset = new SamplePreset(id, std::move(name), std::move(filepath), std::move(adsr), SampleInfo(calculateMidCFreq(pitch_correction, original_pitch, sample_rate)));
    if (newPreset->loadFile())
    {
        m_presets.push_back(newPreset);
    }
    else
    {
        delete newPreset;
    }
}

void PresetsHandler::addSoundFontPresets()
{
    if (soundFontPath.empty())
        return;

    clearSoundfontPresets();

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

        if (std::find_if(m_presets.begin(), m_presets.end(), [&](auto* p) { return p->id == presetId; }) != m_presets.end())
            continue; // Don't add presets if there's already a Sample or Synth version of it

        bool bIsEveryKeySplit = (std::string(name).find("Type 128") != std::string::npos);

        const auto& friendlyNames = getHandledProgramsList();
        const auto& friendlyName = friendlyNames.find(presetId);
        if (friendlyName == friendlyNames.end())
            continue;

        if (bankId == 0)
        {
            std::string label;
            if (presetId < 10 ) label.append("00");
            else if (presetId < 100) label.append("0");
            label.append(std::to_string(presetId));
            label.append(" ");
            label.append(friendlyName->second.name);

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

                auto& adsr = region.ampenv;

                uint8_t attack = 255;
                uint8_t decay = 0;
                uint8_t sustain = 255;
                uint8_t release = 255;

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

                if (bIsEveryKeySplit && region.pan)
                {
                    rhythmPan = static_cast<int8_t>(std::round((region.pan * 256)));
                }

                auto sampleInfo = SoundfontSampleInfo(calculated_mid_c, fixed, region.sample_rate, bLoopEnabled, loopStart, loopEnd);
                sampleInfo.adsr = ADSR(attack, decay, sustain, release);
                sampleInfo.offset = offset;
                sampleInfo.keyRange = { region.lokey, region.hikey };
                sampleInfo.rhythmPan = rhythmPan;
                sampleInfo.notePitch = static_cast<uint8_t>(region.pitch_keycenter);

                samples.push_back(sampleInfo);
            }

            m_presets.push_back(new SoundfontPreset(presetId, std::move(label), *this, std::move(samples)));
        }
    }
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