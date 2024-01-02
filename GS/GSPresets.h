#pragma once

#include "Presets/PresetsHandler.h"

namespace GSVST {

struct GSPresets : public PresetsHandler
{
    GSPresets();

    void addSynthsPresets() override;
    const std::map<int, ProgramInfo>& getHandledProgramsList() const override;

private:
    ADSR getADSRInfo(int programId) const;
};

}
