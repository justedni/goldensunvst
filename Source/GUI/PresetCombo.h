#pragma once

#include <JuceHeader.h>

namespace GSVST {

struct PresetsHandler;

class PresetCombo : public juce::ComboBox
{
public:
    void refresh(const PresetsHandler& presets);

    int getSelectedProgramId() const;
    void setSelectedProgram(int presetId);
};

}
