#include "PresetCombo.h"
#include "Presets/PresetsHandler.h"

namespace GSVST {

ComboItem::ComboItem(juce::String&& name, juce::String&& type, juce::Colour&& colour)
    : m_name(std::move(name))
    , m_type(std::move(type))
    , m_colour(std::move(colour))
{
}

void ComboItem::paint(juce::Graphics& g)
{
    auto textColour = findColour(juce::PopupMenu::textColourId);

    juce::Rectangle<int> area(0, 0, getWidth(), getHeight());
    auto r = area.reduced(1);

    if (isItemHighlighted() && isEnabled())
    {
        g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
        g.fillRect(r);

        g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
    }
    else
    {
        g.setColour(textColour.withMultipliedAlpha(isEnabled() ? 1.0f : 0.5f));
    }

    r.reduce(juce::jmin(5, area.getWidth() / 20), 0);

    g.setFont(12);

    r.removeFromRight(3);
    g.drawFittedText(m_name, r, juce::Justification::centredLeft, 1);

    g.setFont(10);
    g.setColour(m_colour);
    g.drawText(m_type, getWidth() - 50, 0, 50, getHeight(), juce::Justification::centredRight);
}

void ComboItem::resized()
{
}

void PresetCombo::refresh(const PresetsHandler& presets, const ProgramInfo* customInfo)
{
    clear();

    auto getPresetColour = [](EPresetType type)
    {
        switch (type)
        {
        case EPresetType::Sample:
            return juce::Colours::yellow;
        case EPresetType::Synth:
            return juce::Colours::red;
        case EPresetType::Soundfont:
        default:
            return juce::Colours::white;
        }
    };


    auto* menu = getRootMenu();
    for (auto& preset : presets.m_presets)
    {
        juce::String presetName;
        if (customInfo)
            presetName << customInfo->name;
        else
            presetName << preset->bankid << ":" << preset->programid << " " << preset->name;

        juce::PopupMenu::Item item;
        item.text = presetName;
        item.itemID = getMergedId(preset->bankid, preset->programid) + 1;
        item.customComponent = new ComboItem(
            juce::String(presetName),
            juce::String(EnumToString_EPresetType(preset->type)),
            getPresetColour(preset->type));

        menu->addItem(std::move(item));
    }
}

int PresetCombo::getMergedId(int bankId, int presetId) const
{
    return (bankId << 8) | presetId;
}

std::pair<int, int>  PresetCombo::getSelectedProgramId() const
{
    auto realId = getSelectedId() - 1;
    int bankId = realId >> 8;
    int programId = realId & 0xff;
    return std::make_pair(bankId, programId);
}

void PresetCombo::setSelectedProgram(int bankId, int presetId)
{
    if (bankId != m_bankid || presetId != m_program_id)
    {
        m_bankid = bankId;
        m_program_id = presetId;
        setSelectedId(getMergedId(bankId, presetId) + 1, juce::dontSendNotification);
    }
}

}
