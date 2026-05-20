#pragma once

#include <JuceHeader.h>
#include "Presets/PresetsHandler.h"

namespace GSVST {

class MainWindow;
struct PresetsHandler;
class ComboLookAndFeel;
class PresetCombo;
enum EUITheme : uint8_t;
 
class ComboItem : public juce::PopupMenu::CustomComponent
{
public:
    ComboItem(PresetCombo* parent, juce::String&& name, juce::String&& type, juce::Colour&& colour);

    void getIdealSize(int& idealWidth, int& idealHeight) override { idealWidth = 100; idealHeight = 20; }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PresetCombo* m_parent = nullptr;

    juce::String m_name;
    juce::String m_type;
    juce::Colour m_colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboItem)
};

class PresetCombo : public juce::ComboBox
{
public:
    PresetCombo(MainWindow* mainWindow);
    ~PresetCombo();

    void setTheme(EUITheme theme);

    void refresh(const PresetsHandler& presets, const ProgramInfo* customInfo = nullptr);

    std::pair<int, int> getSelectedProgramId() const;
    void setSelectedProgram(int bankId, int presetId);

    ComboLookAndFeel* getComboLookAndFeel() const { return m_lookAndFeel.get(); }
private:
    static juce::String getPresetName(EUITheme theme, int bankid, int programId, const juce::String& programName, const juce::String& customName);
    static juce::String getFullPresetName(int bankid, int programId, const juce::String& programName, const juce::String& customName);

    int getMergedId(int bankId, int presetId) const;
    void updateText(EUITheme theme);

    int m_bankid = 0;
    int m_program_id = 0;
    juce::String m_name;

    std::map<int, juce::String> m_programNames;

    std::unique_ptr<ComboLookAndFeel> m_lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetCombo)
};

}
