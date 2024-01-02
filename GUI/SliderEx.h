#pragma once

#include <JuceHeader.h>
#include "PropertyListener.h"

namespace GSVST {

class SliderEx : public juce::Component
    , public juce::Slider::Listener
{
public:
    SliderEx(EProperty prop, juce::String&& name, PropertyListener* parent, double min, double max, double step);

    void paint(juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;

    const juce::Slider* getSlider() const { return &m_slider; }
    void setValue(int val);

    void setRange(double min, double max, double step);

private:
    juce::Slider m_slider;

    const EProperty m_prop;
    const juce::String m_name;
    PropertyListener* m_propListener = nullptr;

    const int labelHeight = 20;
};

}
