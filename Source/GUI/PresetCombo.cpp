#include "PresetCombo.h"
#include "Presets/PresetsHandler.h"

namespace GSVST {

void PresetCombo::refresh(const PresetsHandler& presets)
{
    clear();

    for (auto& preset : presets.m_presets)
    {
        addItem(preset->name, preset->id);
    }
}

}
