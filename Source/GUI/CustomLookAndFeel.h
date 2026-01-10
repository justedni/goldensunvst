#pragma once

#include <JuceHeader.h>

namespace GSVST {

class MainWindow;

enum EUITheme : uint8_t { GS = 1, CoTM };

class ComboLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ComboLookAndFeel(MainWindow* mainWindow);

    EUITheme getTheme() const;
    juce::Typeface::Ptr getTypeface() { return m_typefacePtr; }
    juce::Font getDefaultFont() const { return m_defaultFont; }
    int getComboFontSize() const;

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
    juce::Font getComboBoxFont(juce::ComboBox& combo) override;

private:
    void updateTheme(bool bInit = false);

    EUITheme m_theme = GS;

    MainWindow* m_mainWindow = nullptr;
    juce::Typeface::Ptr m_typefacePtr;
    juce::Font m_defaultFont{ 14.0f };
};


class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    void setTheme(EUITheme theme);
    EUITheme getTheme() const { return m_theme; }

    juce::Font getDefaultFont() const { return m_defaultFont; }
    int getLabelFontSize() const;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override;

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;

    juce::Font getLabelFont(juce::Label& label) override;
    void drawToggleButton(
        juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown);
    void drawTabButtonText(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown);

    static void drawGSBox(juce::Graphics& g, int x, int y, int width, int height);
    static juce::Colour getBackgroundColour();

private:
    void updateTheme();

    EUITheme m_theme = GS;
    juce::Typeface::Ptr m_typefacePtr;
    juce::Font m_defaultFont{ 14.0f };
};

}
