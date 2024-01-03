#include "CustomLookAndFeel.h"

namespace GSVST {

CustomLookAndFeel::CustomLookAndFeel()
{
    auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::gsfont_ttf, BinaryData::gsfont_ttfSize);
    setDefaultSansSerifTypeface(typeface);

    setColour(juce::TabbedComponent::ColourIds::outlineColourId, juce::Colour());
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox& combo)
{
    auto font = LookAndFeel_V4::getComboBoxFont(combo);
    font.setHeight(10);
    return font;
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label& label)
{
    auto font = LookAndFeel_V4::getLabelFont(label);
    font.setHeight(12);
    return font;
}

void CustomLookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto fontSize = 12.0f;
    auto tickWidth = fontSize * 1.1f;

    drawTickBox(g, button, 4.0f, ((float)button.getHeight() - tickWidth) * 0.5f,
        tickWidth, tickWidth,
        button.getToggleState(),
        button.isEnabled(),
        shouldDrawButtonAsHighlighted,
        shouldDrawButtonAsDown);

    g.setColour(button.findColour(juce::ToggleButton::textColourId));
    g.setFont(fontSize);

    if (!button.isEnabled())
        g.setOpacity(0.5f);

    g.drawFittedText(button.getButtonText(),
        button.getLocalBounds().withTrimmedLeft(juce::roundToInt(tickWidth) + 10)
        .withTrimmedRight(2),
        juce::Justification::centredLeft, 10);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // fill
    g.setColour(juce::Colours::grey);
    g.fillEllipse(rx, ry, rw, rw);

    // outline
    g.setColour(juce::Colours::white);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    juce::Path p;
    auto pointerLength = radius;
    auto pointerThickness = 4.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    // pointer
    g.setColour(juce::Colours::white);
    g.fillPath(p);
}

void CustomLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown)
{
    using namespace juce;

    const Rectangle<int> activeArea (button.getActiveArea());

    const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();

    const Colour bkg = button.getToggleState() ? getBackgroundColour() : juce::Colours::black;
    if (button.getToggleState())
    {
        g.setColour (bkg);
    }
    else
    {
        Point<int> p1, p2;

        switch (o)
        {
            case TabbedButtonBar::TabsAtBottom:   p1 = activeArea.getBottomLeft(); p2 = activeArea.getTopLeft();    break;
            case TabbedButtonBar::TabsAtTop:      p1 = activeArea.getTopLeft();    p2 = activeArea.getBottomLeft(); break;
            case TabbedButtonBar::TabsAtRight:    p1 = activeArea.getTopRight();   p2 = activeArea.getTopLeft();    break;
            case TabbedButtonBar::TabsAtLeft:     p1 = activeArea.getTopLeft();    p2 = activeArea.getTopRight();   break;
            default:                              jassertfalse; break;
        }

        g.setGradientFill (ColourGradient (bkg.brighter (0.2f), p1.toFloat(),
                                           bkg.darker (0.1f),   p2.toFloat(), false));
    }

    g.fillRect (activeArea);

    g.setColour (button.findColour (TabbedButtonBar::tabOutlineColourId));

    Rectangle<int> r (activeArea);

    if (o != TabbedButtonBar::TabsAtBottom)   g.fillRect (r.removeFromTop (1));
    if (o != TabbedButtonBar::TabsAtTop)      g.fillRect (r.removeFromBottom (1));
    if (o != TabbedButtonBar::TabsAtRight)    g.fillRect (r.removeFromLeft (1));
    if (o != TabbedButtonBar::TabsAtLeft)     g.fillRect (r.removeFromRight (1));

    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

    Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

    if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
    {
        TabbedButtonBar::ColourIds colID = button.isFrontTab() ? TabbedButtonBar::frontTextColourId
                                                               : TabbedButtonBar::tabTextColourId;

        if (bar->isColourSpecified (colID))
            col = bar->findColour (colID);
        else if (isColourSpecified (colID))
            col = findColour (colID);
    }

    const Rectangle<float> area (button.getTextArea().toFloat());

    float length = area.getWidth();
    float depth  = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    TextLayout textLayout;
    createTabTextLayout (button, length, depth, col, textLayout);

    AffineTransform t;

    switch (o)
    {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated (MathConstants<float>::pi * -0.5f).translated (area.getX(), area.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated (MathConstants<float>::pi *  0.5f).translated (area.getRight(), area.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
        default:                            jassertfalse; break;
    }

    g.addTransform (t);
    textLayout.draw (g, Rectangle<float> (length, depth));
}

void CustomLookAndFeel::drawTabButtonText(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown)
{
    LookAndFeel_V4::drawTabButtonText(button, g, isMouseOver, isMouseDown);
}

juce::Colour CustomLookAndFeel::getBackgroundColour()
{
    // GS background colour
    return juce::Colour(0, 96, 136);
}

void CustomLookAndFeel::drawGSBox(juce::Graphics& g, int x, int y, int width, int height)
{
    auto goldenSunBackColour = getBackgroundColour();
    g.setColour(goldenSunBackColour);
    g.fillRect(x, y, width, height);

    auto drawRect1 = [&](int offset, auto colour)
    {
        g.setColour(colour);
        g.fillRect(x + offset, y + offset, width - offset * 2, 1);
        g.fillRect(x + offset, y + offset, 1, height - 2 * offset);
    };

    auto drawRect2 = [&](int offset, auto colour)
    {
        g.setColour(colour);
        g.fillRect(x + offset, y + height - (offset + 1), width - offset * 2, 1);
        g.fillRect(x + width - (offset + 1), y + offset, 1, height - offset * 2);
    };

    drawRect1(0, juce::Colour(80, 80, 80));
    drawRect1(1, juce::Colour(248, 248, 248));
    drawRect1(2, juce::Colour(160, 160, 160));
    drawRect1(3, juce::Colour(0, 0, 0));
    drawRect1(4, juce::Colour(0, 72, 80));
    drawRect1(5, juce::Colour(0, 80, 96));
    drawRect1(6, juce::Colour(0, 88, 112));

    drawRect2(0, juce::Colour(0, 0, 0));
    drawRect2(1, juce::Colour(160, 160, 160));
    drawRect2(2, juce::Colour(248, 248, 248));
    drawRect2(3, juce::Colour(80, 80, 80));
    drawRect2(4, juce::Colour(8, 128, 184));
    drawRect2(5, juce::Colour(8, 112, 168));
    drawRect2(6, juce::Colour(8, 104, 152));
}

}
