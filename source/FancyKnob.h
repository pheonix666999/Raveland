#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <functional>

class FancyKnob : public juce::Slider
{
public:
    using Formatter = std::function<juce::String(float)>;

    FancyKnob() : juce::Slider()
    {
        setSliderStyle(RotaryHorizontalVerticalDrag);
        setTextBoxStyle(NoTextBox, false, 0, 0);
        setColour(rotarySliderFillColourId, juce::Colour::fromString("0xffa79678"));
        setColour(rotarySliderOutlineColourId, juce::Colour::fromString("0xffa79678").withAlpha(0.7f));
    }

    void setFormatter(Formatter fmt)
    {
        formatter = std::move(fmt);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float readoutH = juce::jlimit(14.0f, 22.0f, bounds.getHeight() * 0.22f);
        auto knobArea = bounds;
        knobArea.removeFromBottom(readoutH * 0.6f);
        const float diameter = juce::jmax(10.0f, juce::jmin(knobArea.getWidth(), knobArea.getHeight()));
        const float centreX = knobArea.getCentreX();
        const float centreY = knobArea.getCentreY();
        const float outerRadius = diameter * 0.58f;
        const float innerRadius = outerRadius * 0.82f;

        const juce::Colour gold = juce::Colour::fromString("0xffa79678");
        const juce::Colour goldBright = juce::Colour::fromString("0xffc3aa86");
        const juce::Colour knobDark = juce::Colour::fromRGB(26, 28, 32);
        const juce::Colour knobMid = juce::Colour::fromRGB(44, 46, 50);

        // Shadow
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillEllipse(centreX - outerRadius - 2.0f, centreY - outerRadius + 1.5f, outerRadius * 2.0f + 4.0f, outerRadius * 2.0f + 4.0f);

        // Rim
        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.fillEllipse(centreX - outerRadius, centreY - outerRadius, outerRadius * 2.0f, outerRadius * 2.0f);

        // Knob body
        juce::ColourGradient bodyGrad(knobMid, centreX, centreY - innerRadius,
                                      knobDark, centreX, centreY + innerRadius, false);
        g.setGradientFill(bodyGrad);
        g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Subtle highlight
        juce::ColourGradient highlightGrad(juce::Colours::white.withAlpha(0.06f), centreX - innerRadius * 0.4f, centreY - innerRadius * 0.9f,
                                           juce::Colours::transparentBlack, centreX + innerRadius * 0.6f, centreY + innerRadius * 0.2f, false);
        g.setGradientFill(highlightGrad);
        g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Thin gold ring
        g.setColour(gold.withAlpha(0.65f));
        g.drawEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f, 1.2f);

        // Value indicator
        const double minVal = getMinimum();
        const double maxVal = getMaximum();
        const double range = maxVal - minVal;
        const float normalizedValue = (range > 0.0) ? (float)((getValue() - minVal) / range) : 0.5f;

        const float arcStartAngle = juce::MathConstants<float>::pi * 0.75f;
        const float arcEndAngle = arcStartAngle + normalizedValue * juce::MathConstants<float>::pi * 1.5f;
        const float indicatorAngle = arcStartAngle + normalizedValue * (arcEndAngle - arcStartAngle);
        const float indicatorLength = innerRadius * 0.78f;
        const float indicatorX = centreX + indicatorLength * std::cos(indicatorAngle);
        const float indicatorY = centreY + indicatorLength * std::sin(indicatorAngle);

        // Indicator (short rounded pill)
        const float markerW = innerRadius * 0.12f;
        const float markerH = innerRadius * 0.45f;
        juce::Rectangle<float> marker(-markerW * 0.5f, -innerRadius * 0.92f, markerW, markerH);
        juce::Path markerPath;
        markerPath.addRoundedRectangle(marker, markerW * 0.5f);
        markerPath.applyTransform(juce::AffineTransform::rotation(indicatorAngle).translated(centreX, centreY));
        g.setColour(goldBright.withAlpha(0.95f));
        g.fillPath(markerPath);

        // Readout (high-contrast)
        const float value = (float) getValue();
        const int decimals = (std::abs(value) < 10.0f) ? 1 : 0;
        juce::String text = formatter ? formatter(value) : juce::String(value, decimals);

        auto textBounds = bounds.removeFromBottom(readoutH).reduced(2.0f, 1.0f);
        textBounds.setWidth(juce::jmin(textBounds.getWidth(), diameter * 0.9f));
        textBounds.setX(centreX - textBounds.getWidth() * 0.5f);
        g.setColour(juce::Colours::black.withAlpha(0.72f));
        g.fillRoundedRectangle(textBounds, textBounds.getHeight() * 0.45f);
        g.setColour(gold.withAlpha(0.55f));
        g.drawRoundedRectangle(textBounds, textBounds.getHeight() * 0.45f, 0.8f);

        const float fontSize = juce::jlimit(10.0f, 13.5f, textBounds.getHeight() * 0.72f);
        g.setFont(juce::Font(fontSize, juce::Font::bold));
        g.setColour(goldBright.withAlpha(0.95f));
        g.drawText(text, textBounds, juce::Justification::centred, false);
    }

private:
    Formatter formatter;
};
