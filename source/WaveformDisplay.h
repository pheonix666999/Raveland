#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>

static const juce::Colour waveformGold = juce::Colour::fromString("0xffa79678");
static const juce::Colour waveformGoldBright = juce::Colour::fromString("0xffc3aa86");
static const juce::Colour waveformBlue = juce::Colour::fromString("0xff2bb6ff");

class WaveformDisplay : public juce::Component,
                        public juce::Timer
{
public:
    enum class DisplayMode
    {
        Osc,
        Stack,
        Mod
    };

    WaveformDisplay()
    {
        initBars();
        startTimerHz(60);
    }
    ~WaveformDisplay() override = default;

    void setWaveformType(int type)
    {
        waveformType = juce::jlimit(0, 5, type);
        repaint();
    }

    void setDisplayMode(DisplayMode mode)
    {
        displayMode = mode;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;

        const double time = juce::Time::getMillisecondCounterHiRes() * 0.001;
        const float pulse = 0.7f + 0.3f * std::sin(time * 2.5f);

        // Dark scope background
        juce::ColourGradient bg(juce::Colour::fromRGB(8, 9, 12), bounds.getX(), bounds.getY(),
                                juce::Colour::fromRGB(5, 6, 9), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(bg);
        g.fillAll();

        juce::ColourGradient topGlow(waveformBlue.withAlpha(0.18f), bounds.getCentreX(), bounds.getY(),
                                     juce::Colours::transparentBlack, bounds.getCentreX(), bounds.getY() + bounds.getHeight() * 0.4f, false);
        g.setGradientFill(topGlow);
        g.fillRect(bounds);

        // Grid lines for oscilloscope look
        g.setColour(juce::Colour::fromRGB(28, 30, 34));
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
        g.setColour(juce::Colour::fromRGB(48, 50, 55));
        g.drawLine(bounds.getCentreX(), bounds.getY(), bounds.getCentreX(), bounds.getBottom(), 1.0f);
        g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY(), 1.0f);

        const float amplitude = bounds.getHeight() * 0.30f;
        const float centerY = bounds.getCentreY();

        if (displayMode == DisplayMode::Stack)
        {
            const int barsCount = (int) bars.size();
            const float padX = 10.0f;
            const float padY = 8.0f;
            const float innerW = bounds.getWidth() - padX * 2.0f;
            const float innerH = bounds.getHeight() - padY * 2.0f;
            const float step = innerW / (float) barsCount;
            const float barW = juce::jmax(1.0f, step * 0.62f);

            juce::ColourGradient grad(waveformBlue.withAlpha(0.9f), bounds.getX(), bounds.getY(),
                                      waveformGold.withAlpha(0.65f), bounds.getCentreX(), bounds.getY(), false);
            g.setGradientFill(grad);

            for (int i = 0; i < barsCount; ++i)
            {
                const float wob = 0.55f + 0.45f * std::sin(time * 1.25f + i * 0.35f + phaseOffset);
                const float v = juce::jlimit(0.0f, 1.0f, bars[(size_t) i] * wob);
                const float bh = v * innerH;
                const float x = bounds.getX() + padX + i * step + (step - barW) * 0.5f;
                const float y = bounds.getY() + padY + (innerH - bh);
                g.fillRect(x, y, barW, bh);
            }

            // Sparkline overlay
            g.setColour(waveformBlue.withAlpha(0.25f));
            juce::Path spark;
            for (int i = 0; i < barsCount; ++i)
            {
                const float wob = 0.55f + 0.45f * std::sin(time * 1.25f + i * 0.35f + phaseOffset);
                const float v = juce::jlimit(0.0f, 1.0f, bars[(size_t) i] * wob);
                const float bh = v * innerH;
                const float x = bounds.getX() + padX + i * step + step * 0.5f;
                const float y = bounds.getY() + padY + (innerH - bh);
                if (i == 0)
                    spark.startNewSubPath(x, y);
                else
                    spark.lineTo(x, y);
            }
            g.strokePath(spark, juce::PathStrokeType(1.5f));
        }
        else
        {
            // Main waveform - stylized but tied to the selected shape
            juce::Path mainWave;
            const int numPoints = 240;
            for (int i = 0; i <= numPoints; ++i)
            {
                const float x = bounds.getX() + (i / (float) numPoints) * bounds.getWidth();
                const float t = (i / (float) numPoints) * juce::MathConstants<float>::twoPi * 2.0f + phaseOffset;
                const float phase = std::fmod(t / juce::MathConstants<float>::twoPi, 1.0f);
                const float sine = std::sin(t);
                const float triangle = 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;
                const float square = phase < 0.5f ? 1.0f : -1.0f;
                const float saw = 2.0f * phase - 1.0f;

                float waveValue = 0.0f;
                switch (waveformType)
                {
                    case 0: waveValue = saw; break;
                    case 1: waveValue = square; break;
                    case 2: waveValue = sine; break;
                    case 3: waveValue = triangle; break;
                    case 4: waveValue = (saw + 0.2f * std::sin(t * 2.0f)) * 0.9f; break;
                    case 5: waveValue = (saw + 0.35f * std::sin(t * 3.0f)) * 0.95f; break;
                    default: waveValue = saw; break;
                }

                const float y = centerY + amplitude * waveValue * (0.82f + 0.18f * std::sin(t * 0.9f + time));

                if (i == 0)
                    mainWave.startNewSubPath(x, y);
                else
                    mainWave.lineTo(x, y);
            }

            juce::ColourGradient lineGrad(waveformBlue.withAlpha(0.95f), bounds.getX(), bounds.getY(),
                                          waveformGold.withAlpha(0.65f), bounds.getCentreX(), bounds.getY(), false);

            g.setGradientFill(lineGrad);
            g.strokePath(mainWave, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Highlight pass
            g.setColour(waveformBlue.withAlpha(0.22f));
            g.strokePath(mainWave, juce::PathStrokeType(1.0f));

            // Moving sparkle
            const float dotU = std::fmod(phaseOffset * 0.22f, 1.0f);
            const float dotX = bounds.getX() + dotU * bounds.getWidth();
            const float dotY = centerY + std::sin(dotU * juce::MathConstants<float>::twoPi + phaseOffset) * amplitude;
            g.setColour(juce::Colours::white.withAlpha(0.55f));
            g.fillEllipse(dotX - 2.0f, dotY - 2.0f, 4.0f, 4.0f);
        }

        // Measurement markers
        g.setColour(waveformGold.withAlpha(0.6f));
        // Peak markers
        g.drawLine(bounds.getX() + 10, centerY - amplitude, bounds.getX() + 20, centerY - amplitude, 2.0f);
        g.drawLine(bounds.getX() + 10, centerY + amplitude, bounds.getX() + 20, centerY + amplitude, 2.0f);

        // Time markers
        g.drawLine(bounds.getX() + bounds.getWidth() * 0.25f, bounds.getBottom() - 10, bounds.getX() + bounds.getWidth() * 0.25f, bounds.getBottom() - 20, 2.0f);
        g.drawLine(bounds.getX() + bounds.getWidth() * 0.5f, bounds.getBottom() - 10, bounds.getX() + bounds.getWidth() * 0.5f, bounds.getBottom() - 20, 2.0f);
        g.drawLine(bounds.getX() + bounds.getWidth() * 0.75f, bounds.getBottom() - 10, bounds.getX() + bounds.getWidth() * 0.75f, bounds.getBottom() - 20, 2.0f);

        // Border with demo styling
        g.setColour(waveformBlue.withAlpha(0.32f));
        g.drawRect(bounds, 1.2f);
    }

    void timerCallback() override
    {
        phaseOffset += 0.05f;
        if (phaseOffset > juce::MathConstants<float>::twoPi)
            phaseOffset -= juce::MathConstants<float>::twoPi;
        repaint();
    }

private:
    void initBars()
    {
        juce::Random r(1337);
        for (auto& v : bars)
            v = 0.2f + std::pow(r.nextFloat(), 1.7f) * 0.95f;
    }

    float phaseOffset = 0.0f;
    int waveformType = 0;
    DisplayMode displayMode = DisplayMode::Osc;
    std::array<float, 56> bars {};
};
