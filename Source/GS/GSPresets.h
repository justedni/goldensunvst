#pragma once

#include "Presets/PresetsHandler.h"

namespace GSVST {

struct GSPresets : public PresetsHandler
{
    GSPresets();

    void addSynthsPresets() override;
    const std::list<ProgramInfo>& getCustomProgramList() const override;

    Preset* buildCustomSynthPreset(unsigned short presetId, const std::string& synthName, const ADSR& adsr) override;

private:
    void parseXmlInfo();

    ADSR getADSRInfo(int programId);

    bool validateGSSynth(unsigned short presetId, const std::string& synthName, const ADSR& adsr);
    Preset* buildGSSynthPreset(unsigned short presetId);

    std::vector<int> m_pwmPresets;
    std::vector<int> m_sawPresets;
    std::vector<int> m_triPresets;

    std::list<ProgramInfo> m_programsList;
};

}
