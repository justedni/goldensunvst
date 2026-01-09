#include "PresetCombo.h"
#include "Presets/PresetsHandler.h"
#include "CustomLookAndFeel.h"

namespace GSVST {

ComboItem::ComboItem(PresetCombo* parent, juce::String&& name, juce::String&& type, juce::Colour&& colour)
    : m_parent(parent)
    , m_name(std::move(name))
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

    auto* lnf = m_parent->getComboLookAndFeel();
    g.setFont(lnf->getDefaultFont());

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

PresetCombo::PresetCombo(MainWindow* mainWindow)
{
    m_lookAndFeel.reset(new ComboLookAndFeel(mainWindow));
    setLookAndFeel(m_lookAndFeel.get());
    getLookAndFeel().setDefaultSansSerifTypeface(m_lookAndFeel->getTypeface());
}

PresetCombo::~PresetCombo()
{
    setLookAndFeel(nullptr);
}


void PresetCombo::setTheme(EUITheme theme)
{
    updateText(theme);
    lookAndFeelChanged();
}

void PresetCombo::updateText(EUITheme theme)
{
    if (m_bankid != -1 && m_program_id != -1)
    {
        juce::String presetName = getPresetName(theme, m_bankid, m_program_id, m_name, juce::String());
        setText(presetName, juce::dontSendNotification);
    }
    else
    {
        setText("", juce::dontSendNotification);
    }
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

    auto theme = m_lookAndFeel->getTheme();

    auto* menu = getRootMenu();
    menu->clear();
    m_programNames.clear();

    for (auto& preset : presets.m_presets)
    {
        juce::String presetName = getPresetName(theme, preset->bankid, preset->programid, preset->name, customInfo ? customInfo->name : juce::String());

        auto mergedId = getMergedId(preset->bankid, preset->programid);

        juce::PopupMenu::Item item;
        item.text = presetName;
        item.itemID = mergedId + 1;
        item.customComponent = new ComboItem(
            this,
            juce::String(presetName),
            juce::String(EnumToString_EPresetType(preset->type)),
            getPresetColour(preset->type));

        menu->addItem(std::move(item));
        m_programNames[mergedId] = preset->name;
    }
}

juce::String PresetCombo::getPresetName(EUITheme theme, int bankid, int programId, const juce::String& programName, const juce::String& customName)
{
    juce::String presetName;
    if (theme == EUITheme::GS)
    {
        if (customName.isNotEmpty())
            presetName << customName;
        else
            presetName << bankid << ":" << programId << " " << programName;
    }
    else
    {
        presetName << programName;
    }
    return presetName;
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

        auto mergedId = getMergedId(bankId, presetId);
        auto it = m_programNames.find(mergedId);
        if (it != m_programNames.end())
        {
            m_name = it->second;
        }

        setSelectedId(mergedId + 1, juce::dontSendNotification);

        auto theme = m_lookAndFeel->getTheme();
        updateText(theme);
    }
}

}
