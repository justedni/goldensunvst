#include "GSPresets.h"
#include "Presets/Presets.h"

#include <assert.h>

namespace GSVST {

GSPresets::GSPresets()
{
    auto programs = getCustomProgramList();

    for (auto& [id, info] : programs)
    {
        if (info.type == EDSPType::ModPulse)
            m_pwmPresets.push_back(id);
        else if (info.type == EDSPType::Saw)
            m_sawPresets.push_back(id);
        else if (info.type == EDSPType::Tri)
            m_triPresets.push_back(id);
    }

    addSynthsPresets();
    sort();
}

ADSR GSPresets::getADSRInfo(int programId)
{
    switch (programId)
    {
    case 8: // Celesta
    case 24: // Nylon-str. Gt
    case 33: // Rock Bass
    case 45: // Pizz. Str
    case 46: // Harp
    case 47: // Timpani
    case 52: // Choir Aahs
    case 56: // Trumpet
    case 61: // Brass ff
    case 68: // Oboe
    case 72: // Flute 1
    case 73: // Flute 2
    case 75: // Pan Flute
    case 107: // Balaphone
    case 108: // Shout
    case 109: // Bonang
    case 111: // Sitar
    case 112: // Ritual Loop
    case 113: // Daila Loop
        return ADSR(255, 0, 255, 165);
    case 48: // Bright Str
        return ADSR(255, 0, 255, 216);
    case 80: // PWM 1
    case 81: // PWM 2
    case 82: // PWM 3
        return ADSR(85, 165, 193, 178);
    case 83: // Sawtooth
        return ADSR(85, 165, 167, 178);
    case 84: // Triangle 1
        return ADSR(85, 127, 218, 165);
    case 85: // Unused PWM
    case 86: // Unused PWM
    case 87: // Unused PWM
        return ADSR(22, 165, 193, 195);
    case 88: // Unused Saw
        return ADSR(22, 165, 167, 195);
    case 89: // Triangle 2
        return ADSR(22, 127, 218, 195);
    case 90: // PWM 1
    case 91: // PWM 2
    case 92: // Unused PWM
        return ADSR(255, 235, 90, 178);
    case 93: // Sawtooth 2
        return ADSR(255, 235, 103, 178);
    case 94: // Unused Triangle
        return ADSR(255, 235, 103, 165);
    case 95: // Unused PWM
    case 96: // Unused PWM
    case 97: // Unused PWM
    case 98: // Unused Saw
        return ADSR(255, 127, 90, 165);
    case 99: // Unused Triangle
        return ADSR(255, 127, 103, 165);
    case 105: // Music Box
    case 110: // Gender
    case 114: // Steel Drums
        return ADSR(255, 235, 103, 178);
    case 106: // Sitar Gliss
        return ADSR(255, 242, 103, 178);
    case 116: // Verb Lo Tom
        return ADSR(255, 0, 255, 0);
    case 127: // Drum kit
        assert(false); // Please do not use: multiple possible values here
        return ADSR(255, 0, 255, 165);
    }

    return ADSR();
}

void GSPresets::addSynthsPresets()
{
    auto addAll = [&](auto& list)
    {
        for (auto& id : list)
        {
            if (auto* preset = buildGSSynthPreset(static_cast<uint8_t>(id)))
                m_presets.push_back(preset);
        }
    };

    addAll(m_pwmPresets);
    addAll(m_sawPresets);
    addAll(m_triPresets);
}

bool GSPresets::validateGSSynth(unsigned short presetId, const std::string& synthName, const ADSR& adsr)
{
    bool bIsValid = false;

    if (synthName.find("Square @0x") != std::string::npos)
    {
        bIsValid = (std::find(m_pwmPresets.begin(), m_pwmPresets.end(), presetId) != m_pwmPresets.end());
    }
    else if (synthName.find("Saw @0x") != std::string::npos)
    {
        bIsValid = (std::find(m_sawPresets.begin(), m_sawPresets.end(), presetId) != m_sawPresets.end());
    }
    else if (synthName.find("Triangle @0x") != std::string::npos)
    {
        bIsValid = (std::find(m_triPresets.begin(), m_triPresets.end(), presetId) != m_triPresets.end());
    }

    assert(bIsValid);

    if (bIsValid)
    {
        // Checking ADSR
        auto knownAdsr = getADSRInfo(presetId);

        auto almostEqual = [](auto& a, auto& b)
        {
            return (std::abs(a - b) <= 1);
        };

        assert(almostEqual(knownAdsr.att, adsr.att) && almostEqual(knownAdsr.dec, adsr.dec)
            && almostEqual(knownAdsr.sus, adsr.sus) && almostEqual(knownAdsr.rel, adsr.rel));
    }

    return bIsValid;
}

Preset* GSPresets::buildCustomSynthPreset(unsigned short presetId, const std::string& synthName, const ADSR& adsr)
{
    if (!validateGSSynth(presetId, synthName, adsr))
        return nullptr;

    return buildGSSynthPreset(presetId);
}

Preset* GSPresets::buildGSSynthPreset(unsigned short presetId)
{
    Preset* newPreset = nullptr;

    auto adsr = getADSRInfo(presetId);

    static const auto pwmPreset1 = PWMData(128, 16, 240, 224);
    static const auto pwmPreset2 = PWMData(128, 96, 32, 64);
    static const auto pwmPreset3 = PWMData(64, 128, 64, 64);

    switch (presetId)
    {
    case 80: newPreset = new PWMSynthPreset(presetId, "080 PWM Synth 1", std::move(adsr), PWMData(pwmPreset1)); break;
    case 81: newPreset = new PWMSynthPreset(presetId, "081 PWM Synth 2", std::move(adsr), PWMData(pwmPreset2)); break;
    case 82: newPreset = new PWMSynthPreset(presetId, "082 PWM Synth 3", std::move(adsr), PWMData(pwmPreset3)); break;
    case 90: newPreset = new PWMSynthPreset(presetId, "090 PWM Synth 1", std::move(adsr), PWMData(pwmPreset1)); break;
    case 91: newPreset = new PWMSynthPreset(presetId, "091 PWM Synth 2", std::move(adsr), PWMData(pwmPreset2)); break;
    case 92: newPreset = new PWMSynthPreset(presetId, "092 Unused PWM",  std::move(adsr), PWMData(pwmPreset3)); break;

    case 83: newPreset = new SynthPreset(presetId, "083 Sawtooth Synth", EDSPType::Saw, std::move(adsr)); break;
    case 93: newPreset = new SynthPreset(presetId, "093 Sawtooth Synth 2", EDSPType::Saw, std::move(adsr)); break;

    case 84: newPreset = new SynthPreset(presetId, "084 Triangle Synth 1", EDSPType::Tri, std::move(adsr)); break;
    case 89: newPreset = new SynthPreset(presetId, "089 Triangle Synth 2", EDSPType::Tri, std::move(adsr)); break;

    case 85: newPreset = new PWMSynthPreset(presetId, "085 Unused PWM", std::move(adsr), PWMData(pwmPreset1)); break;
    case 86: newPreset = new PWMSynthPreset(presetId, "086 Unused PWM", std::move(adsr), PWMData(pwmPreset2)); break;
    case 87: newPreset = new PWMSynthPreset(presetId, "087 Unused PWM", std::move(adsr), PWMData(pwmPreset3)); break;
    case 88: newPreset = new SynthPreset(presetId, "088 Unused Sawtooth", EDSPType::Saw, std::move(adsr)); break;   
    case 94: newPreset = new SynthPreset(presetId, "094 Unused Triangle", EDSPType::Tri, std::move(adsr)); break;
    case 95: newPreset = new PWMSynthPreset(presetId, "095 Unused PWM", std::move(adsr), PWMData(pwmPreset1)); break;
    case 96: newPreset = new PWMSynthPreset(presetId, "096 Unused PWM", std::move(adsr), PWMData(pwmPreset2)); break;
    case 97: newPreset = new PWMSynthPreset(presetId, "097 Unused PWM", std::move(adsr), PWMData(pwmPreset3)); break;
    case 98: newPreset = new SynthPreset(presetId, "098 Unused Sawtooth", EDSPType::Saw, std::move(adsr)); break;
    case 99: newPreset = new SynthPreset(presetId, "099 Unused Triangle", EDSPType::Tri, std::move(adsr)); break;
    }

    return newPreset;
}

const std::map<int, ProgramInfo>& GSPresets::getCustomProgramList() const
{
    static std::map<int, ProgramInfo> presetsList = {
        { 8,   { "Music Box", "SC-88"} },
        { 24,  { "Nylon-str. Gt", "SC-88"} },
        { 33,  { "Picked Bass", "SC-88"} },
        { 45,  { "Pizz. Str", "SC-88"} },
        { 46,  { "Harp", "SC-88"} },
        { 47,  { "Timpani", "SC-88"} },
        { 48,  { "Bright Str", "SC-88"} },
        { 52,  { "Choir Aahs", "SC-88"} },
        { 56,  { "Trumpet", "SC-88"} },
        { 61,  { "Brass ff", "SC-88"} },
        { 68,  { "Oboe", "SC-88"} },
        { 72,  { "Flute 1", "SC-88"} },
        { 73,  { "Flute 2", "SC-88"} },
        { 75,  { "Pan Flute", "SC-88"} },
        { 80,  { "PWM Synth 1", "Synth", EDSPType::ModPulse } },
        { 81,  { "PWM Synth 2", "Synth", EDSPType::ModPulse } },
        { 82,  { "PWM Synth 3", "Synth", EDSPType::ModPulse } },
        { 83,  { "Sawtooth Synth", "Synth", EDSPType::Saw} },
        { 84,  { "Triangle Synth 1", "Synth", EDSPType::Tri } },
        { 89,  { "Triangle Synth 2", "Synth", EDSPType::Tri } },
        { 90,  { "PWM Synth 1", "Synth", EDSPType::ModPulse } },
        { 91,  { "PWM Synth 2", "Synth", EDSPType::ModPulse } },
        { 93,  { "Sawtooth Synth 2", "Synth", EDSPType::Saw } },
        { 105, { "Music Box", "JV-1080" } },
        { 106, { "Sitar Gliss", "JV-1080"} },
        { 107, { "Balaphone", "JV-1080"} },
        { 108, { "Shout", "JV-1080"} },
        { 109, { "Bonang", "JV-1080"} },
        { 110, { "Gender", "JV-1080"} },
        { 111, { "Sitar", "JV-1080"} },
        { 112, { "Ritual Loop", "JV-1080"} },
        { 113, { "Daila Loop", "JV-1080"} },
        { 114, { "Steel Drums", "JV-1080"} },
        { 116, { "Verb Lo Tom", "JV-1080"} },
        { 127, { "Drum kit", "SC-88" } },

        // Unused synths (only used for SFX)
        { 85,  { "Unused PWM Synth", "Synth", EDSPType::ModPulse, false } },
        { 86,  { "Unused PWM Synth", "Synth", EDSPType::ModPulse, false } },
        { 87,  { "Unused PWM Synth", "Synth", EDSPType::ModPulse, false } },
        { 92,  { "Unused PWM Synth", "Synth", EDSPType::ModPulse, false } },
        { 95,  { "Unused PWM Synth", "Synth", EDSPType::ModPulse, false } },
        { 96,  { "Unused PWM Synth", "Synth", EDSPType::ModPulse, false } },
        { 97,  { "Unused PWM Synth", "Synth", EDSPType::ModPulse, false } },
        { 88,  { "Unused Sawtooth",  "Synth", EDSPType::Saw, false } },
        { 98,  { "Unused Sawtooth",  "Synth", EDSPType::Saw, false } },
        { 94,  { "Unused Triangle",  "Synth", EDSPType::Tri, false } },
        { 99,  { "Unused Triangle",  "Synth", EDSPType::Tri, false } },
    };


    return presetsList;
}

}
