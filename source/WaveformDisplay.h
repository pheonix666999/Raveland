#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

static const juce::Colour waveformAccent = juce::Colour::fromString("0xff00d4ff");
static const juce::Colour waveformGold = juce::Colour::fromString("0xffd4af37");
static const juce::Colour waveformNeon = juce::Colour::fromString("0xff00ff88");

class WaveformDisplay : public juce::Component,
                        public juce::Timer
{
public:
    WaveformDisplay() { startTimerHz(60); }
    ~WaveformDisplay() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;

        const double time = juce::Time::getMillisecondCounterHiRes() * 0.001;
        const float pulse = 0.7f + 0.3f * std::sin(time * 2.5f);

        // Professional dark background
        g.fillAll(juce::Colour::fromRGB(12, 12, 15));

        // Grid lines for oscilloscope look
        g.setColour(juce::Colour::fromRGB(40, 40, 45));
        const int numVerticalLines = 8;
        const int numHorizontalLines = 4;

        for (int i = 1; i < numVerticalLines; ++i)
        {
            const float x = bounds.getX() + (i / (float)numVerticalLines) * bounds.getWidth();
            g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 0.5f);
        }

        for (int i = 1; i < numHorizontalLines; ++i)
        {
            const float y = bounds.getY() + (i / (float)numHorizontalLines) * bounds.getHeight();
            g.drawLine(bounds.getX(), y, bounds.getRight(), y, 0.5f);
        }

        // Center lines (brighter)
        g.setColour(juce::Colour::fromRGB(60, 60, 65));
        g.drawLine(bounds.getCentreX(), bounds.getY(), bounds.getCentreX(), bounds.getBottom(), 1.0f);
        g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY(), 1.0f);

        // Main waveform - clean and professional
        juce::Path mainWave;
        const int numPoints = 400;
        const float amplitude = bounds.getHeight() * 0.35f;
        const float centerY = bounds.getCentreY();

        for (int i = 0; i <= numPoints; ++i)
        {
            const float x = bounds.getX() + (i / (float)numPoints) * bounds.getWidth();
            const float t = (i / (float)numPoints) * juce::MathConstants<float>::twoPi * 2.0f + phaseOffset;
            // Create a more complex waveform like Serum's LFO shapes
            const float sine = std::sin(t);
            const float triangle = std::asin(sine) * 2.0f / juce::MathConstants<float>::pi;
            const float square = sine > 0 ? 1.0f : -1.0f;
            const float saw = 2.0f * (t / juce::MathConstants<float>::twoPi - std::floor(t / juce::MathConstants<float>::twoPi + 0.5f));

            // Blend between different shapes based on time
            const float shapeBlend = (std::sin(time * 0.5f) + 1.0f) * 0.5f;
            const float waveValue = sine * (1.0f - shapeBlend) + triangle * shapeBlend;

            const float y = centerY + amplitude * waveValue * (0.8f + 0.2f * std::sin(t * 0.3f + time));

            if (i == 0)
                mainWave.startNewSubPath(x, y);
            else
                mainWave.lineTo(x, y);
        }

        // Professional glow effect
        juce::ColourGradient waveGlow(waveformAccent.withAlpha(0.4f * pulse), bounds.getCentreX(), centerY - amplitude,
                                      waveformAccent.withAlpha(0.0f), bounds.getCentreX(), centerY + amplitude, false);
        g.setGradientFill(waveGlow);
        g.strokePath(mainWave, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Main waveform stroke
        g.setColour(waveformAccent.withAlpha(0.9f));
        g.strokePath(mainWave, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Secondary waveform for depth
        juce::Path secondaryWave;
        for (int i = 0; i <= numPoints; ++i)
        {
            const float x = bounds.getX() + (i / (float)numPoints) * bounds.getWidth();
            const float t = (i / (float)numPoints) * juce::MathConstants<float>::twoPi * 3.0f + phaseOffset * 1.3f;
            const float y = centerY + amplitude * 0.6f * std::sin(t + time * 0.3f) * std::cos(t * 0.7f);

            if (i == 0)
                secondaryWave.startNewSubPath(x, y);
            else
                secondaryWave.lineTo(x, y);
        }

        g.setColour(waveformGold.withAlpha(0.6f * pulse));
        g.strokePath(secondaryWave, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Trigger indicator (like oscilloscope)
        const float triggerX = bounds.getX() + bounds.getWidth() * 0.2f;
        const float triggerY = centerY + amplitude * std::sin(phaseOffset * 2.0f);
        g.setColour(waveformAccent);
        g.fillEllipse(triggerX - 3, triggerY - 3, 6, 6);
        g.setColour(juce::Colours::white);
        g.drawEllipse(triggerX - 3, triggerY - 3, 6, 6, 1.0f);

        // Measurement markers
        g.setColour(waveformGold.withAlpha(0.7f));
        // Peak markers
        g.drawLine(bounds.getX() + 10, centerY - amplitude, bounds.getX() + 20, centerY - amplitude, 2.0f);
        g.drawLine(bounds.getX() + 10, centerY + amplitude, bounds.getX() + 20, centerY + amplitude, 2.0f);

        // Time markers
        g.drawLine(bounds.getX() + bounds.getWidth() * 0.25f, bounds.getBottom() - 10, bounds.getX() + bounds.getWidth() * 0.25f, bounds.getBottom() - 20, 2.0f);
        g.drawLine(bounds.getX() + bounds.getWidth() * 0.5f, bounds.getBottom() - 10, bounds.getX() + bounds.getWidth() * 0.5f, bounds.getBottom() - 20, 2.0f);
        g.drawLine(bounds.getX() + bounds.getWidth() * 0.75f, bounds.getBottom() - 10, bounds.getX() + bounds.getWidth() * 0.75f, bounds.getBottom() - 20, 2.0f);

        // Border with professional styling
        g.setColour(juce::Colour::fromRGB(60, 60, 65));
        g.drawRect(bounds, 1.5f);

        // Corner indicators
        g.setColour(waveformAccent.withAlpha(0.5f));
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX() + 15, bounds.getY(), 2.0f);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getY() + 15, 2.0f);
        g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight() - 15, bounds.getY(), 2.0f);
        g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getY() + 15, 2.0f);
        g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getX() + 15, bounds.getBottom(), 2.0f);
        g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getX(), bounds.getBottom() - 15, 2.0f);
        g.drawLine(bounds.getRight(), bounds.getBottom(), bounds.getRight() - 15, bounds.getBottom(), 2.0f);
        g.drawLine(bounds.getRight(), bounds.getBottom(), bounds.getRight(), bounds.getBottom() - 15, 2.0f);
    }

    void timerCallback() override
    {
        phaseOffset += 0.05f;
        if (phaseOffset > juce::MathConstants<float>::twoPi)
            phaseOffset -= juce::MathConstants<float>::twoPi;
        repaint();
    }

private:
    float phaseOffset = 0.0f;
};
