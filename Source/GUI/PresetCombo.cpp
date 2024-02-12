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

void PresetCombo::refresh(const PresetsHandler& presets)
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
        juce::PopupMenu::Item item;
        item.text = juce::String(preset->name);
        item.itemID = preset->programid + 1;
        item.customComponent = new ComboItem(
            juce::String(preset->name),
            juce::String(EnumToString_EPresetType(preset->type)),
            getPresetColour(preset->type));

        menu->addItem(std::move(item));
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
