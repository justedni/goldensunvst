#include "SliderEx.h"
#include "CustomLookAndFeel.h"

namespace GSVST {

SliderEx::SliderEx(EProperty prop, juce::String&& name, PropertyListener* parent, double min, double max, double step)
    : m_prop(prop)
    , m_name(std::move(name))
    , m_propListener(parent)
{
    addAndMakeVisible(m_slider);
    m_slider.setRange(min, max, step);
    m_slider.setSliderStyle(juce::Slider::Rotary);
    m_slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 20);
    m_slider.addListener(this);
}

void SliderEx::paint(juce::Graphics& g)
{
    //g.fillAll(juce::Colours::white);
    //g.fillAll(CustomLookAndFeel::getBackgroundColour());

    g.setColour(juce::Colours::white);
    g.setFont(12);
    g.drawText(m_name, 0, 2, getWidth(), labelHeight, juce::Justification::centred);
}

void SliderEx::resized()
{
    m_slider.setBounds(0, labelHeight, getWidth(), getHeight() - labelHeight);
}

void SliderEx::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &m_slider)
    {
        if (m_propListener)
        {
            auto val = static_cast<uint8_t>(slider->getValue());
            m_propListener->setPropertyVal(m_prop, val);
        }
    }
}

void SliderEx::setValue(int val)
{
    m_slider.setValue(val, juce::dontSendNotification);
}

void SliderEx::setRange(double min, double max, double step)
{
    m_slider.setRange(min, max, step);
}

}
