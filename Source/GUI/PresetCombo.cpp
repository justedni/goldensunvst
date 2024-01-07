#include "PresetCombo.h"
#include "Presets/PresetsHandler.h"

namespace GSVST {

void PresetCombo::refresh(const PresetsHandler& presets)
{
    clear();

    for (auto& preset : presets.m_presets)
    {
        addItem(preset->name, preset->programid + 1);
    }
}

int PresetCombo::getSelectedProgramId() const
{
    return (getSelectedId() - 1);

}

void PresetCombo::setSelectedProgram(int presetId)
{
    setSelectedId(presetId + 1);
}

}
