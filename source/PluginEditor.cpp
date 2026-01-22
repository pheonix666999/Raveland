#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_graphics/juce_graphics.h>

static const juce::Colour colourBackground = juce::Colour::fromRGB(12, 12, 15);
static const juce::Colour colourSurface = juce::Colour::fromRGB(18, 18, 22);
static const juce::Colour colourGold = juce::Colour::fromString("0xffd4af37");
static const juce::Colour colourAccent = juce::Colour::fromString("0xff00d4ff");
static const juce::Colour colourNeon = juce::Colour::fromString("0xff00ff88");
static const juce::Colour colourText = juce::Colour::fromRGB(240, 240, 245);
static const juce::Colour colourTextSecondary = juce::Colour::fromRGB(180, 180, 190);

void RavelandAudioProcessorEditor::loadLogos()
{
    // Load RaveLand logo from file - try multiple locations
    juce::File logoFile;
    
    // Try current directory first
    logoFile = juce::File::getCurrentWorkingDirectory().getChildFile("1.jpg");
    if (!logoFile.existsAsFile())
    {
        // Try executable directory
        logoFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("1.jpg");
    }
    if (!logoFile.existsAsFile())
    {
        // Try project root
        logoFile = juce::File("h:/PROJECTS/Fiverr/Raveland/1.jpg");
    }
    if (!logoFile.existsAsFile())
    {
        // Try relative to source
        logoFile = juce::File::getCurrentWorkingDirectory().getParentDirectory().getChildFile("1.jpg");
    }
    
    if (logoFile.existsAsFile())
    {
        // Use JPEGImageFormat directly for JPG files
        juce::JPEGImageFormat jpegFormat;
        auto inputStream = logoFile.createInputStream();
        if (inputStream != nullptr)
        {
            juce::Image logo = jpegFormat.decodeImage(*inputStream);
            if (logo.isValid())
            {
                ravelandLogoImage = logo;
            }
        }
    }
    
    // Create NS Audio logo programmatically
    nsAudioLogoImage = juce::Image(juce::Image::ARGB, 160, 32, true);
    juce::Graphics g(nsAudioLogoImage);
    g.fillAll(juce::Colours::transparentBlack);
    
    g.setColour(juce::Colour::fromFloatRGBA(0, 0, 0, 0.3f));
    g.fillRoundedRectangle(0.5f, 0.5f, 159.0f, 31.0f, 12.0f);
    
    g.setColour(colourGold.withAlpha(0.3f));
    g.drawRoundedRectangle(0.5f, 0.5f, 159.0f, 31.0f, 12.0f, 1.0f);
    
    juce::ColourGradient textGrad(juce::Colours::white.withAlpha(0.92f), 0, 0,
                                  colourGold.withAlpha(0.95f), 0, 32, false);
    g.setGradientFill(textGrad);
    g.setFont(juce::Font(16.0f, juce::Font::bold).withExtraKerningFactor(0.15f));
    g.drawText("NS", juce::Rectangle<float>(16, 0, 50, 32), juce::Justification::centredLeft, false);
    
    g.setColour(colourGold);
    g.setFont(juce::Font(12.0f, juce::Font::bold).withExtraKerningFactor(0.1f));
    g.drawText("AUDIO", juce::Rectangle<float>(62, 0, 100, 32), juce::Justification::centredLeft, false);
}

void RavelandAudioProcessorEditor::drawRaveLandLogo(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (ravelandLogoImage.isValid())
    {
        // Draw the actual logo image
        g.drawImage(ravelandLogoImage, bounds, juce::RectanglePlacement::centred);
        
        // Add glow effect
        juce::ColourGradient logoGlow(colourAccent.withAlpha(0.3f), bounds.getCentreX(), bounds.getCentreY(),
                                      juce::Colours::transparentBlack, bounds.getCentreX(), bounds.getCentreY(), true);
        g.setGradientFill(logoGlow);
        g.fillEllipse(bounds.expanded(15));
    }
    else
    {
        // Fallback: draw text logo
        juce::ColourGradient textGrad(colourGold, bounds.getX(), bounds.getY(),
                                      colourAccent.withAlpha(0.9f), bounds.getRight(), bounds.getY(), false);
        g.setGradientFill(textGrad);
        g.setFont(juce::Font(32.0f, juce::Font::italic | juce::Font::bold).withExtraKerningFactor(0.1f));
        g.drawText("RaveLand", bounds, juce::Justification::centred, false);
        
        // Glow
        juce::ColourGradient logoGlow(colourAccent.withAlpha(0.25f), bounds.getCentreX(), bounds.getCentreY(),
                                      juce::Colours::transparentBlack, bounds.getCentreX(), bounds.getCentreY(), true);
        g.setGradientFill(logoGlow);
        g.fillEllipse(bounds.expanded(10));
    }
}

void RavelandAudioProcessorEditor::drawNeonGlow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const double time = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const float phase = glowPhase;
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float pulse = 0.6f + 0.4f * std::sin(time * 2.5f);

    // Enhanced animated radial gradients with more layers
    for (int i = 0; i < 6; ++i)
    {
        const float offset = phase + (i * juce::MathConstants<float>::twoPi / 6.0f);
        const float x = cx + std::cos(offset + time) * (80.0f + i * 20.0f);
        const float y = cy + std::sin(offset + time * 1.3f) * (50.0f + i * 15.0f);

        const float alpha = 0.4f * pulse * (1.0f - i * 0.15f);
        juce::ColourGradient grad(colourAccent.withAlpha(alpha), x, y,
                                  juce::Colours::transparentBlack, x, y, true);
        grad.addColour(0.3f, colourGold.withAlpha(alpha * 0.8f));
        grad.addColour(0.6f, colourNeon.withAlpha(alpha * 0.6f));
        grad.addColour(0.8f, colourAccent.withAlpha(alpha * 0.4f));

        g.setGradientFill(grad);
        g.fillEllipse(x - (350 + i * 50), y - (250 + i * 30), (700 + i * 100), (500 + i * 60));
    }

    // Enhanced particle effects with trails
    for (int i = 0; i < 80; ++i)
    {
        const float p = phase + i * 0.08f;
        const float px = cx + std::cos(p + time * 0.5f) * (bounds.getWidth() * 0.45f);
        const float py = cy + std::sin(p * 1.5f + time * 0.7f) * (bounds.getHeight() * 0.35f);
        const float particleSize = 3.0f + std::sin(time * 4.0f + i * 0.1f) * 2.0f;
        const float alpha = 0.4f * pulse * (0.3f + 0.7f * std::sin(p * 3.0f + time * 1.5f));

        // Particle trail
        juce::ColourGradient trailGrad(colourAccent.withAlpha(alpha * 0.5f), px, py,
                                       juce::Colours::transparentBlack, px + std::cos(p) * 20, py + std::sin(p) * 20, false);
        g.setGradientFill(trailGrad);
        g.fillEllipse(px - particleSize * 3, py - particleSize * 3, particleSize * 6, particleSize * 6);

        // Bright particle core
        g.setColour(colourAccent.withAlpha(alpha));
        g.fillEllipse(px - particleSize * 0.5f, py - particleSize * 0.5f, particleSize, particleSize);

        // Additional sparkle
        if (i % 3 == 0)
        {
            g.setColour(colourGold.withAlpha(alpha * 0.8f));
            g.fillEllipse(px - 1, py - 1, 2, 2);
        }
    }

    // Add sweeping wave patterns
    for (int wave = 0; wave < 3; ++wave)
    {
        juce::Path sweepWave;
        const float wavePhase = phase + wave * 1.0f;
        const float amplitude = bounds.getHeight() * 0.15f;
        const float frequency = 2.0f + wave * 0.5f;

        for (int i = 0; i <= 100; ++i)
        {
            const float t = i / 100.0f;
            const float x = bounds.getX() + t * bounds.getWidth();
            const float y = cy + amplitude * std::sin(t * juce::MathConstants<float>::twoPi * frequency + wavePhase + time * 2.0f);

            if (i == 0)
                sweepWave.startNewSubPath(x, y);
            else
                sweepWave.lineTo(x, y);
        }

        const float alpha = 0.2f * pulse * (1.0f - wave * 0.3f);
        juce::ColourGradient waveGrad(colourAccent.withAlpha(alpha), bounds.getCentreX(), cy - amplitude,
                                      colourGold.withAlpha(alpha * 0.5f), bounds.getCentreX(), cy + amplitude, false);
        g.setGradientFill(waveGrad);
        g.strokePath(sweepWave, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Add geometric patterns
    for (int geo = 0; geo < 4; ++geo)
    {
        const float geoPhase = phase + geo * juce::MathConstants<float>::pi * 0.5f;
        const float radius = 100.0f + geo * 50.0f;
        const float geoX = cx + std::cos(geoPhase + time) * radius;
        const float geoY = cy + std::sin(geoPhase + time * 1.2f) * (radius * 0.7f);

        juce::Path hexagon;
        for (int side = 0; side < 6; ++side)
        {
            const float angle = side * juce::MathConstants<float>::twoPi / 6.0f + geoPhase;
            const float hx = geoX + std::cos(angle) * 30.0f;
            const float hy = geoY + std::sin(angle) * 30.0f;

            if (side == 0)
                hexagon.startNewSubPath(hx, hy);
            else
                hexagon.lineTo(hx, hy);
        }
        hexagon.closeSubPath();

        const float alpha = 0.15f * pulse * (1.0f - geo * 0.2f);
        g.setColour(colourAccent.withAlpha(alpha));
        g.strokePath(hexagon, juce::PathStrokeType(1.5f));

        g.setColour(colourGold.withAlpha(alpha * 0.7f));
        g.strokePath(hexagon, juce::PathStrokeType(0.5f));
    }
}

void RavelandAudioProcessorEditor::drawPanelWithGlow(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& title)
{
    const double time = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const float pulse = 0.8f + 0.2f * std::sin(time * 2.0f);

    // Modern card shadow
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillRoundedRectangle(bounds.translated(3, 3), 12.0f);

    // Clean card background
    juce::ColourGradient cardBg(colourSurface, bounds.getX(), bounds.getY(),
                                 colourSurface.darker(0.1f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(cardBg);
    g.fillRoundedRectangle(bounds, 12.0f);

    // Subtle border
    g.setColour(colourGold.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 12.0f, 1.0f);

    // Modern accent line
    g.setColour(colourAccent.withAlpha(0.4f * pulse));
    g.fillRect(bounds.getX() + 16, bounds.getY() + 16, bounds.getWidth() - 32, 2.0f);

    // Clean title typography
    auto titleArea = bounds.removeFromTop(24.0f);
    g.setColour(colourText);
    g.setFont(juce::Font(13.0f, juce::Font::bold).withExtraKerningFactor(0.05f));
    g.drawText(title, titleArea.reduced(16, 4), juce::Justification::centredLeft, false);
}

void RavelandAudioProcessorEditor::setupToggle(juce::ToggleButton& button)
{
    button.setColour(juce::ToggleButton::textColourId, colourGold);
    button.setColour(juce::ToggleButton::tickColourId, colourAccent);
    button.setColour(juce::ToggleButton::tickDisabledColourId, colourGold.withAlpha(0.3f));
}

RavelandAudioProcessorEditor::RavelandAudioProcessorEditor(RavelandAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    loadLogos();
    startTimerHz(60); // 60fps for smooth animations

    auto& vts = processor.getValueTreeState();

    // Create waveform displays
    for (int i = 0; i < 3; ++i)
    {
        oscWaveforms[i] = std::make_unique<WaveformDisplay>();
        addAndMakeVisible(oscWaveforms[i].get());
        
        layerWaveforms[i] = std::make_unique<WaveformDisplay>();
        addAndMakeVisible(layerWaveforms[i].get());
    }

    // Preset browser
    presetCombo.addItemList(processor.getPresetNames(), 1);
    presetCombo.setSelectedId(processor.getCurrentPresetIndex() + 1);
    presetCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromFloatRGBA(0.1f, 0.1f, 0.12f, 0.95f));
    presetCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    presetCombo.setColour(juce::ComboBox::outlineColourId, colourAccent.withAlpha(0.5f));
    presetCombo.setColour(juce::ComboBox::arrowColourId, colourAccent);
    presetCombo.onChange = [this] { processor.setCurrentProgram(presetCombo.getSelectedId() - 1); };
    addAndMakeVisible(presetCombo);

    // Master
    addAndMakeVisible(masterGainSlider);
    masterGainLabel.setText("MASTER", juce::dontSendNotification);
    masterGainLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    masterGainLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    masterGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterGainLabel);
    masterGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "masterGain", masterGainSlider);

    // Oscillators
    for (int i = 0; i < 3; ++i)
    {
        auto prefix = "osc" + juce::String(i + 1);
        auto& osc = oscControls[i];

        setupToggle(osc.enabled);
        osc.enabled.setButtonText("OSC " + juce::String(i + 1));
        addAndMakeVisible(osc.enabled);

        addAndMakeVisible(osc.voices);
        osc.voicesLabel.setText("VOICES", juce::dontSendNotification);
        osc.voicesLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.voicesLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.voicesLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.voicesLabel);

        addAndMakeVisible(osc.detune);
        osc.detuneLabel.setText("DETUNE", juce::dontSendNotification);
        osc.detuneLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.detuneLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.detuneLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.detuneLabel);

        addAndMakeVisible(osc.level);
        osc.levelLabel.setText("LEVEL", juce::dontSendNotification);
        osc.levelLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.levelLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.levelLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.levelLabel);

        osc.enabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, prefix + "Enabled", osc.enabled);
        osc.voicesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Voices", osc.voices);
        osc.detuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Detune", osc.detune);
        osc.levelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Level", osc.level);
    }

    // Layers
    for (int i = 0; i < 3; ++i)
    {
        auto prefix = "layer" + juce::String(i + 1);
        auto& layer = layerControls[i];

        setupToggle(layer.enabled);
        layer.enabled.setButtonText("LAYER " + juce::String(i + 1));
        addAndMakeVisible(layer.enabled);

        addAndMakeVisible(layer.gain);
        layer.gainLabel.setText("GAIN", juce::dontSendNotification);
        layer.gainLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        layer.gainLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        layer.gainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(layer.gainLabel);

        addAndMakeVisible(layer.startRand);
        layer.startRandLabel.setText("START RAND", juce::dontSendNotification);
        layer.startRandLabel.setFont(juce::Font(8.0f, juce::Font::bold));
        layer.startRandLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        layer.startRandLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(layer.startRandLabel);

        layer.enabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, prefix + "Enabled", layer.enabled);
        layer.gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Gain", layer.gain);
        layer.startRandAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "StartRand", layer.startRand);
    }

    // FX
    addAndMakeVisible(reverbMixSlider);
    reverbMixLabel.setText("MIX", juce::dontSendNotification);
    reverbMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    reverbMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    reverbMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(reverbMixLabel);

    addAndMakeVisible(reverbSizeSlider);
    reverbSizeLabel.setText("SIZE", juce::dontSendNotification);
    reverbSizeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    reverbSizeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    reverbSizeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(reverbSizeLabel);

    addAndMakeVisible(reverbDampSlider);
    reverbDampLabel.setText("DAMP", juce::dontSendNotification);
    reverbDampLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    reverbDampLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    reverbDampLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(reverbDampLabel);

    addAndMakeVisible(delayMixSlider);
    delayMixLabel.setText("MIX", juce::dontSendNotification);
    delayMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    delayMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    delayMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayMixLabel);

    addAndMakeVisible(delayTimeSlider);
    delayTimeLabel.setText("TIME", juce::dontSendNotification);
    delayTimeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    delayTimeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayTimeLabel);

    addAndMakeVisible(delayFeedbackSlider);
    delayFeedbackLabel.setText("FEEDBACK", juce::dontSendNotification);
    delayFeedbackLabel.setFont(juce::Font(7.0f, juce::Font::bold));
    delayFeedbackLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    delayFeedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayFeedbackLabel);

    addAndMakeVisible(chorusMixSlider);
    chorusMixLabel.setText("MIX", juce::dontSendNotification);
    chorusMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    chorusMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    chorusMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(chorusMixLabel);

    addAndMakeVisible(chorusRateSlider);
    chorusRateLabel.setText("RATE", juce::dontSendNotification);
    chorusRateLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    chorusRateLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    chorusRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(chorusRateLabel);

    addAndMakeVisible(chorusDepthSlider);
    chorusDepthLabel.setText("DEPTH", juce::dontSendNotification);
    chorusDepthLabel.setFont(juce::Font(7.0f, juce::Font::bold));
    chorusDepthLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    chorusDepthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(chorusDepthLabel);

    addAndMakeVisible(distMixSlider);
    distMixLabel.setText("MIX", juce::dontSendNotification);
    distMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    distMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    distMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distMixLabel);

    addAndMakeVisible(distDriveSlider);
    distDriveLabel.setText("DRIVE", juce::dontSendNotification);
    distDriveLabel.setFont(juce::Font(7.0f, juce::Font::bold));
    distDriveLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    distDriveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distDriveLabel);

    addAndMakeVisible(distToneSlider);
    distToneLabel.setText("TONE", juce::dontSendNotification);
    distToneLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    distToneLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    distToneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distToneLabel);

    reverbMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "reverbMix", reverbMixSlider);
    reverbSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "reverbSize", reverbSizeSlider);
    reverbDampAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "reverbDamp", reverbDampSlider);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "delayMix", delayMixSlider);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "delayTime", delayTimeSlider);
    delayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "delayFeedback", delayFeedbackSlider);
    chorusMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "chorusMix", chorusMixSlider);
    chorusRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "chorusRate", chorusRateSlider);
    chorusDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "chorusDepth", chorusDepthSlider);
    distMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "distMix", distMixSlider);
    distDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "distDrive", distDriveSlider);
    distToneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "distTone", distToneSlider);

    // Mono/Legato
    setupToggle(monoButton);
    monoButton.setButtonText("Mono");
    addAndMakeVisible(monoButton);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "monoEnabled", monoButton);

    setupToggle(legatoButton);
    legatoButton.setButtonText("Legato");
    addAndMakeVisible(legatoButton);
    legatoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "legatoEnabled", legatoButton);

    addAndMakeVisible(portamentoSlider);
    portamentoLabel.setText("PORTAMENTO", juce::dontSendNotification);
    portamentoLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    portamentoLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    portamentoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(portamentoLabel);
    portamentoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "portamento", portamentoSlider);

    // Waveform display labels
    oscWaveformLabel1.setText("OSCILLATOR WAVEFORM", juce::dontSendNotification);
    oscWaveformLabel1.setFont(juce::Font(8.0f, juce::Font::bold));
    oscWaveformLabel1.setColour(juce::Label::textColourId, colourTextSecondary);
    oscWaveformLabel1.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(oscWaveformLabel1);

    oscWaveformLabel2.setText("OSCILLATOR WAVEFORM", juce::dontSendNotification);
    oscWaveformLabel2.setFont(juce::Font(8.0f, juce::Font::bold));
    oscWaveformLabel2.setColour(juce::Label::textColourId, colourTextSecondary);
    oscWaveformLabel2.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(oscWaveformLabel2);

    oscWaveformLabel3.setText("OSCILLATOR WAVEFORM", juce::dontSendNotification);
    oscWaveformLabel3.setFont(juce::Font(8.0f, juce::Font::bold));
    oscWaveformLabel3.setColour(juce::Label::textColourId, colourTextSecondary);
    oscWaveformLabel3.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(oscWaveformLabel3);

    layerWaveformLabel1.setText("SAMPLE LAYER", juce::dontSendNotification);
    layerWaveformLabel1.setFont(juce::Font(8.0f, juce::Font::bold));
    layerWaveformLabel1.setColour(juce::Label::textColourId, colourTextSecondary);
    layerWaveformLabel1.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(layerWaveformLabel1);

    layerWaveformLabel2.setText("SAMPLE LAYER", juce::dontSendNotification);
    layerWaveformLabel2.setFont(juce::Font(8.0f, juce::Font::bold));
    layerWaveformLabel2.setColour(juce::Label::textColourId, colourTextSecondary);
    layerWaveformLabel2.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(layerWaveformLabel2);

    layerWaveformLabel3.setText("SAMPLE LAYER", juce::dontSendNotification);
    layerWaveformLabel3.setFont(juce::Font(8.0f, juce::Font::bold));
    layerWaveformLabel3.setColour(juce::Label::textColourId, colourTextSecondary);
    layerWaveformLabel3.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(layerWaveformLabel3);

    // Set default size last so `resized()` can safely layout child components.
    setResizable(true, true);
    setResizeLimits(1000, 750, 1920, 1080);
    setSize(1200, 750);
}

void RavelandAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Set window icon on first paint (when peer is available)
    static bool iconSet = false;
    if (!iconSet && getPeer() != nullptr)
    {
        juce::Image iconImage = juce::Image(juce::Image::ARGB, 64, 64, true);
        juce::Graphics iconG(iconImage);
        iconG.fillAll(colourBackground);
        
        if (ravelandLogoImage.isValid())
        {
            // Scale logo to fit icon
            auto scaledBounds = juce::Rectangle<float>(0, 0, 64, 64);
            iconG.drawImage(ravelandLogoImage, scaledBounds, juce::RectanglePlacement::centred);
        }
        else
        {
            // Fallback: create simple icon with RaveLand branding
            iconG.setColour(colourGold);
            iconG.setFont(juce::Font(20.0f, juce::Font::bold));
            iconG.drawText("RL", juce::Rectangle<float>(0, 0, 64, 64), juce::Justification::centred, false);
            
            // Add accent color circle
            iconG.setColour(colourAccent);
            iconG.drawEllipse(18, 18, 28, 28, 2.0f);
        }
        
        getPeer()->setIcon(iconImage);
        iconSet = true;
    }
    
    auto bounds = getLocalBounds().toFloat();
    
    // Modern gradient background
    juce::ColourGradient bgGrad(colourBackground, bounds.getX(), bounds.getY(),
                                 juce::Colour::fromRGB(8, 8, 12), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Subtle animated background pattern
    drawNeonGlow(g, bounds);

    // Modern card-style main container
    auto pluginBounds = bounds.reduced(24, 24);

    // Sleek shadow effect
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(pluginBounds.translated(4, 4), 24.0f);

    // Main surface with modern gradient
    juce::ColourGradient surfaceGrad(colourSurface, pluginBounds.getX(), pluginBounds.getY(),
                                      juce::Colour::fromRGB(15, 15, 18), pluginBounds.getX(), pluginBounds.getBottom(), false);
    g.setGradientFill(surfaceGrad);
    g.fillRoundedRectangle(pluginBounds, 20.0f);

    // Modern border with subtle glow
    g.setColour(colourGold.withAlpha(0.4f));
    g.drawRoundedRectangle(pluginBounds, 20.0f, 1.5f);

    // Accent border
    g.setColour(colourAccent.withAlpha(0.2f));
    g.drawRoundedRectangle(pluginBounds.reduced(1), 19.0f, 0.8f);

    // Modern header with clean design
    auto header = pluginBounds.removeFromTop(120.0f);

    // Header background with subtle gradient
    juce::ColourGradient headerBg(colourSurface.brighter(0.05f), header.getX(), header.getY(),
                                   colourSurface, header.getX(), header.getBottom(), false);
    g.setGradientFill(headerBg);
    g.fillRoundedRectangle(header, 20.0f);

    // Clean separator line
    g.setColour(colourGold.withAlpha(0.3f));
    g.fillRect(header.getX() + 20, header.getBottom() - 1, header.getWidth() - 40, 1.0f);

    // Modern logo section (left)
    if (nsAudioLogoImage.isValid())
    {
        auto logoBounds = juce::Rectangle<float>(32, header.getY() + 20, 160, 32);
        g.drawImage(nsAudioLogoImage, logoBounds, juce::RectanglePlacement::centred);
    }

    // Clean typography for branding
    g.setColour(colourTextSecondary);
    g.setFont(juce::Font(12.0f, juce::Font::bold).withExtraKerningFactor(0.1f));
    g.drawText("NS AUDIO", juce::Rectangle<float>(32, header.getY() + 60, 160, 20),
                juce::Justification::centredLeft, false);

    // Modern title (center)
    auto titleBounds = juce::Rectangle<float>(header.getCentreX() - 200, header.getY() + 25, 400, 50);
    g.setColour(colourText);
    g.setFont(juce::Font(28.0f, juce::Font::bold).withExtraKerningFactor(0.05f));
    g.drawText("RAVELAND", titleBounds, juce::Justification::centred, false);

    // Subtitle with modern styling
    g.setColour(colourAccent);
    g.setFont(juce::Font(11.0f).withExtraKerningFactor(0.2f));
    g.drawText("SYNTHESIZER", juce::Rectangle<float>(header.getCentreX() - 200, header.getY() + 70, 400, 15),
                juce::Justification::centred, false);

    // Version info (right)
    g.setColour(colourTextSecondary);
    g.setFont(juce::Font(10.0f));
    g.drawText("v1.0", juce::Rectangle<float>(header.getRight() - 120, header.getY() + 30, 100, 15),
                juce::Justification::centredRight, false);

    // Main three-column layout
    auto main = pluginBounds.reduced(30, 25);

    auto left = main.removeFromLeft(main.getWidth() * 0.27f);
    auto centre = main.removeFromLeft(main.getWidth() * 0.44f);
    auto right = main;

    // Modern section headers
    g.setColour(colourText);
    g.setFont(juce::Font(14.0f, juce::Font::bold).withExtraKerningFactor(0.1f));

    // Left section
    auto leftHeader = left.removeFromTop(35.0f);
    g.drawText("AUDIO LAYERS", leftHeader.reduced(15, 8), juce::Justification::centredLeft, false);

    // Center section
    auto centreHeader = centre.removeFromTop(35.0f);
    g.drawText("FX & MODULATION", centreHeader.reduced(15, 8), juce::Justification::centredLeft, false);

    // Right section
    auto rightHeader = right.removeFromTop(35.0f);
    g.drawText("SYNTH OSCILLATORS", rightHeader.reduced(15, 8), juce::Justification::centredLeft, false);

    // Modern footer with clean design
    auto footer = pluginBounds.removeFromBottom(80.0f).reduced(24, 16);

    // Clean footer background
    juce::ColourGradient footerBg(colourSurface.darker(0.05f), footer.getX(), footer.getY(),
                                   colourSurface, footer.getX(), footer.getBottom(), false);
    g.setGradientFill(footerBg);
    g.fillRoundedRectangle(footer, 12.0f);

    // Subtle border
    g.setColour(colourGold.withAlpha(0.2f));
    g.drawRoundedRectangle(footer, 12.0f, 1.0f);

    // Modern footer text
    g.setColour(colourTextSecondary);
    g.setFont(juce::Font(11.0f).withExtraKerningFactor(0.05f));
    g.drawText("PERFORMANCE CONTROLS", footer.reduced(16, 8), juce::Justification::centredLeft, false);
    
    // Update animation phase with faster speed for flashier effects
    glowPhase += 0.035f;
    if (glowPhase > juce::MathConstants<float>::twoPi)
        glowPhase -= juce::MathConstants<float>::twoPi;
}

void RavelandAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().toFloat();
    bounds.reduce(24, 24); // Main margin

    // Header section (increased height for better spacing)
    auto header = bounds.removeFromTop(140.0f);

    // Preset combo positioned better
    presetCombo.setBounds(juce::Rectangle<int>(static_cast<int>(bounds.getX() + bounds.getWidth() * 0.5f - 300),
                                                static_cast<int>(header.getBottom() - 50),
                                                600, 40).toNearestInt());

    // Main content area with better proportions
    auto mainArea = bounds.reduced(16, 16);
    mainArea.removeFromTop(20); // Space after header

    // Three main columns with better proportions
    auto leftColumn = mainArea.removeFromLeft(mainArea.getWidth() * 0.30f);
    auto centerColumn = mainArea.removeFromLeft(mainArea.getWidth() * 0.40f);
    auto rightColumn = mainArea;

    // Left Column: Audio Layers
    layoutLayerSection(leftColumn.reduced(8, 8));

    // Center Column: FX Section
    layoutFXSection(centerColumn.reduced(8, 8));

    // Right Column: Oscillators
    layoutOscillatorSection(rightColumn.reduced(8, 8));

    // Footer section
    layoutFooterSection(bounds);
}

void RavelandAudioProcessorEditor::layoutLayerSection(juce::Rectangle<float> area)
{
    // Section title
    auto titleArea = area.removeFromTop(30.0f);
    // Title is drawn in paint()

    // Three layer sections with equal spacing
    const float layerHeight = (area.getHeight() - 20) / 3.0f; // Account for spacing

    for (int i = 0; i < 3; ++i)
    {
        auto layerArea = area.removeFromTop(layerHeight).reduced(4, 8);

        // Enable button
        layerControls[i].enabled.setBounds(layerArea.removeFromTop(28).toNearestInt());

        // Waveform display
        auto waveformArea = layerArea.removeFromTop(90).reduced(2, 4);
        layerWaveforms[i]->setBounds(waveformArea.toNearestInt());

        // Waveform label
        auto waveformLabelArea = waveformArea.withY(waveformArea.getBottom() - 16).withHeight(14);
        if (i == 0) layerWaveformLabel1.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 1) layerWaveformLabel2.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 2) layerWaveformLabel3.setBounds(waveformLabelArea.toNearestInt());

        // Two knobs with labels
        auto knobArea = layerArea.reduced(2, 4);
        auto gainArea = knobArea.removeFromLeft(knobArea.getWidth() * 0.5f).reduced(4, 0);
        layerControls[i].gain.setBounds(gainArea.withTrimmedTop(20).toNearestInt());
        layerControls[i].gainLabel.setBounds(gainArea.withHeight(16).toNearestInt());

        auto startRandArea = knobArea.reduced(4, 0);
        layerControls[i].startRand.setBounds(startRandArea.withTrimmedTop(20).toNearestInt());
        layerControls[i].startRandLabel.setBounds(startRandArea.withHeight(16).toNearestInt());

        // Space between layers
        area.removeFromTop(8);
    }
}

void RavelandAudioProcessorEditor::layoutFXSection(juce::Rectangle<float> area)
{
    // Section title
    auto titleArea = area.removeFromTop(30.0f);
    // Title is drawn in paint()

    // Four FX sections with equal spacing
    const float fxHeight = (area.getHeight() - 30) / 4.0f; // Account for spacing

    // Reverb
    layoutFXBlock(area, 0, fxHeight, {&reverbMixSlider, &reverbSizeSlider, &reverbDampSlider},
                  {&reverbMixLabel, &reverbSizeLabel, &reverbDampLabel});

    // Delay
    layoutFXBlock(area, 1, fxHeight, {&delayMixSlider, &delayTimeSlider, &delayFeedbackSlider},
                  {&delayMixLabel, &delayTimeLabel, &delayFeedbackLabel});

    // Chorus
    layoutFXBlock(area, 2, fxHeight, {&chorusMixSlider, &chorusRateSlider, &chorusDepthSlider},
                  {&chorusMixLabel, &chorusRateLabel, &chorusDepthLabel});

    // Distortion
    layoutFXBlock(area, 3, fxHeight, {&distMixSlider, &distDriveSlider, &distToneSlider},
                  {&distMixLabel, &distDriveLabel, &distToneLabel});
}

void RavelandAudioProcessorEditor::layoutFXBlock(juce::Rectangle<float>& area, int blockIndex, float height,
                                                std::array<FancyKnob*, 3> knobs, std::array<juce::Label*, 3> labels)
{
    auto blockArea = area.removeFromTop(height).reduced(4, 8);

    // Three knobs per row
    auto knobRow = blockArea.reduced(2, 4);
    const float knobWidth = knobRow.getWidth() / 3.0f;

    for (int i = 0; i < 3; ++i)
    {
        auto knobArea = knobRow.removeFromLeft(knobWidth).reduced(2, 0);
        knobs[i]->setBounds(knobArea.withTrimmedTop(20).toNearestInt());
        labels[i]->setBounds(knobArea.withHeight(16).toNearestInt());
    }

    // Space between blocks
    area.removeFromTop(8);
}

void RavelandAudioProcessorEditor::layoutOscillatorSection(juce::Rectangle<float> area)
{
    // Section title
    auto titleArea = area.removeFromTop(30.0f);
    // Title is drawn in paint()

    // Three oscillator sections
    const float oscHeight = (area.getHeight() - 40) / 3.0f; // Account for master section

    for (int i = 0; i < 3; ++i)
    {
        auto oscArea = area.removeFromTop(oscHeight).reduced(4, 8);

        // Enable button
        oscControls[i].enabled.setBounds(oscArea.removeFromTop(28).toNearestInt());

        // Waveform display
        auto waveformArea = oscArea.removeFromTop(90).reduced(2, 4);
        oscWaveforms[i]->setBounds(waveformArea.toNearestInt());

        // Waveform label
        auto waveformLabelArea = waveformArea.withY(waveformArea.getBottom() - 16).withHeight(14);
        if (i == 0) oscWaveformLabel1.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 1) oscWaveformLabel2.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 2) oscWaveformLabel3.setBounds(waveformLabelArea.toNearestInt());

        // Three knobs with labels
        auto knobArea = oscArea.reduced(2, 4);
        const float knobWidth = knobArea.getWidth() / 3.0f;

        auto voicesArea = knobArea.removeFromLeft(knobWidth).reduced(2, 0);
        oscControls[i].voices.setBounds(voicesArea.withTrimmedTop(20).toNearestInt());
        oscControls[i].voicesLabel.setBounds(voicesArea.withHeight(16).toNearestInt());

        auto detuneArea = knobArea.removeFromLeft(knobWidth).reduced(2, 0);
        oscControls[i].detune.setBounds(detuneArea.withTrimmedTop(20).toNearestInt());
        oscControls[i].detuneLabel.setBounds(detuneArea.withHeight(16).toNearestInt());

        auto levelArea = knobArea.reduced(2, 0);
        oscControls[i].level.setBounds(levelArea.withTrimmedTop(20).toNearestInt());
        oscControls[i].levelLabel.setBounds(levelArea.withHeight(16).toNearestInt());

        // Space between oscillators
        area.removeFromTop(8);
    }

    // Master section at bottom
    auto masterArea = area.removeFromBottom(100).reduced(8, 8);
    masterGainSlider.setBounds(masterArea.withTrimmedTop(20).toNearestInt());
    masterGainLabel.setBounds(masterArea.withHeight(16).toNearestInt());
}

void RavelandAudioProcessorEditor::layoutFooterSection(juce::Rectangle<float> bounds)
{
    auto footer = bounds.removeFromBottom(100.0f).reduced(24, 12);

    // Performance controls
    auto controlsArea = footer.reduced(8, 8);

    monoButton.setBounds(controlsArea.removeFromLeft(120).toNearestInt());
    controlsArea.removeFromLeft(20);

    legatoButton.setBounds(controlsArea.removeFromLeft(120).toNearestInt());
    controlsArea.removeFromLeft(40);

    auto portamentoArea = controlsArea.removeFromLeft(180);
    portamentoSlider.setBounds(portamentoArea.withTrimmedTop(20).toNearestInt());
    portamentoLabel.setBounds(portamentoArea.withHeight(16).toNearestInt());
}
