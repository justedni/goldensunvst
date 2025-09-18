#include "GSPresets.h"
#include "Presets/Presets.h"

#include <assert.h>

namespace GSVST {

GSPresets::GSPresets()
    : m_pwmPresets({ 80, 81, 82, 85, 86, 87, 90, 91, 92, 95, 96, 97 })
    , m_sawPresets({ 83, 93, 88, 98 })
    , m_triPresets({ 84, 89, 94, 99 })
{
    parseXmlInfo();

    addSynthsPresets();
    sort();
}

void GSPresets::parseXmlInfo()
{
    auto dllLocation = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    auto xmlFileName = dllLocation.getParentDirectory().getFullPathName() + "\\GoldenSunVST_presets_info.xml";

    auto presetsFile = juce::File(xmlFileName);
    if (!presetsFile.exists())
        return;

    auto doc = juce::XmlDocument(presetsFile);

    bool bHasErrors = !doc.getLastParseError().isEmpty();
    if (bHasErrors)
        return;

    auto parseType = [](auto& typeStr)
    {
        if (typeStr == "PCM")
            return EDSPType::PCM;
        else if (typeStr == "ModPulse")
            return EDSPType::ModPulse;
        else if (typeStr == "Saw")
            return EDSPType::Saw;
        else if (typeStr == "Tri")
            return EDSPType::Tri;
        else if (typeStr == "Square")
            return EDSPType::Square;

        return EDSPType::PCM;
    };

    if (auto mainElement = doc.getDocumentElement())
    {
        for (const auto* gameElem : mainElement->getChildIterator())
        {
            auto gameName = gameElem->getStringAttribute("name").toStdString();
            auto& gameContainer = m_programsList[gameName];

            // Parse instruments
            {
                if (auto * instrumentsElem = gameElem->getChildByName("instruments"))
                {
                    for (const auto* instrElem : instrumentsElem->getChildIterator())
                    {
                        auto bankid = instrElem->getIntAttribute("bank");
                        auto programid = instrElem->getIntAttribute("id");
                        auto name = instrElem->getStringAttribute("name");
                        auto device = instrElem->getStringAttribute("device");
                        auto type = instrElem->hasAttribute("type") ? parseType(instrElem->getStringAttribute("type")) : EDSPType::PCM;
                        auto visible = instrElem->hasAttribute("visible") ? (instrElem->getIntAttribute("visible") == 1) : true;

                        gameContainer.push_back(ProgramInfo{
                            bankid, programid,
                            std::string(name.getCharPointer()),
                            std::string(device.getCharPointer()), type, visible});
                    }
                }
            }
        }
    }
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
    if (m_pwmPresets.empty() && m_sawPresets.empty() && m_triPresets.empty())
        return false;

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
    const auto bankId = 0;

    Preset* newPreset = nullptr;

    auto adsr = getADSRInfo(presetId);

    static const auto pwmPreset1 = PWMData(128, 16, 240, 224);
    static const auto pwmPreset2 = PWMData(128, 96, 32, 64);
    static const auto pwmPreset3 = PWMData(64, 128, 64, 64);

    switch (presetId)
    {
    case 80: newPreset = new PWMSynthPreset(bankId, presetId, "PWM Synth 1", std::move(adsr), PWMData(pwmPreset1)); break;
    case 81: newPreset = new PWMSynthPreset(bankId, presetId, "PWM Synth 2", std::move(adsr), PWMData(pwmPreset2)); break;
    case 82: newPreset = new PWMSynthPreset(bankId, presetId, "PWM Synth 3", std::move(adsr), PWMData(pwmPreset3)); break;
    case 90: newPreset = new PWMSynthPreset(bankId, presetId, "PWM Synth 1", std::move(adsr), PWMData(pwmPreset1)); break;
    case 91: newPreset = new PWMSynthPreset(bankId, presetId, "PWM Synth 2", std::move(adsr), PWMData(pwmPreset2)); break;
    case 92: newPreset = new PWMSynthPreset(bankId, presetId, "Unused PWM",  std::move(adsr), PWMData(pwmPreset3)); break;

    case 83: newPreset = new SynthPreset(bankId, presetId, "Sawtooth Synth", EDSPType::Saw, std::move(adsr)); break;
    case 93: newPreset = new SynthPreset(bankId, presetId, "Sawtooth Synth 2", EDSPType::Saw, std::move(adsr)); break;

    case 84: newPreset = new SynthPreset(bankId, presetId, "Triangle Synth 1", EDSPType::Tri, std::move(adsr)); break;
    case 89: newPreset = new SynthPreset(bankId, presetId, "Triangle Synth 2", EDSPType::Tri, std::move(adsr)); break;

    case 85: newPreset = new PWMSynthPreset(bankId, presetId, "Unused PWM", std::move(adsr), PWMData(pwmPreset1)); break;
    case 86: newPreset = new PWMSynthPreset(bankId, presetId, "Unused PWM", std::move(adsr), PWMData(pwmPreset2)); break;
    case 87: newPreset = new PWMSynthPreset(bankId, presetId, "Unused PWM", std::move(adsr), PWMData(pwmPreset3)); break;
    case 88: newPreset = new SynthPreset(bankId, presetId, "Unused Sawtooth", EDSPType::Saw, std::move(adsr)); break;   
    case 94: newPreset = new SynthPreset(bankId, presetId, "Unused Triangle", EDSPType::Tri, std::move(adsr)); break;
    case 95: newPreset = new PWMSynthPreset(bankId, presetId, "Unused PWM", std::move(adsr), PWMData(pwmPreset1)); break;
    case 96: newPreset = new PWMSynthPreset(bankId, presetId, "Unused PWM", std::move(adsr), PWMData(pwmPreset2)); break;
    case 97: newPreset = new PWMSynthPreset(bankId, presetId, "Unused PWM", std::move(adsr), PWMData(pwmPreset3)); break;
    case 98: newPreset = new SynthPreset(bankId, presetId, "Unused Sawtooth", EDSPType::Saw, std::move(adsr)); break;
    case 99: newPreset = new SynthPreset(bankId, presetId, "Unused Triangle", EDSPType::Tri, std::move(adsr)); break;
    }

    return newPreset;
}

const GSPresets::ProgramList& GSPresets::getGamesProgramList() const
{
    return m_programsList;
}

}
