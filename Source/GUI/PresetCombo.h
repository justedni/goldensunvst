#pragma once

#include <JuceHeader.h>

namespace GSVST {

struct PresetsHandler;

class ComboItem : public juce::PopupMenu::CustomComponent
{
public:
    ComboItem(juce::String&& name, juce::String&& type, juce::Colour&& colour);

    void getIdealSize(int& idealWidth, int& idealHeight) override { idealWidth = 100; idealHeight = 20; }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::String m_name;
    juce::String m_type;
    juce::Colour m_colour;
};

class PresetCombo : public juce::ComboBox
{
public:
    void refresh(const PresetsHandler& presets);

    int getSelectedProgramId() const;
    void setSelectedProgram(int presetId);
};

}
