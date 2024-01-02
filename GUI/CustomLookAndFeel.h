#pragma once

#include <JuceHeader.h>

namespace GSVST {

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override;

    juce::Font getComboBoxFont(juce::ComboBox& combo) override;
    juce::Font getLabelFont(juce::Label& label) override;
    void drawToggleButton(
        juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown);
    void drawTabButtonText(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown);

    static void drawGSBox(juce::Graphics& g, int x, int y, int width, int height);
    static juce::Colour getBackgroundColour();
};

}
