#include "LevelMeter.h"

namespace GSVST {

void LevelMeter::paint(juce::Graphics& g)
{
    int width = getWidth();
    int height = getHeight();
    float level = (float)std::exp(std::log(m_level) / 3.0);

    auto outerCornerSize = 3.0f;
    auto outerBorderWidth = 2.0f;
    auto totalBlocks = 12;
    auto spacingFraction = 0.03f;

    g.setColour(findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height), outerCornerSize);

    auto doubleOuterBorderWidth = 2.0f * outerBorderWidth;
    auto numBlocks = juce::roundToInt((float)totalBlocks * level);

    auto blockWidth = ((float)width - doubleOuterBorderWidth) / static_cast<float> (totalBlocks);
    auto blockHeight = (float)height - doubleOuterBorderWidth;

    auto blockRectWidth = (1.0f - 2.0f * spacingFraction) * blockWidth;
    auto blockRectSpacing = spacingFraction * blockWidth;

    auto blockCornerSize = 0.1f * blockWidth;

    auto c = m_colour;

    for (auto i = 0; i < totalBlocks; ++i)
    {
        if (i >= numBlocks)
            g.setColour(c.withAlpha(0.5f));
        else
            g.setColour(i < totalBlocks - 1 ? c : juce::Colours::red);

        g.fillRoundedRectangle(outerBorderWidth + ((float)i * blockWidth) + blockRectSpacing,
            outerBorderWidth,
            blockRectWidth,
            blockHeight,
            blockCornerSize);
    }
}

}
