#pragma once

#include <JuceHeader.h>

namespace GSVST {

class LevelMeter : public juce::Component
{
public:
    void paint(juce::Graphics& g) override;

    void setLevel(float lev) { m_level = lev; }
    void setColour(juce::Colour colour) { m_colour = colour; }

private:
    float m_level = 0.0f;
    juce::Colour m_colour;
};

}
