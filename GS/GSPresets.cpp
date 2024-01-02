#include "GSPresets.h"
#include "Presets/Presets.h"

#include <assert.h>

namespace GSVST {

GSPresets::GSPresets()
{
    addSynthsPresets();
    sort();
}

ADSR GSPresets::getADSRInfo(int programId) const
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
    case 89: // Triangle 2
        return ADSR(22, 127, 218, 195);
    case 90: // PWM 1
    case 91: // PWM 2
        return ADSR(255, 235, 90, 178);
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
    m_presets.push_back(new PWMSynthPreset(80, "080 PWM Synth 1", ADSR(85, 165, 193, 178), PWMData(128, 16, -16, 224)));
    m_presets.push_back(new PWMSynthPreset(81, "081 PWM Synth 2", ADSR(85, 165, 193, 178), PWMData(128, 96, 32, 64)));
    m_presets.push_back(new PWMSynthPreset(82, "082 PWM Synth 3", ADSR(85, 165, 193, 178), PWMData(64, 128, 64, 64)));
    m_presets.push_back(new PWMSynthPreset(90, "090 PWM Synth 1", ADSR(255, 235, 90, 178), PWMData(128, 16, -16, 224)));
    m_presets.push_back(new PWMSynthPreset(91, "091 PWM Synth 2", ADSR(255, 235, 90, 178), PWMData(128, 96, 32, 64)));

    m_presets.push_back(new SynthPreset(83, "083 Sawtooth Synth", EDSPType::Saw, ADSR(85, 165, 167, 178)));
    m_presets.push_back(new SynthPreset(84, "084 Triangle Synth 1", EDSPType::Tri, ADSR(85, 127, 218, 165)));
    m_presets.push_back(new SynthPreset(89, "085 Triangle Synth 2", EDSPType::Tri, ADSR(22, 127, 218, 195)));
}

const std::map<int, ProgramInfo>& GSPresets::getHandledProgramsList() const
{
    static std::map<int, ProgramInfo> presetsList = {
        { 8,   { "Celesta", "SC-88"} },
        { 24,  { "Nylon-str. Gt", "SC-88"} },
        { 33,  { "Rock Bass", "SC-88"} },
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
        { 80,  { "PWM Synth 1", "Synth" } },
        { 81,  { "PWM Synth 2", "Synth"  }},
        { 82,  { "PWM Synth 3", "Synth" } },
        { 83,  { "Sawtooth Synth", "Synth" } },
        { 84,  { "Triangle Synth 1", "Synth" } },
        { 89,  { "Triangle Synth 2", "Synth" } },
        { 90,  { "PWM Synth 1", "Synth" } },
        { 91,  { "PWM Synth 2", "Synth" } },
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
        { 127, { "Drum kit", "SC-88" } }
    };

    return presetsList;
}

}
