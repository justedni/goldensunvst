#include "AboutWindow.h"
#include "MainWindow.h"

#include "CustomLookAndFeel.h"

namespace GSVST {

AboutWindow::AboutWindow(MainWindow& e)
 : m_mainWindow(e)
{
    addAndMakeVisible(m_closeButton);
    m_closeButton.setButtonText("Close");
    m_closeButton.addListener(this);
}

void AboutWindow::paint(juce::Graphics& g)
{
    CustomLookAndFeel::drawGSBox(g, 0, 0, getWidth(), getHeight());

    auto width = getWidth() - 15;
    g.setFont(14);
    g.setColour(juce::Colours::white);
    g.drawText("Massive thanks to:", 0, 110, width, 20, juce::Justification::centredRight);
    g.setFont(9);
    g.drawText("ipatix: creator of agbplay", 0, 130, width, 20, juce::Justification::centredRight);
    g.drawText("FreeJusticeHere: feedback and sample wisdom", 0, 140, width, 20, juce::Justification::centredRight);
    g.setFont(8);
    g.drawText("v0.1 alpha", 0, getHeight() - 15, width, 15, juce::Justification::centredRight);
}

void AboutWindow::resized()
{
    m_closeButton.setBounds(30, getHeight() - 30, 60, 20);
}


void AboutWindow::buttonClicked(juce::Button* button)
{
    if (button == &m_closeButton)
    {
        m_mainWindow.closePopupWindow();
    }
}

}
