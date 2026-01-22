#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FancyKnob : public juce::Slider
{
public:
    FancyKnob() : juce::Slider()
    {
        setSliderStyle(RotaryHorizontalVerticalDrag);
        setTextBoxStyle(NoTextBox, false, 0, 0);
        setColour(rotarySliderFillColourId, juce::Colour::fromString("0xff00d4ff"));
        setColour(rotarySliderOutlineColourId, juce::Colour::fromString("0xffd4af37").withAlpha(0.6f));
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();
        const float outerRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.45f;
        const float innerRadius = outerRadius * 0.7f;

        const double time = juce::Time::getMillisecondCounterHiRes() * 0.001;
        const float pulse = 0.6f + 0.4f * std::sin(time * 3.0f);

        // Outer shadow/glow
        juce::ColourGradient outerGlow(juce::Colour::fromString("0xffff2fb2").withAlpha(0.3f * pulse), centreX, centreY,
                                       juce::Colours::transparentBlack, centreX, centreY, true);
        g.setGradientFill(outerGlow);
        g.fillEllipse(centreX - outerRadius * 1.4f, centreY - outerRadius * 1.4f, outerRadius * 2.8f, outerRadius * 2.8f);

        // Main knob body - black with subtle gradients
        juce::ColourGradient bodyGrad(juce::Colour::fromRGB(20, 20, 25), centreX, centreY - innerRadius,
                                      juce::Colour::fromRGB(10, 10, 15), centreX, centreY + innerRadius, false);
        g.setGradientFill(bodyGrad);
        g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Subtle inner highlight for depth
        juce::ColourGradient highlightGrad(juce::Colour::fromRGB(35, 35, 40).withAlpha(0.6f), centreX - innerRadius * 0.3f, centreY - innerRadius * 0.7f,
                                          juce::Colour::fromRGB(45, 45, 50).withAlpha(0.3f), centreX + innerRadius * 0.3f, centreY + innerRadius * 0.3f, false);
        g.setGradientFill(highlightGrad);
        g.fillEllipse(centreX - innerRadius * 0.8f, centreY - innerRadius * 0.8f, innerRadius * 1.6f, innerRadius * 1.6f);

        // Tick marks around the knob
        g.setColour(juce::Colour::fromString("0xffa79678").withAlpha(0.6f));
        for (int i = 0; i < 12; ++i)
        {
            const float angle = juce::degreesToRadians((float)i * 30.0f);
            const float tickInner = innerRadius * 0.9f;
            const float tickOuter = outerRadius * 0.95f;
            const float x1 = centreX + tickInner * std::cos(angle);
            const float y1 = centreY + tickInner * std::sin(angle);
            const float x2 = centreX + tickOuter * std::cos(angle);
            const float y2 = centreY + tickOuter * std::sin(angle);
            g.drawLine(x1, y1, x2, y2, (i % 3 == 0) ? 2.0f : 1.0f);
        }

        // Outer ring with subtle gradient
        juce::ColourGradient ringGrad(juce::Colour::fromString("0xffa79678").withAlpha(0.8f), centreX, centreY - outerRadius,
                                      juce::Colour::fromString("0xffa79678").withAlpha(0.4f), centreX, centreY + outerRadius, false);
        g.setGradientFill(ringGrad);
        g.drawEllipse(centreX - outerRadius, centreY - outerRadius, outerRadius * 2.0f, outerRadius * 2.0f, 3.0f);

        // Neon accent ring
        g.setColour(juce::Colour::fromString("0xffff2fb2").withAlpha(0.7f * pulse));
        g.drawEllipse(centreX - outerRadius + 1, centreY - outerRadius + 1, outerRadius * 2.0f - 2.0f, outerRadius * 2.0f - 2.0f, 1.5f);

        // Value indicator arc
        const double minVal = getMinimum();
        const double maxVal = getMaximum();
        const double range = maxVal - minVal;
        const float normalizedValue = (range > 0.0) ? (float)((getValue() - minVal) / range) : 0.5f;

        juce::Path valueArc;
        const float arcStartAngle = juce::MathConstants<float>::pi * 0.75f;
        const float arcEndAngle = arcStartAngle + normalizedValue * juce::MathConstants<float>::pi * 1.5f;
        const float arcRadius = outerRadius * 0.85f;

        valueArc.addArc(centreX - arcRadius, centreY - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                       arcStartAngle, arcEndAngle, true);

        juce::ColourGradient arcGrad(juce::Colour::fromString("0xffff2fb2").withAlpha(0.9f * pulse), centreX, centreY,
                                     juce::Colour::fromString("0xffa79678").withAlpha(0.7f), centreX, centreY, true);
        g.setGradientFill(arcGrad);
        g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Main indicator pointer
        const float indicatorAngle = arcStartAngle + normalizedValue * (arcEndAngle - arcStartAngle);
        const float indicatorLength = innerRadius * 0.8f;
        const float indicatorX = centreX + indicatorLength * std::cos(indicatorAngle);
        const float indicatorY = centreY + indicatorLength * std::sin(indicatorAngle);

        // Indicator shadow
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawLine(centreX + 1, centreY + 1, indicatorX + 1, indicatorY + 1, 3.0f);

        // Main indicator
        juce::ColourGradient indicatorGrad(juce::Colour::fromString("0xffff2fb2").withAlpha(0.95f), centreX, centreY,
                                           juce::Colour::fromString("0xffa79678").withAlpha(0.8f), indicatorX, indicatorY, false);
        g.setGradientFill(indicatorGrad);
        g.drawLine(centreX, centreY, indicatorX, indicatorY, 3.0f);

        // Center indicator dot
        g.setColour(juce::Colour::fromString("0xffff2fb2").withAlpha(0.9f));
        g.fillEllipse(centreX - 4, centreY - 4, 8, 8);

        g.setColour(juce::Colour::fromString("0xffa79678"));
        g.drawEllipse(centreX - 4, centreY - 4, 8, 8, 1.5f);

        // Subtle center highlight
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillEllipse(centreX - 2, centreY - 3, 4, 4);
    }
};
