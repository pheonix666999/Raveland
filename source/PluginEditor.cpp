#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "BinaryData.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_graphics/juce_graphics.h>

static const juce::Colour colourBackground = juce::Colour::fromString("0xff05060b");
static const juce::Colour colourBackground2 = juce::Colour::fromString("0xff070912");
static const juce::Colour colourPanel = juce::Colour::fromRGBA(10, 12, 18, 220);
static const juce::Colour colourPanelAlt = juce::Colour::fromRGBA(7, 9, 12, 190);
static const juce::Colour colourBlue = juce::Colour::fromString("0xff2bb6ff");
static const juce::Colour colourGold = juce::Colour::fromString("0xffa79678");
static const juce::Colour colourGoldBright = juce::Colour::fromString("0xffc3aa86");
static const juce::Colour colourPink = juce::Colour::fromString("0xffff2fb2");
static const juce::Colour colourAccent = juce::Colour::fromString("0xff2bb6ff");
static const juce::Colour colourText = juce::Colour::fromString("0xfff8f6f2");
static const juce::Colour colourTextSecondary = juce::Colour::fromRGBA(248, 246, 242, 160);

void RavelandLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                           bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto switchArea = bounds.removeFromLeft(46.0f).withHeight(24.0f).withY(bounds.getCentreY() - 12.0f);

    const bool isOn = button.getToggleState();
    const float radius = switchArea.getHeight() * 0.5f;

    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRoundedRectangle(switchArea.translated(0, 1), radius);

    g.setColour(isOn ? colourGold.withAlpha(0.22f) : juce::Colours::black.withAlpha(0.20f));
    g.fillRoundedRectangle(switchArea, radius);

    g.setColour(isOn ? colourGoldBright.withAlpha(0.45f) : colourGold.withAlpha(0.20f));
    g.drawRoundedRectangle(switchArea, radius, 1.0f);

    const float knobSize = switchArea.getHeight() - 6.0f;
    const float knobX = isOn ? (switchArea.getRight() - knobSize - 3.0f) : (switchArea.getX() + 3.0f);
    const float knobY = switchArea.getY() + 3.0f;

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(knobX - 1.0f, knobY + 1.0f, knobSize + 2.0f, knobSize + 2.0f);

    juce::ColourGradient knobGrad(juce::Colours::white.withAlpha(0.95f), knobX, knobY,
                                  (isOn ? colourGoldBright : colourGold).withAlpha(0.55f), knobX, knobY + knobSize, false);
    g.setGradientFill(knobGrad);
    g.fillEllipse(knobX, knobY, knobSize, knobSize);

    g.setColour((isOn ? colourGoldBright : colourGold).withAlpha(0.5f));
    g.drawEllipse(knobX, knobY, knobSize, knobSize, 1.0f);

    auto textArea = bounds.withTrimmedLeft(52.0f);
    g.setColour(colourTextSecondary);
    g.setFont(juce::Font(10.0f, juce::Font::bold).withExtraKerningFactor(0.08f));
    g.drawText(button.getButtonText(), textArea, juce::Justification::centredLeft, false);
}

void RavelandLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                               const juce::Colour&, bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    const float radius = 12.0f;
    const bool secondary = button.getProperties().getWithDefault("variant", "primary").toString() == "secondary";

    juce::Colour base = secondary ? colourGold.withAlpha(0.10f) : colourGold.withAlpha(0.18f);
    juce::Colour border = secondary ? colourGold.withAlpha(0.26f) : colourGoldBright.withAlpha(0.28f);
    juce::Colour glow = secondary ? colourGold.withAlpha(0.16f) : colourGoldBright.withAlpha(0.14f);

    if (isDown)
        base = base.brighter(0.08f);
    else if (isHighlighted)
        base = base.brighter(0.04f);

    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRoundedRectangle(bounds.translated(0, 1.2f), radius);

    juce::ColourGradient bg(base, bounds.getX(), bounds.getY(), base.darker(0.3f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, radius);

    g.setColour(border);
    g.drawRoundedRectangle(bounds, radius, 1.0f);

    g.setColour(glow);
    g.drawRoundedRectangle(bounds.reduced(1), radius - 1.0f, 1.0f);
}

void RavelandLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float, float, const juce::Slider::SliderStyle style,
                                           juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height);
        auto track = bounds.reduced(bounds.getWidth() * 0.4f, 6.0f);

        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(track.translated(0, 1.0f), track.getWidth() * 0.5f);

        g.setColour(juce::Colours::black.withAlpha(0.28f));
        g.fillRoundedRectangle(track, track.getWidth() * 0.5f);

        auto fill = track.withY(sliderPos).withHeight(track.getBottom() - sliderPos);
        g.setColour(colourGoldBright.withAlpha(0.55f));
        g.fillRoundedRectangle(fill, track.getWidth() * 0.5f);

        g.setColour(colourGold.withAlpha(0.35f));
        g.drawRoundedRectangle(track, track.getWidth() * 0.5f, 1.0f);

        const float thumbR = 8.0f;
        auto thumbX = track.getCentreX() - thumbR;
        auto thumbY = sliderPos - thumbR;
        juce::ColourGradient thumbGrad(juce::Colours::white.withAlpha(0.95f), thumbX, thumbY,
                                       colourGold.withAlpha(0.6f), thumbX, thumbY + thumbR * 2.0f, false);
        g.setGradientFill(thumbGrad);
        g.fillEllipse(thumbX, thumbY, thumbR * 2.0f, thumbR * 2.0f);
        g.setColour(colourGold.withAlpha(0.4f));
        g.drawEllipse(thumbX, thumbY, thumbR * 2.0f, thumbR * 2.0f, 1.0f);
        return;
    }

    if (style != juce::Slider::LinearHorizontal)
    {
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0, 0, style, slider);
        return;
    }

    auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height);
    auto track = bounds.reduced(6.0f, bounds.getHeight() * 0.4f);

    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRoundedRectangle(track.translated(0, 1.0f), track.getHeight() * 0.5f);

    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(track, track.getHeight() * 0.5f);

    auto filled = track.withWidth(sliderPos - track.getX());
    g.setColour(colourGoldBright.withAlpha(0.35f));
    g.fillRoundedRectangle(filled, track.getHeight() * 0.5f);

    g.setColour(colourGold.withAlpha(0.18f));
    g.drawRoundedRectangle(track, track.getHeight() * 0.5f, 1.0f);

    const float thumbR = 9.0f;
    auto thumbX = sliderPos - thumbR;
    auto thumbY = track.getCentreY() - thumbR;
    juce::ColourGradient thumbGrad(juce::Colours::white.withAlpha(0.95f), thumbX, thumbY,
                                   colourGold.withAlpha(0.55f), thumbX, thumbY + thumbR * 2.0f, false);
    g.setGradientFill(thumbGrad);
    g.fillEllipse(thumbX, thumbY, thumbR * 2.0f, thumbR * 2.0f);
    g.setColour(colourBlue.withAlpha(0.25f));
    g.drawEllipse(thumbX, thumbY, thumbR * 2.0f, thumbR * 2.0f, 1.0f);
}

void RavelandLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox&)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    const float radius = bounds.getHeight() * 0.5f;

    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRoundedRectangle(bounds.translated(0, 1.0f), radius);

    juce::ColourGradient bg(colourPanel, bounds.getX(), bounds.getY(),
                            colourPanelAlt, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, radius);

    g.setColour(colourGold.withAlpha(0.26f));
    g.drawRoundedRectangle(bounds, radius, 1.0f);

    auto arrowArea = bounds.removeFromRight(24.0f);
    g.setColour(colourGold.withAlpha(0.8f));
    juce::Path arrow;
    const float midX = arrowArea.getCentreX();
    const float midY = arrowArea.getCentreY();
    arrow.startNewSubPath(midX - 5.0f, midY - 2.0f);
    arrow.lineTo(midX, midY + 4.0f);
    arrow.lineTo(midX + 5.0f, midY - 2.0f);
    g.strokePath(arrow, juce::PathStrokeType(1.6f));

    g.setColour(colourGold.withAlpha(0.14f));
    g.drawLine(arrowArea.getX() - 2.0f, bounds.getY() + 6.0f, arrowArea.getX() - 2.0f, bounds.getBottom() - 6.0f, 1.0f);

    g.setColour(colourGold.withAlpha(0.14f));
    g.drawRoundedRectangle(bounds.reduced(1.2f), radius - 1.0f, 1.0f);

    // Accent dot (top-right)
    g.setColour(colourGoldBright.withAlpha(0.9f));
    g.fillEllipse(bounds.getRight() - 10.0f, bounds.getY() + 6.0f, 4.0f, 4.0f);
}

void RavelandLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    const float radius = 12.0f;
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillRoundedRectangle(bounds.translated(0, 1.5f), radius);

    juce::ColourGradient bg(juce::Colour::fromRGB(10, 12, 16), bounds.getX(), bounds.getY(),
                            juce::Colour::fromRGB(6, 7, 10), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, radius);

        g.setColour(colourGold.withAlpha(0.25f));
        g.drawRoundedRectangle(bounds, radius, 1.0f);

        juce::ColourGradient glow(colourGoldBright.withAlpha(0.22f), bounds.getCentreX(), bounds.getY(),
                              juce::Colours::transparentBlack, bounds.getCentreX(), bounds.getY() + 24.0f, false);
        g.setGradientFill(glow);
        g.fillRoundedRectangle(bounds.reduced(2.0f), radius - 2.0f);
}

void RavelandLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                                            bool, const juce::String& text,
                                            const juce::String&, const juce::Drawable*,
                                            const juce::Colour*)
{
    if (isSeparator)
    {
        g.setColour(colourBlue.withAlpha(0.2f));
        g.drawLine((float) area.getX() + 8.0f, (float) area.getCentreY(),
                   (float) area.getRight() - 8.0f, (float) area.getCentreY());
        return;
    }

    auto bounds = area.toFloat().reduced(4, 2);
    if (isHighlighted && isActive)
    {
        g.setColour(colourGold.withAlpha(0.18f));
        g.fillRoundedRectangle(bounds, 8.0f);
    }

    g.setColour(isActive ? colourGoldBright : colourTextSecondary);
    g.setFont(juce::Font(12.0f, juce::Font::bold).withExtraKerningFactor(0.05f));
    g.drawText(text, bounds, juce::Justification::centredLeft, false);

    if (isTicked)
    {
        g.setColour(colourGoldBright);
        g.fillEllipse(bounds.getRight() - 14.0f, bounds.getCentreY() - 4.0f, 8.0f, 8.0f);
    }
}

void RavelandAudioProcessorEditor::loadLogos()
{
    ravelandLogoDrawable = juce::Drawable::createFromImageData(BinaryData::raveland_logo_jpg,
                                                               BinaryData::raveland_logo_jpgSize);
    nsAudioLogoDrawable = juce::Drawable::createFromImageData(BinaryData::ns_logo_png,
                                                              BinaryData::ns_logo_pngSize);

    if (ravelandLogoDrawable == nullptr)
        ravelandLogoDrawable = juce::Drawable::createFromImageData(BinaryData::raveland_svg,
                                                                   BinaryData::raveland_svgSize);
    if (nsAudioLogoDrawable == nullptr)
        nsAudioLogoDrawable = juce::Drawable::createFromImageData(BinaryData::nsaudio_svg,
                                                                  BinaryData::nsaudio_svgSize);
}

void RavelandAudioProcessorEditor::drawRaveLandLogo(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (ravelandLogoDrawable != nullptr)
    {
        ravelandLogoDrawable->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
        return;
    }

    juce::ColourGradient textGrad(colourGold, bounds.getX(), bounds.getY(),
                                  colourGoldBright.withAlpha(0.9f), bounds.getRight(), bounds.getY(), false);
    g.setGradientFill(textGrad);
    g.setFont(juce::Font(32.0f, juce::Font::italic | juce::Font::bold).withExtraKerningFactor(0.1f));
    g.drawText("RaveLand", bounds, juce::Justification::centred, false);
}

void RavelandAudioProcessorEditor::refreshSampleSetCombos()
{
    const auto& names = processor.getSampleSetNames();
    for (int i = 0; i < 3; ++i)
    {
        auto& combo = layerControls[i].sampleSet;
        combo.clear();

        if (names.isEmpty())
        {
            combo.addItem("No Sample Sets Found", 1);
            combo.setSelectedId(1, juce::dontSendNotification);
            combo.setEnabled(false);
            continue;
        }

        for (int n = 0; n < names.size(); ++n)
            combo.addItem(names[n], n + 1);

        const int selected = processor.getSampleSetIndex(i);
        combo.setSelectedId(selected + 1, juce::dontSendNotification);
        combo.setEnabled(true);
    }
}

void RavelandAudioProcessorEditor::drawNeonGlow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float pulse = 0.6f + 0.4f * std::sin(juce::Time::getMillisecondCounterHiRes() * 0.0007);

    juce::ColourGradient grad1(colourGold.withAlpha(0.10f * pulse), cx, bounds.getY() + bounds.getHeight() * 0.12f,
                               juce::Colours::transparentBlack, cx, cy + bounds.getHeight() * 0.45f, true);
    g.setGradientFill(grad1);
    g.fillRect(bounds);

    juce::ColourGradient grad2(colourGold.withAlpha(0.06f * pulse), bounds.getX() + bounds.getWidth() * 0.2f, bounds.getY() + bounds.getHeight() * 0.2f,
                               juce::Colours::transparentBlack, cx, cy, true);
    g.setGradientFill(grad2);
    g.fillRect(bounds);
}

void RavelandAudioProcessorEditor::drawPanelWithGlow(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& title)
{
    const float radius = 16.0f;
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillRoundedRectangle(bounds.translated(2, 3), radius);

    juce::ColourGradient cardBg(colourPanel, bounds.getX(), bounds.getY(),
                                colourPanelAlt, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(cardBg);
    g.fillRoundedRectangle(bounds, radius);

    g.setColour(colourGold.withAlpha(0.22f));
    g.drawRoundedRectangle(bounds, radius, 1.0f);

    g.setColour(colourGold.withAlpha(0.10f));
    g.drawRoundedRectangle(bounds.reduced(1), radius - 1.0f, 1.0f);

    auto titleArea = bounds.removeFromTop(20.0f);
    g.setColour(colourText);
    g.setFont(juce::Font(11.0f, juce::Font::bold).withExtraKerningFactor(0.14f));
    g.drawText(title, titleArea.reduced(12, 2), juce::Justification::centredLeft, false);

    auto lineArea = titleArea;
    lineArea.removeFromLeft(120.0f);
    g.setColour(colourGold.withAlpha(0.45f));
    g.fillRect(lineArea.withHeight(2.0f).withY(titleArea.getCentreY() + 6.0f));
}

void RavelandAudioProcessorEditor::setupToggle(juce::ToggleButton& button)
{
    button.setColour(juce::ToggleButton::textColourId, colourTextSecondary);
    button.setColour(juce::ToggleButton::tickColourId, colourGold);
    button.setColour(juce::ToggleButton::tickDisabledColourId, colourGold.withAlpha(0.2f));
}

RavelandAudioProcessorEditor::RavelandAudioProcessorEditor(RavelandAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lookAndFeel);
    loadLogos();
    startTimerHz(60); // 60fps for smooth animations

    auto& vts = processor.getValueTreeState();
    auto fmtInt = [](float v) { return juce::String((int) std::round(v)); };
    auto fmtPercent = [](float v) { return juce::String((int) std::round(v * 100.0f)); };
    auto fmtSignedPercent = [](float v) { return juce::String((int) std::round(v * 100.0f)); };
    auto fmtHz = [](float v) { return juce::String((int) std::round(v)); };
    auto fmtMs = [](float v) { return juce::String((int) std::round(v)); };
    auto fmtDb = [](float v) { return juce::String(v, 1); };
    auto fmtRate = [](float v)
    {
        if (std::abs(v) < 10.0f)
            return juce::String(v, 2);
        return juce::String(v, 1);
    };
    auto fmtPortaMs = [](float v) { return juce::String((int) std::round(v * 350.0f)); };
    auto fmtVoices = [](float v) { return juce::String((int) std::round(v)) + " v"; };
    auto fmtDetune = [](float v) { return juce::String((int) std::round(v)) + "%"; };
    auto fmtSemis = [](float v)
    {
        const int iv = (int) std::round(v);
        return juce::String(iv >= 0 ? "+" : "") + juce::String(iv) + " st";
    };
    auto fmtCents = [](float v)
    {
        const int iv = (int) std::round(v);
        return juce::String(iv >= 0 ? "+" : "") + juce::String(iv) + " ct";
    };
    auto fmtLevel = [](float v) { return juce::String((int) std::round(v * 100.0f)) + "%"; };

    // Create waveform displays
    for (int i = 0; i < 3; ++i)
    {
        oscWaveforms[i] = std::make_unique<WaveformDisplay>();
        oscWaveforms[i]->setDisplayMode(WaveformDisplay::DisplayMode::Osc);
        addAndMakeVisible(oscWaveforms[i].get());
        
        layerWaveforms[i] = std::make_unique<WaveformDisplay>();
        layerWaveforms[i]->setDisplayMode(WaveformDisplay::DisplayMode::Stack);
        addAndMakeVisible(layerWaveforms[i].get());
    }

    modWaveform = std::make_unique<WaveformDisplay>();
    modWaveform->setDisplayMode(WaveformDisplay::DisplayMode::Mod);
    modWaveform->setWaveformType(2);
    addAndMakeVisible(modWaveform.get());

    // Preset browser
    presetCombo.addItemList(processor.getPresetNames(), 1);
    presetCombo.setSelectedId(processor.getCurrentPresetIndex() + 1);
    presetCombo.setColour(juce::ComboBox::backgroundColourId, colourPanel);
    presetCombo.setColour(juce::ComboBox::textColourId, colourText);
    presetCombo.setColour(juce::ComboBox::outlineColourId, colourGold.withAlpha(0.42f));
    presetCombo.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
    presetCombo.setJustificationType(juce::Justification::centred);
    presetCombo.onChange = [this] { processor.setCurrentProgram(presetCombo.getSelectedId() - 1); };
    addAndMakeVisible(presetCombo);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setFont(juce::Font(9.0f, juce::Font::bold).withExtraKerningFactor(0.12f));
    presetLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(presetLabel);

    randomizeButton.setColour(juce::TextButton::buttonColourId, colourBlue.withAlpha(0.14f));
    randomizeButton.setColour(juce::TextButton::textColourOffId, colourText);
    randomizeButton.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
    addAndMakeVisible(randomizeButton);

    resetButton.setColour(juce::TextButton::buttonColourId, colourGold.withAlpha(0.12f));
    resetButton.setColour(juce::TextButton::textColourOffId, colourText);
    resetButton.setColour(juce::ComboBox::outlineColourId, colourGold.withAlpha(0.24f));
    resetButton.getProperties().set("variant", "secondary");
    addAndMakeVisible(resetButton);

    modShapeButton.setColour(juce::TextButton::buttonColourId, colourGold.withAlpha(0.12f));
    modShapeButton.setColour(juce::TextButton::textColourOffId, colourText);
    modShapeButton.setColour(juce::ComboBox::outlineColourId, colourGold.withAlpha(0.24f));
    modShapeButton.getProperties().set("variant", "secondary");
    addAndMakeVisible(modShapeButton);

    modTriggerButton.setColour(juce::TextButton::buttonColourId, colourGold.withAlpha(0.12f));
    modTriggerButton.setColour(juce::TextButton::textColourOffId, colourText);
    modTriggerButton.setColour(juce::ComboBox::outlineColourId, colourGold.withAlpha(0.24f));
    modTriggerButton.getProperties().set("variant", "secondary");
    addAndMakeVisible(modTriggerButton);

    modModeCombo.addItemList(juce::StringArray { "LFO", "ENV" }, 1);
    modModeCombo.setColour(juce::ComboBox::backgroundColourId, colourPanel);
    modModeCombo.setColour(juce::ComboBox::textColourId, colourText);
    modModeCombo.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
    modModeCombo.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
    addAndMakeVisible(modModeCombo);

    modTargetCombo.addItemList(juce::StringArray { "None", "OSC1 Detune", "OSC2 Detune", "OSC3 Detune",
                                                   "Filter Cutoff", "Dist Drive", "Layer A Start Rand" }, 1);
    modTargetCombo.setColour(juce::ComboBox::backgroundColourId, colourPanel);
    modTargetCombo.setColour(juce::ComboBox::textColourId, colourText);
    modTargetCombo.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
    modTargetCombo.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
    addAndMakeVisible(modTargetCombo);

    addAndMakeVisible(modRateKnob);
    modRateKnob.setFormatter(fmtRate);
    modRateLabel.setText("RATE", juce::dontSendNotification);
    modRateLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    modRateLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    modRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modRateLabel);

    addAndMakeVisible(modAmountKnob);
    modAmountKnob.setFormatter(fmtInt);
    modAmountLabel.setText("AMOUNT", juce::dontSendNotification);
    modAmountLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    modAmountLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    modAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modAmountLabel);

    modModeLabel.setText("MODE", juce::dontSendNotification);
    modModeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    modModeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    modModeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(modModeLabel);

    modTargetLabel.setText("TARGET", juce::dontSendNotification);
    modTargetLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    modTargetLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    modTargetLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(modTargetLabel);

    modHintLabel.setText("Use the Target selector to route modulation.", juce::dontSendNotification);
    modHintLabel.setFont(juce::Font(9.0f));
    modHintLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    modHintLabel.setJustificationType(juce::Justification::topLeft);
    modHintLabel.setMinimumHorizontalScale(0.7f);
    addAndMakeVisible(modHintLabel);

    modModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(vts, "modMode", modModeCombo);
    modTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(vts, "modTarget", modTargetCombo);
    modRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "modRate", modRateKnob);
    modAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "modAmount", modAmountKnob);

    // Master
    addAndMakeVisible(masterGainSlider);
    masterGainSlider.setFormatter(fmtDb);
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
        osc.enabled.setButtonText("On");
        addAndMakeVisible(osc.enabled);

        osc.waveform.addItemList(juce::StringArray { "Saw (A)", "Pulse", "Sine", "Triangle", "Saw (B)", "Saw (C)" }, 1);
        osc.waveform.setColour(juce::ComboBox::backgroundColourId, colourPanel);
        osc.waveform.setColour(juce::ComboBox::textColourId, colourText);
        osc.waveform.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
        osc.waveform.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
        addAndMakeVisible(osc.waveform);

        osc.waveformLabel.setText("WAVE", juce::dontSendNotification);
        osc.waveformLabel.setFont(juce::Font(8.0f, juce::Font::bold));
        osc.waveformLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.waveformLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.waveformLabel);

        addAndMakeVisible(osc.voices);
        osc.voices.setFormatter(fmtVoices);
        osc.voicesLabel.setText("UNISON", juce::dontSendNotification);
        osc.voicesLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.voicesLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.voicesLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.voicesLabel);

        addAndMakeVisible(osc.detune);
        osc.detune.setFormatter(fmtDetune);
        osc.detuneLabel.setText("DETUNE", juce::dontSendNotification);
        osc.detuneLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.detuneLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.detuneLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.detuneLabel);

        addAndMakeVisible(osc.semi);
        osc.semi.setFormatter(fmtSemis);
        osc.semiLabel.setText("PITCH", juce::dontSendNotification);
        osc.semiLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.semiLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.semiLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.semiLabel);

        addAndMakeVisible(osc.fine);
        osc.fine.setFormatter(fmtCents);
        osc.fineLabel.setText("FINE", juce::dontSendNotification);
        osc.fineLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.fineLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.fineLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.fineLabel);

        addAndMakeVisible(osc.level);
        osc.level.setFormatter(fmtLevel);
        osc.levelLabel.setText("LEVEL", juce::dontSendNotification);
        osc.levelLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        osc.levelLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        osc.levelLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(osc.levelLabel);

        osc.enabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, prefix + "Enabled", osc.enabled);
        osc.waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(vts, prefix + "Wave", osc.waveform);
        osc.voicesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Voices", osc.voices);
        osc.detuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Detune", osc.detune);
        osc.semiAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Semi", osc.semi);
        osc.fineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Fine", osc.fine);
        osc.levelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Level", osc.level);

        osc.waveform.onChange = [this, i]()
        {
            oscWaveforms[i]->setWaveformType(oscControls[i].waveform.getSelectedItemIndex());
        };
        oscWaveforms[i]->setWaveformType(osc.waveform.getSelectedItemIndex());
    }

    // Layers
    for (int i = 0; i < 3; ++i)
    {
        auto prefix = "layer" + juce::String(i + 1);
        auto& layer = layerControls[i];

        setupToggle(layer.enabled);
        layer.enabled.setButtonText("On");
        addAndMakeVisible(layer.enabled);

        layer.sampleSetLabel.setText("STACK", juce::dontSendNotification);
        layer.sampleSetLabel.setFont(juce::Font(8.0f, juce::Font::bold));
        layer.sampleSetLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        layer.sampleSetLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(layer.sampleSetLabel);

        layer.sampleSet.setColour(juce::ComboBox::backgroundColourId, colourPanel);
        layer.sampleSet.setColour(juce::ComboBox::textColourId, colourText);
        layer.sampleSet.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
        layer.sampleSet.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
        addAndMakeVisible(layer.sampleSet);

        layer.sampleSet.onChange = [this, i]()
        {
            const int idx = layerControls[i].sampleSet.getSelectedItemIndex();
            if (idx >= 0)
                processor.setSampleSetIndex(i, idx);
        };

        addAndMakeVisible(layer.tune);
        layer.tune.setFormatter(fmtInt);
        layer.tuneLabel.setText("TUNE", juce::dontSendNotification);
        layer.tuneLabel.setFont(juce::Font(9.0f, juce::Font::bold));
        layer.tuneLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        layer.tuneLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(layer.tuneLabel);

        addAndMakeVisible(layer.startRand);
        layer.startRand.setFormatter(fmtInt);
        layer.startRandLabel.setText("START RAND", juce::dontSendNotification);
        layer.startRandLabel.setFont(juce::Font(8.0f, juce::Font::bold));
        layer.startRandLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        layer.startRandLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(layer.startRandLabel);

        layer.enabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, prefix + "Enabled", layer.enabled);
        layer.tuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Tune", layer.tune);
        layer.startRandAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "StartRand", layer.startRand);

        layer.attack.setSliderStyle(juce::Slider::LinearHorizontal);
        layer.attack.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(layer.attack);
        layer.attackLabel.setText("ATTACK", juce::dontSendNotification);
        layer.attackLabel.setFont(juce::Font(8.0f, juce::Font::bold));
        layer.attackLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        layer.attackLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(layer.attackLabel);

        layer.release.setSliderStyle(juce::Slider::LinearHorizontal);
        layer.release.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(layer.release);
        layer.releaseLabel.setText("RELEASE", juce::dontSendNotification);
        layer.releaseLabel.setFont(juce::Font(8.0f, juce::Font::bold));
        layer.releaseLabel.setColour(juce::Label::textColourId, colourTextSecondary);
        layer.releaseLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(layer.releaseLabel);

        layer.attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Attack", layer.attack);
        layer.releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, prefix + "Release", layer.release);
    }

    refreshSampleSetCombos();

    // FX
    filterTitleLabel.setText("FILTER (PRE)", juce::dontSendNotification);
    filterTitleLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    filterTitleLabel.setColour(juce::Label::textColourId, colourText);
    filterTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(filterTitleLabel);

    setupToggle(filterEnabled);
    filterEnabled.setButtonText("On");
    addAndMakeVisible(filterEnabled);
    filterEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "filterEnabled", filterEnabled);

    filterTypeLabel.setText("TYPE", juce::dontSendNotification);
    filterTypeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    filterTypeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    filterTypeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(filterTypeLabel);

    filterTypeCombo.addItemList(juce::StringArray { "Lowpass", "Highpass", "Bandpass" }, 1);
    filterTypeCombo.setColour(juce::ComboBox::backgroundColourId, colourPanel);
    filterTypeCombo.setColour(juce::ComboBox::textColourId, colourText);
    filterTypeCombo.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
    filterTypeCombo.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
    addAndMakeVisible(filterTypeCombo);
    filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(vts, "filterType", filterTypeCombo);

    addAndMakeVisible(filterCutoffSlider);
    filterCutoffSlider.setFormatter(fmtHz);
    filterCutoffLabel.setText("CUTOFF", juce::dontSendNotification);
    filterCutoffLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    filterCutoffLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    filterCutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterCutoffLabel);
    filterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "filterCutoff", filterCutoffSlider);

    addAndMakeVisible(filterResoSlider);
    filterResoSlider.setFormatter(fmtInt);
    filterResoLabel.setText("RESO", juce::dontSendNotification);
    filterResoLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    filterResoLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    filterResoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterResoLabel);
    filterResoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "filterReso", filterResoSlider);

    chorusTitleLabel.setText("CHORUS", juce::dontSendNotification);
    chorusTitleLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    chorusTitleLabel.setColour(juce::Label::textColourId, colourText);
    chorusTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(chorusTitleLabel);

    setupToggle(chorusEnabled);
    chorusEnabled.setButtonText("On");
    addAndMakeVisible(chorusEnabled);
    chorusEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "chorusEnabled", chorusEnabled);

    addAndMakeVisible(chorusRateSlider);
    chorusRateSlider.setFormatter(fmtRate);
    chorusRateLabel.setText("RATE", juce::dontSendNotification);
    chorusRateLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    chorusRateLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    chorusRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(chorusRateLabel);
    chorusRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "chorusRate", chorusRateSlider);

    addAndMakeVisible(chorusMixSlider);
    chorusMixSlider.setFormatter(fmtPercent);
    chorusMixLabel.setText("MIX", juce::dontSendNotification);
    chorusMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    chorusMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    chorusMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(chorusMixLabel);
    chorusMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "chorusMix", chorusMixSlider);
    chorusDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "chorusDepth", chorusDepthSlider);

    delayTitleLabel.setText("DELAY", juce::dontSendNotification);
    delayTitleLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    delayTitleLabel.setColour(juce::Label::textColourId, colourText);
    delayTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(delayTitleLabel);

    setupToggle(delayEnabled);
    delayEnabled.setButtonText("On");
    addAndMakeVisible(delayEnabled);
    delayEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "delayEnabled", delayEnabled);

    addAndMakeVisible(delayTimeSlider);
    delayTimeSlider.setFormatter(fmtMs);
    delayTimeLabel.setText("TIME", juce::dontSendNotification);
    delayTimeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    delayTimeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayTimeLabel);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "delayTime", delayTimeSlider);

    addAndMakeVisible(delayFeedbackSlider);
    delayFeedbackSlider.setFormatter(fmtPercent);
    delayFeedbackLabel.setText("FEEDBACK", juce::dontSendNotification);
    delayFeedbackLabel.setFont(juce::Font(7.0f, juce::Font::bold));
    delayFeedbackLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    delayFeedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayFeedbackLabel);
    delayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "delayFeedback", delayFeedbackSlider);

    delayMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    delayMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayMixSlider);
    delayMixLabel.setText("MIX", juce::dontSendNotification);
    delayMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    delayMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    delayMixLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(delayMixLabel);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "delayMix", delayMixSlider);

    reverbTitleLabel.setText("REVERB", juce::dontSendNotification);
    reverbTitleLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    reverbTitleLabel.setColour(juce::Label::textColourId, colourText);
    reverbTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(reverbTitleLabel);

    setupToggle(reverbEnabled);
    reverbEnabled.setButtonText("On");
    addAndMakeVisible(reverbEnabled);
    reverbEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "reverbEnabled", reverbEnabled);

    reverbModeLabel.setText("MODE", juce::dontSendNotification);
    reverbModeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    reverbModeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    reverbModeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(reverbModeLabel);

    reverbModeCombo.addItemList(juce::StringArray { "Hall", "Room" }, 1);
    reverbModeCombo.setColour(juce::ComboBox::backgroundColourId, colourPanel);
    reverbModeCombo.setColour(juce::ComboBox::textColourId, colourText);
    reverbModeCombo.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
    reverbModeCombo.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
    addAndMakeVisible(reverbModeCombo);
    reverbModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(vts, "reverbMode", reverbModeCombo);

    addAndMakeVisible(reverbSizeSlider);
    reverbSizeSlider.setFormatter(fmtPercent);
    reverbSizeLabel.setText("SIZE", juce::dontSendNotification);
    reverbSizeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    reverbSizeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    reverbSizeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(reverbSizeLabel);
    reverbSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "reverbSize", reverbSizeSlider);

    addAndMakeVisible(reverbDampSlider);
    reverbDampSlider.setFormatter(fmtPercent);
    reverbDampLabel.setText("DAMP", juce::dontSendNotification);
    reverbDampLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    reverbDampLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    reverbDampLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(reverbDampLabel);
    reverbDampAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "reverbDamp", reverbDampSlider);

    reverbMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    reverbMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(reverbMixSlider);
    reverbMixLabel.setText("MIX", juce::dontSendNotification);
    reverbMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    reverbMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    reverbMixLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(reverbMixLabel);
    reverbMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "reverbMix", reverbMixSlider);

    distTitleLabel.setText("DISTORTION", juce::dontSendNotification);
    distTitleLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    distTitleLabel.setColour(juce::Label::textColourId, colourText);
    distTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(distTitleLabel);

    setupToggle(distEnabled);
    distEnabled.setButtonText("On");
    addAndMakeVisible(distEnabled);
    distEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "distEnabled", distEnabled);

    distTypeLabel.setText("TYPE", juce::dontSendNotification);
    distTypeLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    distTypeLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    distTypeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(distTypeLabel);

    distTypeCombo.addItemList(juce::StringArray { "Soft", "Hard", "Tape" }, 1);
    distTypeCombo.setColour(juce::ComboBox::backgroundColourId, colourPanel);
    distTypeCombo.setColour(juce::ComboBox::textColourId, colourText);
    distTypeCombo.setColour(juce::ComboBox::outlineColourId, colourBlue.withAlpha(0.22f));
    distTypeCombo.setColour(juce::ComboBox::arrowColourId, colourTextSecondary);
    addAndMakeVisible(distTypeCombo);
    distTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(vts, "distType", distTypeCombo);

    addAndMakeVisible(distDriveSlider);
    distDriveSlider.setFormatter(fmtPercent);
    distDriveLabel.setText("DRIVE", juce::dontSendNotification);
    distDriveLabel.setFont(juce::Font(7.0f, juce::Font::bold));
    distDriveLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    distDriveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distDriveLabel);
    distDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "distDrive", distDriveSlider);

    addAndMakeVisible(distToneSlider);
    distToneSlider.setFormatter(fmtSignedPercent);
    distToneLabel.setText("TONE", juce::dontSendNotification);
    distToneLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    distToneLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    distToneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distToneLabel);
    distToneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "distTone", distToneSlider);

    distMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    distMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(distMixSlider);
    distMixLabel.setText("MIX", juce::dontSendNotification);
    distMixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    distMixLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    distMixLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(distMixLabel);
    distMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "distMix", distMixSlider);

    routingTitleLabel.setText("ROUTING", juce::dontSendNotification);
    routingTitleLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    routingTitleLabel.setColour(juce::Label::textColourId, colourText);
    routingTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(routingTitleLabel);

    routingListLabel.setText("Filter\nChorus\nDelay\nReverb\nDistortion", juce::dontSendNotification);
    routingListLabel.setFont(juce::Font(10.0f));
    routingListLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    routingListLabel.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(routingListLabel);

    // Mono/Legato
    setupToggle(monoButton);
    monoButton.setButtonText("Mono");
    addAndMakeVisible(monoButton);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "monoEnabled", monoButton);

    setupToggle(legatoButton);
    legatoButton.setButtonText("Legato");
    addAndMakeVisible(legatoButton);
    legatoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "legatoEnabled", legatoButton);

    pitchWheel.setSliderStyle(juce::Slider::LinearVertical);
    pitchWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pitchWheel.setRange(-1.0, 1.0, 0.01);
    pitchWheel.setValue(0.0);
    pitchWheel.setColour(juce::Slider::trackColourId, colourBlue.withAlpha(0.35f));
    pitchWheel.setColour(juce::Slider::thumbColourId, colourGold);
    addAndMakeVisible(pitchWheel);

    pitchLabel.setText("PITCH", juce::dontSendNotification);
    pitchLabel.setFont(juce::Font(10.0f, juce::Font::bold).withExtraKerningFactor(0.1f));
    pitchLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    pitchLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(pitchLabel);

    modWheel.setSliderStyle(juce::Slider::LinearVertical);
    modWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    modWheel.setRange(0.0, 1.0, 0.01);
    modWheel.setValue(0.0);
    modWheel.setColour(juce::Slider::trackColourId, colourBlue.withAlpha(0.35f));
    modWheel.setColour(juce::Slider::thumbColourId, colourGold);
    addAndMakeVisible(modWheel);

    modLabel.setText("MOD", juce::dontSendNotification);
    modLabel.setFont(juce::Font(10.0f, juce::Font::bold).withExtraKerningFactor(0.1f));
    modLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    modLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modLabel);

    addAndMakeVisible(portamentoSlider);
    portamentoSlider.setFormatter(fmtPortaMs);
    portamentoLabel.setText("PORTAMENTO", juce::dontSendNotification);
    portamentoLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    portamentoLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    portamentoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(portamentoLabel);
    portamentoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "portamento", portamentoSlider);

    addAndMakeVisible(glideCurveSlider);
    glideCurveSlider.setFormatter(fmtInt);
    glideCurveLabel.setText("GLIDE CURVE", juce::dontSendNotification);
    glideCurveLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    glideCurveLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    glideCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(glideCurveLabel);
    glideCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "glideCurve", glideCurveSlider);

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

    modWaveformLabel.setText("", juce::dontSendNotification);
    modWaveformLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    modWaveformLabel.setColour(juce::Label::textColourId, colourTextSecondary);
    modWaveformLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modWaveformLabel);

    // Set default size last so `resized()` can safely layout child components.
    const auto display = juce::Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    const int minW = 1200;
    const int maxW = 1800;
    const int minH = 820;
    const int maxH = 1080;
    const int maxAvailW = juce::jmax(800, display.getWidth() - 40);
    const int maxAvailH = juce::jmax(600, display.getHeight() - 40);
    const int minWClamped = juce::jmin(minW, maxAvailW);
    const int minHClamped = juce::jmin(minH, maxAvailH);
    const int targetW = juce::jlimit(minWClamped, maxW, maxAvailW);
    const int targetH = juce::jlimit(minHClamped, maxH, maxAvailH);
    setResizable(true, true);
    setResizeLimits(minWClamped, minHClamped, maxW, maxH);
    setSize(targetW, targetH);
}

RavelandAudioProcessorEditor::~RavelandAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
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
        
        if (ravelandLogoDrawable != nullptr)
        {
            auto scaledBounds = juce::Rectangle<float>(0, 0, 64, 64);
            ravelandLogoDrawable->drawWithin(iconG, scaledBounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else
        {
            iconG.setColour(colourGold);
            iconG.setFont(juce::Font(20.0f, juce::Font::bold));
            iconG.drawText("RL", juce::Rectangle<float>(0, 0, 64, 64), juce::Justification::centred, false);

            iconG.setColour(colourAccent);
            iconG.drawEllipse(18, 18, 28, 28, 2.0f);
        }
        
        getPeer()->setIcon(iconImage);
        iconSet = true;
    }
    
    auto bounds = getLocalBounds().toFloat();
    
    // Dark background
    juce::ColourGradient bgGrad(colourBackground2, bounds.getX(), bounds.getY(),
                                 colourBackground, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Subtle gold glow
    drawNeonGlow(g, bounds);

    // Modern card-style main container
    auto pluginBounds = bounds.reduced(24, 24);

    // Sleek shadow effect
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillRoundedRectangle(pluginBounds.translated(3, 4), 18.0f);

    // Main surface with subtle gradient
    juce::ColourGradient surfaceGrad(colourPanel, pluginBounds.getX(), pluginBounds.getY(),
                                      colourPanelAlt, pluginBounds.getX(), pluginBounds.getBottom(), false);
    g.setGradientFill(surfaceGrad);
    g.fillRoundedRectangle(pluginBounds, 16.0f);

    // Modern border with subtle glow
    g.setColour(colourGold.withAlpha(0.22f));
    g.drawRoundedRectangle(pluginBounds, 16.0f, 1.0f);

    // Accent border
    g.setColour(colourGold.withAlpha(0.10f));
    g.drawRoundedRectangle(pluginBounds.reduced(1), 15.0f, 0.6f);

    // Header
    const float headerHeight = 112.0f;
    auto header = pluginBounds.removeFromTop(headerHeight);

    // Header background with subtle gradient
    juce::ColourGradient headerBg(colourPanel.brighter(0.02f), header.getX(), header.getY(),
                                   colourPanelAlt, header.getX(), header.getBottom(), false);
    g.setGradientFill(headerBg);
    g.fillRoundedRectangle(header, 16.0f);

    // Clean separator line
    g.setColour(colourGold.withAlpha(0.16f));
    g.fillRect(header.getX() + 18, header.getBottom() - 1, header.getWidth() - 36, 1.0f);

    // NS Audio logo (left)
    if (nsAudioLogoDrawable != nullptr)
    {
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        auto logoBounds = juce::Rectangle<float>(28, header.getY() + 10, 68, 68);
        auto logoPill = logoBounds.expanded(10.0f, 8.0f);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(logoPill.translated(0, 1.5f), 12.0f);
        juce::ColourGradient pillGrad(colourPanel.brighter(0.02f), logoPill.getX(), logoPill.getY(),
                                      colourPanelAlt, logoPill.getX(), logoPill.getBottom(), false);
        g.setGradientFill(pillGrad);
        g.fillRoundedRectangle(logoPill, 12.0f);
        g.setColour(colourGold.withAlpha(0.22f));
        g.drawRoundedRectangle(logoPill, 12.0f, 1.0f);

        nsAudioLogoDrawable->drawWithin(g, logoBounds.reduced(4), juce::RectanglePlacement::centred, 1.0f);
        g.setImageResamplingQuality(juce::Graphics::mediumResamplingQuality);
    }
    else
    {
        g.setColour(colourTextSecondary);
        g.setFont(juce::Font(12.0f, juce::Font::bold).withExtraKerningFactor(0.1f));
        g.drawText("NS AUDIO", juce::Rectangle<float>(32, header.getY() + 30, 160, 20),
                    juce::Justification::centredLeft, false);
    }

    // RaveLand logo (center)
    auto titleBounds = juce::Rectangle<float>(header.getCentreX() - 140, header.getY() + 6, 280, 60);
    auto titlePlate = titleBounds.expanded(10.0f, 8.0f);
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRoundedRectangle(titlePlate.translated(0, 1.5f), 14.0f);
    g.setColour(colourPanel);
    g.fillRoundedRectangle(titlePlate, 14.0f);
    g.setColour(colourGold.withAlpha(0.18f));
    g.drawRoundedRectangle(titlePlate, 14.0f, 1.0f);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    drawRaveLandLogo(g, titleBounds);
    g.setImageResamplingQuality(juce::Graphics::mediumResamplingQuality);

    // Version info (right)
    g.setColour(colourTextSecondary);
    g.setFont(juce::Font(9.0f).withExtraKerningFactor(0.12f));
    g.drawText("RAVELAND V1", juce::Rectangle<float>(header.getRight() - 140, header.getY() + 22, 120, 16),
                juce::Justification::centredRight, false);

    // Panels (outer)
    if (leftPanelBounds.getWidth() > 0.0f)
    {
        drawPanelWithGlow(g, leftPanelBounds, "AUDIO LAYERS");
        g.setColour(colourTextSecondary);
        g.setFont(juce::Font(10.0f).withExtraKerningFactor(0.05f));
        g.drawText("Pre-layered stacks (wav multisamples)", leftPanelBounds.withTrimmedTop(20).reduced(12, 4),
                    juce::Justification::centredLeft, false);
    }

    if (perfPanelBounds.getWidth() > 0.0f)
        drawPanelWithGlow(g, perfPanelBounds, "PERFORMANCE");

    if (modPanelBounds.getWidth() > 0.0f)
    {
        drawPanelWithGlow(g, modPanelBounds, "MOD (LFO / ENV)");
        g.setColour(colourTextSecondary);
        g.setFont(juce::Font(10.0f).withExtraKerningFactor(0.05f));
        g.drawText("Serum-style: LFO or envelope.", modPanelBounds.withTrimmedTop(20).reduced(12, 4),
                    juce::Justification::centredLeft, false);
    }

    if (fxPanelBounds.getWidth() > 0.0f)
    {
        drawPanelWithGlow(g, fxPanelBounds, "FX");
        g.setColour(colourTextSecondary);
        g.setFont(juce::Font(10.0f).withExtraKerningFactor(0.05f));
        g.drawText("Reverb (hall/room), delay, chorus, distortion + filter", fxPanelBounds.withTrimmedTop(20).reduced(12, 4),
                    juce::Justification::centredLeft, false);
    }

    if (rightPanelBounds.getWidth() > 0.0f)
    {
        drawPanelWithGlow(g, rightPanelBounds, "OSCILLATORS");
        g.setColour(colourTextSecondary);
        g.setFont(juce::Font(10.0f).withExtraKerningFactor(0.05f));
        g.drawText("3 oscillators, 16+ unison, heavy detune", rightPanelBounds.withTrimmedTop(20).reduced(12, 4),
                    juce::Justification::centredLeft, false);
    }

    // Inner cards
    auto drawInnerCard = [&](const juce::Rectangle<float>& card, const juce::String& title)
    {
        if (card.isEmpty())
            return;
        const float radius = 14.0f;
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(card.translated(1.5f, 2.0f), radius);

        juce::ColourGradient bg(colourPanel.darker(0.18f), card.getX(), card.getY(),
                                colourPanelAlt, card.getX(), card.getBottom(), false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(card, radius);

        g.setColour(colourBlue.withAlpha(0.18f));
        g.drawRoundedRectangle(card, radius, 1.0f);

        g.setColour(colourText);
        g.setFont(juce::Font(11.0f, juce::Font::bold).withExtraKerningFactor(0.12f));
        auto titleArea = card;
        g.drawText(title, titleArea.removeFromTop(18.0f).reduced(10, 2), juce::Justification::centredLeft, false);
    };

    const juce::String layerTitles[] = { "LAYER A", "LAYER B", "LAYER C" };
    for (int i = 0; i < 3; ++i)
        drawInnerCard(layerCardBounds[i], layerTitles[i]);

    const juce::String oscTitles[] = { "OSC 1", "OSC 2", "OSC 3" };
    for (int i = 0; i < 3; ++i)
        drawInnerCard(oscCardBounds[i], oscTitles[i]);

    auto drawSubPanel = [&](const juce::Rectangle<float>& bounds)
    {
        if (bounds.isEmpty())
            return;
        const float radius = 14.0f;
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(bounds.translated(1.2f, 1.8f), radius);

        juce::ColourGradient bg(colourPanel.darker(0.18f), bounds.getX(), bounds.getY(),
                                colourPanelAlt, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(bounds, radius);

        g.setColour(colourBlue.withAlpha(0.18f));
        g.drawRoundedRectangle(bounds, radius, 1.0f);
    };

    drawSubPanel(modLeftBounds);
    drawSubPanel(modRightBounds);
    for (const auto& block : fxBlockBounds)
        drawSubPanel(block);
    
    // Update animation phase
    glowPhase += 0.01f;
    if (glowPhase > juce::MathConstants<float>::twoPi)
        glowPhase -= juce::MathConstants<float>::twoPi;
}

void RavelandAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().toFloat();
    bounds.reduce(14, 12); // Main margin

    // Header section
    const float headerHeight = 96.0f;
    auto header = bounds.removeFromTop(headerHeight);

    // Preset + buttons centered
    const float headerMidX = header.getCentreX();
    const float presetWidth = 380.0f;
    const float buttonWidth = 90.0f;
    const float headerY = header.getBottom() - 36.0f;

    const float labelWidth = 54.0f;
    presetLabel.setBounds(juce::Rectangle<int>(static_cast<int>(headerMidX - presetWidth * 0.5f - labelWidth - 6.0f),
                                               static_cast<int>(headerY),
                                               static_cast<int>(labelWidth),
                                               32).toNearestInt());

    presetCombo.setBounds(juce::Rectangle<int>(static_cast<int>(headerMidX - presetWidth * 0.5f),
                                                static_cast<int>(headerY),
                                                static_cast<int>(presetWidth),
                                                32).toNearestInt());

    randomizeButton.setBounds(juce::Rectangle<int>(static_cast<int>(headerMidX + presetWidth * 0.5f + 12),
                                                   static_cast<int>(headerY),
                                                   static_cast<int>(buttonWidth),
                                                   32).toNearestInt());

    resetButton.setBounds(juce::Rectangle<int>(static_cast<int>(headerMidX + presetWidth * 0.5f + 12 + buttonWidth + 8),
                                               static_cast<int>(headerY),
                                               static_cast<int>(buttonWidth),
                                               32).toNearestInt());

    // Main content area with better proportions
    auto mainArea = bounds.reduced(8, 8);
    mainArea.removeFromTop(10); // Space after header

    // Three main columns with better proportions
    auto leftColumn = mainArea.removeFromLeft(mainArea.getWidth() * 0.25f);
    auto centerColumn = mainArea.removeFromLeft(mainArea.getWidth() * 0.40f);
    auto rightColumn = mainArea;

    // Left Column: Audio Layers + Performance
    const float perfHeight = juce::jlimit(160.0f, 200.0f, leftColumn.getHeight() * 0.20f);
    leftPanelBounds = leftColumn.removeFromTop(leftColumn.getHeight() - perfHeight).reduced(4, 4);
    perfPanelBounds = leftColumn.reduced(4, 4);

    auto leftContent = leftPanelBounds.reduced(10, 18);
    layoutLayerSection(leftContent);
    auto perfContent = perfPanelBounds.reduced(10, 16);
    layoutPerformanceSection(perfContent);

    // Center Column: MOD + FX
    modPanelBounds = centerColumn.removeFromTop(centerColumn.getHeight() * 0.36f).reduced(4, 4);
    fxPanelBounds = centerColumn.reduced(4, 4);
    layoutModSection(modPanelBounds.reduced(10, 16));
    layoutFXSection(fxPanelBounds.reduced(10, 16));

    // Right Column: Oscillators
    rightPanelBounds = rightColumn.reduced(4, 4);
    layoutOscillatorSection(rightPanelBounds.reduced(10, 18));
}

void RavelandAudioProcessorEditor::layoutLayerSection(juce::Rectangle<float> area)
{
    area.removeFromTop(34.0f); // space for title/subtitle

    // Three layer sections with equal spacing
    const float layerHeight = (area.getHeight() - 14.0f) / 3.0f;

    for (int i = 0; i < 3; ++i)
    {
        auto card = area.removeFromTop(layerHeight).reduced(4, 6);
        layerCardBounds[i] = card;
        auto layerArea = card.reduced(8, 8);
        layerArea.removeFromTop(18.0f);

        // Enable switch (right)
        auto layerHeader = layerArea.removeFromTop(22.0f);
        layerControls[i].enabled.setBounds(layerHeader.removeFromRight(80).toNearestInt());

        const float stackRowH = 24.0f;
        const float scopeH = juce::jlimit(54.0f, 72.0f, layerArea.getHeight() * 0.23f);
        const float knobRowH = juce::jlimit(70.0f, 95.0f, layerArea.getHeight() * 0.30f);
        const float sliderRowH = juce::jmax(0.0f, layerArea.getHeight() - stackRowH - scopeH - knobRowH - 6.0f);

        // Sample set selector row
        auto sampleRow = layerArea.removeFromTop(stackRowH).reduced(4, 0);
        auto sampleLabelArea = sampleRow.removeFromLeft(60);
        layerControls[i].sampleSetLabel.setBounds(sampleLabelArea.toNearestInt());
        layerControls[i].sampleSet.setBounds(sampleRow.toNearestInt());

        // Waveform display
        auto waveformArea = layerArea.removeFromTop(scopeH).reduced(2, 4);
        layerWaveforms[i]->setBounds(waveformArea.toNearestInt());

        // Waveform label
        auto waveformLabelArea = waveformArea.withY(waveformArea.getBottom() - 16).withHeight(14);
        if (i == 0) layerWaveformLabel1.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 1) layerWaveformLabel2.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 2) layerWaveformLabel3.setBounds(waveformLabelArea.toNearestInt());

        // Tune / Start Rand knobs
        auto knobRow = layerArea.removeFromTop(knobRowH).reduced(2, 0);
        auto tuneArea = knobRow.removeFromLeft(knobRow.getWidth() * 0.5f).reduced(4, 0);
        auto tuneLabelArea = tuneArea.removeFromTop(14);
        layerControls[i].tuneLabel.setBounds(tuneLabelArea.toNearestInt());
        const float knobSizeA = juce::jmin(78.0f, tuneArea.getWidth(), tuneArea.getHeight());
        layerControls[i].tune.setBounds(tuneArea.withSizeKeepingCentre(knobSizeA, knobSizeA).toNearestInt());

        auto startRandArea = knobRow.reduced(4, 0);
        auto startLabelArea = startRandArea.removeFromTop(14);
        layerControls[i].startRandLabel.setBounds(startLabelArea.toNearestInt());
        const float knobSizeB = juce::jmin(78.0f, startRandArea.getWidth(), startRandArea.getHeight());
        layerControls[i].startRand.setBounds(startRandArea.withSizeKeepingCentre(knobSizeB, knobSizeB).toNearestInt());

        // Attack / Release sliders
        auto sliderRow = layerArea.removeFromTop(sliderRowH).reduced(2, 0);
        auto attackArea = sliderRow.removeFromLeft(sliderRow.getWidth() * 0.5f).reduced(4, 0);
        auto attackLabelArea = attackArea.removeFromTop(14);
        layerControls[i].attackLabel.setBounds(attackLabelArea.toNearestInt());
        layerControls[i].attack.setBounds(attackArea.reduced(0, 2).toNearestInt());

        auto releaseArea = sliderRow.reduced(4, 0);
        auto releaseLabelArea = releaseArea.removeFromTop(14);
        layerControls[i].releaseLabel.setBounds(releaseLabelArea.toNearestInt());
        layerControls[i].release.setBounds(releaseArea.reduced(0, 2).toNearestInt());

        // Space between layers
        area.removeFromTop(6);
    }
}

void RavelandAudioProcessorEditor::layoutModSection(juce::Rectangle<float> area)
{
    area.removeFromTop(26.0f);
    auto content = area.reduced(8, 10);

    const float gap = 10.0f;
    const float leftWidth = juce::jmin(300.0f, content.getWidth() * 0.38f);
    modLeftBounds = content.removeFromLeft(leftWidth);
    content.removeFromLeft(gap);
    modRightBounds = content;

    auto left = modLeftBounds.reduced(8, 8);
    auto modeRow = left.removeFromTop(28.0f);
    auto modeLabelArea = modeRow.removeFromLeft(60.0f);
    modModeLabel.setBounds(modeLabelArea.toNearestInt());
    modModeCombo.setBounds(modeRow.toNearestInt());

    auto knobRow = left.removeFromTop(120.0f).reduced(0, 2);
    auto rateArea = knobRow.removeFromLeft(knobRow.getWidth() * 0.5f).reduced(4, 0);
    auto rateLabelArea = rateArea.removeFromTop(14);
    modRateLabel.setBounds(rateLabelArea.toNearestInt());
    const float knobSizeA = juce::jmin(78.0f, rateArea.getWidth(), rateArea.getHeight());
    modRateKnob.setBounds(rateArea.withSizeKeepingCentre(knobSizeA, knobSizeA).toNearestInt());

    auto amtArea = knobRow.reduced(4, 0);
    auto amtLabelArea = amtArea.removeFromTop(14);
    modAmountLabel.setBounds(amtLabelArea.toNearestInt());
    const float knobSizeB = juce::jmin(78.0f, amtArea.getWidth(), amtArea.getHeight());
    modAmountKnob.setBounds(amtArea.withSizeKeepingCentre(knobSizeB, knobSizeB).toNearestInt());

    auto targetRow = left.removeFromTop(28.0f);
    auto targetLabelArea = targetRow.removeFromLeft(70.0f);
    modTargetLabel.setBounds(targetLabelArea.toNearestInt());
    modTargetCombo.setBounds(targetRow.toNearestInt());

    modHintLabel.setBounds(left.reduced(2, 4).toNearestInt());

    auto right = modRightBounds.reduced(8, 8);
    modWaveformLabel.setBounds(right.removeFromTop(16).toNearestInt());
    auto waveArea = right.removeFromTop(140.0f).reduced(2, 4);
    modWaveform->setBounds(waveArea.toNearestInt());

    right.removeFromTop(8.0f);
    auto buttons = right.removeFromTop(36.0f);
    auto btnW = (buttons.getWidth() - 8.0f) * 0.5f;
    modShapeButton.setBounds(buttons.removeFromLeft(btnW).toNearestInt());
    buttons.removeFromLeft(8.0f);
    modTriggerButton.setBounds(buttons.toNearestInt());
}

void RavelandAudioProcessorEditor::layoutFXSection(juce::Rectangle<float> area)
{
    area.removeFromTop(26.0f);
    auto content = area.reduced(8, 10);
    const float gap = 12.0f;

    const float colW = (content.getWidth() - gap * 2.0f) / 3.0f;
    auto leftCol = content.removeFromLeft(colW);
    content.removeFromLeft(gap);
    auto midCol = content.removeFromLeft(colW);
    content.removeFromLeft(gap);
    auto rightCol = content;

    const float blockGap = 8.0f;
    const float blockH = (leftCol.getHeight() - blockGap) * 0.5f;

    fxBlockBounds[0] = leftCol.removeFromTop(blockH).reduced(2, 2);
    leftCol.removeFromTop(blockGap);
    fxBlockBounds[1] = leftCol.reduced(2, 2);

    fxBlockBounds[2] = midCol.removeFromTop(blockH).reduced(2, 2);
    midCol.removeFromTop(blockGap);
    fxBlockBounds[3] = midCol.reduced(2, 2);

    fxBlockBounds[4] = rightCol.removeFromTop(blockH).reduced(2, 2);
    rightCol.removeFromTop(blockGap);
    fxBlockBounds[5] = rightCol.reduced(2, 2);

    // Filter block
    {
        auto block = fxBlockBounds[0].reduced(8, 8);
        auto header = block.removeFromTop(22.0f);
        filterTitleLabel.setBounds(header.removeFromLeft(140).toNearestInt());
        filterEnabled.setBounds(header.removeFromRight(80).toNearestInt());

        auto typeRow = block.removeFromTop(26.0f);
        auto typeLabelArea = typeRow.removeFromLeft(60.0f);
        filterTypeLabel.setBounds(typeLabelArea.toNearestInt());
        filterTypeCombo.setBounds(typeRow.toNearestInt());

        auto knobRow = block.removeFromTop(110.0f).reduced(2, 0);
        auto cutoffArea = knobRow.removeFromLeft(knobRow.getWidth() * 0.5f).reduced(4, 0);
        auto cutoffLabelArea = cutoffArea.removeFromTop(14);
        filterCutoffLabel.setBounds(cutoffLabelArea.toNearestInt());
        const float knobSizeA = juce::jmin(76.0f, cutoffArea.getWidth(), cutoffArea.getHeight());
        filterCutoffSlider.setBounds(cutoffArea.withSizeKeepingCentre(knobSizeA, knobSizeA).toNearestInt());

        auto resoArea = knobRow.reduced(4, 0);
        auto resoLabelArea = resoArea.removeFromTop(14);
        filterResoLabel.setBounds(resoLabelArea.toNearestInt());
        const float knobSizeB = juce::jmin(76.0f, resoArea.getWidth(), resoArea.getHeight());
        filterResoSlider.setBounds(resoArea.withSizeKeepingCentre(knobSizeB, knobSizeB).toNearestInt());
    }

    // Chorus block
    {
        auto block = fxBlockBounds[1].reduced(8, 8);
        auto header = block.removeFromTop(22.0f);
        chorusTitleLabel.setBounds(header.removeFromLeft(120).toNearestInt());
        chorusEnabled.setBounds(header.removeFromRight(80).toNearestInt());

        auto knobRow = block.removeFromTop(110.0f).reduced(2, 0);
        auto rateArea = knobRow.removeFromLeft(knobRow.getWidth() * 0.5f).reduced(4, 0);
        auto rateLabelArea = rateArea.removeFromTop(14);
        chorusRateLabel.setBounds(rateLabelArea.toNearestInt());
        const float knobSizeA = juce::jmin(76.0f, rateArea.getWidth(), rateArea.getHeight());
        chorusRateSlider.setBounds(rateArea.withSizeKeepingCentre(knobSizeA, knobSizeA).toNearestInt());

        auto mixArea = knobRow.reduced(4, 0);
        auto mixLabelArea = mixArea.removeFromTop(14);
        chorusMixLabel.setBounds(mixLabelArea.toNearestInt());
        const float knobSizeB = juce::jmin(76.0f, mixArea.getWidth(), mixArea.getHeight());
        chorusMixSlider.setBounds(mixArea.withSizeKeepingCentre(knobSizeB, knobSizeB).toNearestInt());
    }

    // Delay block
    {
        auto block = fxBlockBounds[2].reduced(8, 8);
        auto header = block.removeFromTop(22.0f);
        delayTitleLabel.setBounds(header.removeFromLeft(120).toNearestInt());
        delayEnabled.setBounds(header.removeFromRight(80).toNearestInt());

        auto knobRow = block.removeFromTop(110.0f).reduced(2, 0);
        auto timeArea = knobRow.removeFromLeft(knobRow.getWidth() * 0.5f).reduced(4, 0);
        auto timeLabelArea = timeArea.removeFromTop(14);
        delayTimeLabel.setBounds(timeLabelArea.toNearestInt());
        const float knobSizeA = juce::jmin(76.0f, timeArea.getWidth(), timeArea.getHeight());
        delayTimeSlider.setBounds(timeArea.withSizeKeepingCentre(knobSizeA, knobSizeA).toNearestInt());

        auto fbArea = knobRow.reduced(4, 0);
        auto fbLabelArea = fbArea.removeFromTop(14);
        delayFeedbackLabel.setBounds(fbLabelArea.toNearestInt());
        const float knobSizeB = juce::jmin(76.0f, fbArea.getWidth(), fbArea.getHeight());
        delayFeedbackSlider.setBounds(fbArea.withSizeKeepingCentre(knobSizeB, knobSizeB).toNearestInt());

        auto mixRow = block.removeFromTop(36.0f);
        auto mixLabelArea = mixRow.removeFromLeft(40.0f);
        delayMixLabel.setBounds(mixLabelArea.toNearestInt());
        delayMixSlider.setBounds(mixRow.toNearestInt());
    }

    // Reverb block
    {
        auto block = fxBlockBounds[3].reduced(8, 8);
        auto header = block.removeFromTop(22.0f);
        reverbTitleLabel.setBounds(header.removeFromLeft(120).toNearestInt());
        reverbEnabled.setBounds(header.removeFromRight(80).toNearestInt());

        auto modeRow = block.removeFromTop(26.0f);
        auto modeLabelArea = modeRow.removeFromLeft(50.0f);
        reverbModeLabel.setBounds(modeLabelArea.toNearestInt());
        reverbModeCombo.setBounds(modeRow.toNearestInt());

        auto knobRow = block.removeFromTop(110.0f).reduced(2, 0);
        auto sizeArea = knobRow.removeFromLeft(knobRow.getWidth() * 0.5f).reduced(4, 0);
        auto sizeLabelArea = sizeArea.removeFromTop(14);
        reverbSizeLabel.setBounds(sizeLabelArea.toNearestInt());
        const float knobSizeA = juce::jmin(76.0f, sizeArea.getWidth(), sizeArea.getHeight());
        reverbSizeSlider.setBounds(sizeArea.withSizeKeepingCentre(knobSizeA, knobSizeA).toNearestInt());

        auto dampArea = knobRow.reduced(4, 0);
        auto dampLabelArea = dampArea.removeFromTop(14);
        reverbDampLabel.setBounds(dampLabelArea.toNearestInt());
        const float knobSizeB = juce::jmin(76.0f, dampArea.getWidth(), dampArea.getHeight());
        reverbDampSlider.setBounds(dampArea.withSizeKeepingCentre(knobSizeB, knobSizeB).toNearestInt());

        auto mixRow = block.removeFromTop(36.0f);
        auto mixLabelArea = mixRow.removeFromLeft(40.0f);
        reverbMixLabel.setBounds(mixLabelArea.toNearestInt());
        reverbMixSlider.setBounds(mixRow.toNearestInt());
    }

    // Distortion block
    {
        auto block = fxBlockBounds[4].reduced(8, 8);
        auto header = block.removeFromTop(22.0f);
        distTitleLabel.setBounds(header.removeFromLeft(140).toNearestInt());
        distEnabled.setBounds(header.removeFromRight(80).toNearestInt());

        auto typeRow = block.removeFromTop(26.0f);
        auto typeLabelArea = typeRow.removeFromLeft(50.0f);
        distTypeLabel.setBounds(typeLabelArea.toNearestInt());
        distTypeCombo.setBounds(typeRow.toNearestInt());

        auto knobRow = block.removeFromTop(110.0f).reduced(2, 0);
        auto driveArea = knobRow.removeFromLeft(knobRow.getWidth() * 0.5f).reduced(4, 0);
        auto driveLabelArea = driveArea.removeFromTop(14);
        distDriveLabel.setBounds(driveLabelArea.toNearestInt());
        const float knobSizeA = juce::jmin(76.0f, driveArea.getWidth(), driveArea.getHeight());
        distDriveSlider.setBounds(driveArea.withSizeKeepingCentre(knobSizeA, knobSizeA).toNearestInt());

        auto toneArea = knobRow.reduced(4, 0);
        auto toneLabelArea = toneArea.removeFromTop(14);
        distToneLabel.setBounds(toneLabelArea.toNearestInt());
        const float knobSizeB = juce::jmin(76.0f, toneArea.getWidth(), toneArea.getHeight());
        distToneSlider.setBounds(toneArea.withSizeKeepingCentre(knobSizeB, knobSizeB).toNearestInt());

        auto mixRow = block.removeFromTop(36.0f);
        auto mixLabelArea = mixRow.removeFromLeft(40.0f);
        distMixLabel.setBounds(mixLabelArea.toNearestInt());
        distMixSlider.setBounds(mixRow.toNearestInt());
    }

    // Routing block
    {
        auto block = fxBlockBounds[5].reduced(8, 8);
        auto header = block.removeFromTop(22.0f);
        routingTitleLabel.setBounds(header.toNearestInt());
        routingListLabel.setBounds(block.reduced(2, 4).toNearestInt());
    }
}

void RavelandAudioProcessorEditor::layoutOscillatorSection(juce::Rectangle<float> area)
{
    area.removeFromTop(24.0f);

    auto masterArea = area.removeFromBottom(52).reduced(6, 4);

    const float gap = 4.0f;
    const float oscHeight = (area.getHeight() - gap * 2.0f) / 3.0f;

    for (int i = 0; i < 3; ++i)
    {
        auto card = area.removeFromTop(oscHeight).reduced(3, 3);
        oscCardBounds[i] = card;
        auto oscArea = card.reduced(6, 6);
        oscArea.removeFromTop(12.0f);

        // Enable switch (right)
        auto oscHeader = oscArea.removeFromTop(22.0f);
        oscControls[i].enabled.setBounds(oscHeader.removeFromRight(80).toNearestInt());

        const float waveRowH = 12.0f;
        const float minScopeH = 4.0f;
        const float maxScopeH = 16.0f;
        float scopeH = juce::jlimit(minScopeH, maxScopeH, oscArea.getHeight() * 0.16f);
        float remaining = oscArea.getHeight() - waveRowH - scopeH;
        float rowH = remaining * 0.5f;
        const float desiredRowH = 96.0f;
        if (rowH < desiredRowH)
        {
            const float needed = desiredRowH * 2.0f + waveRowH;
            scopeH = juce::jmax(minScopeH, oscArea.getHeight() - needed);
            remaining = oscArea.getHeight() - waveRowH - scopeH;
            rowH = remaining * 0.5f;
        }
        rowH = juce::jmax(0.0f, rowH);

        // Waveform selector row
        auto waveRow = oscArea.removeFromTop(waveRowH).reduced(4, 0);
        auto waveLabelArea = waveRow.removeFromLeft(60);
        oscControls[i].waveformLabel.setBounds(waveLabelArea.toNearestInt());
        oscControls[i].waveform.setBounds(waveRow.toNearestInt());

        // Waveform display
        auto waveformArea = oscArea.removeFromTop(scopeH).reduced(2, 4);
        oscWaveforms[i]->setBounds(waveformArea.toNearestInt());

        // Waveform label
        auto waveformLabelArea = waveformArea.withY(waveformArea.getBottom() - 14).withHeight(12);
        if (i == 0) oscWaveformLabel1.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 1) oscWaveformLabel2.setBounds(waveformLabelArea.toNearestInt());
        else if (i == 2) oscWaveformLabel3.setBounds(waveformLabelArea.toNearestInt());

        auto knobSizeFor = [](float areaHeight, float width)
        {
            float size = juce::jmin(width, areaHeight);
            size = juce::jlimit(10.0f, 88.0f, size);
            return juce::jmax(10.0f, size);
        };

        // Unison / Detune row
        auto rowOne = oscArea.removeFromTop(rowH).reduced(2, 0);
        auto voicesArea = rowOne.removeFromLeft(rowOne.getWidth() * 0.5f).reduced(2, 0);
        auto voicesLabelArea = voicesArea.removeFromTop(14);
        oscControls[i].voicesLabel.setBounds(voicesLabelArea.toNearestInt());
        const float knobSize1 = knobSizeFor(voicesArea.getHeight(), voicesArea.getWidth());
        oscControls[i].voices.setBounds(voicesArea.withSizeKeepingCentre(knobSize1, knobSize1).toNearestInt());

        auto detuneArea = rowOne.reduced(2, 0);
        auto detuneLabelArea = detuneArea.removeFromTop(14);
        oscControls[i].detuneLabel.setBounds(detuneLabelArea.toNearestInt());
        const float knobSize2 = knobSizeFor(detuneArea.getHeight(), detuneArea.getWidth());
        oscControls[i].detune.setBounds(detuneArea.withSizeKeepingCentre(knobSize2, knobSize2).toNearestInt());

        // Semi / Fine / Level row
        auto rowTwo = oscArea.removeFromTop(rowH).reduced(2, 0);
        const float knobWidth = rowTwo.getWidth() / 3.0f;

        auto semiArea = rowTwo.removeFromLeft(knobWidth).reduced(2, 0);
        auto semiLabelArea = semiArea.removeFromTop(14);
        oscControls[i].semiLabel.setBounds(semiLabelArea.toNearestInt());
        const float knobSize3 = knobSizeFor(semiArea.getHeight(), semiArea.getWidth());
        oscControls[i].semi.setBounds(semiArea.withSizeKeepingCentre(knobSize3, knobSize3).toNearestInt());

        auto fineArea = rowTwo.removeFromLeft(knobWidth).reduced(2, 0);
        auto fineLabelArea = fineArea.removeFromTop(14);
        oscControls[i].fineLabel.setBounds(fineLabelArea.toNearestInt());
        const float knobSize4 = knobSizeFor(fineArea.getHeight(), fineArea.getWidth());
        oscControls[i].fine.setBounds(fineArea.withSizeKeepingCentre(knobSize4, knobSize4).toNearestInt());

        auto levelArea = rowTwo.reduced(2, 0);
        auto levelLabelArea = levelArea.removeFromTop(14);
        oscControls[i].levelLabel.setBounds(levelLabelArea.toNearestInt());
        const float knobSize5 = knobSizeFor(levelArea.getHeight(), levelArea.getWidth());
        oscControls[i].level.setBounds(levelArea.withSizeKeepingCentre(knobSize5, knobSize5).toNearestInt());

        // Space between oscillators
        if (i < 2)
            area.removeFromTop(gap);
    }

    auto masterLabelArea = masterArea.removeFromTop(14);
    masterGainLabel.setBounds(masterLabelArea.toNearestInt());
    const float knobSizeM = juce::jmin(90.0f, masterArea.getWidth(), masterArea.getHeight());
    masterGainSlider.setBounds(masterArea.withSizeKeepingCentre(knobSizeM, knobSizeM).toNearestInt());
}

void RavelandAudioProcessorEditor::layoutFooterSection(juce::Rectangle<float> bounds)
{
    auto footer = bounds.removeFromBottom(100.0f).reduced(24, 12);

    // Performance controls
    auto controlsArea = footer.reduced(8, 8);

    auto wheelsArea = controlsArea.removeFromLeft(220);
    auto pitchArea = wheelsArea.removeFromLeft(100);
    pitchLabel.setBounds(pitchArea.removeFromTop(14).toNearestInt());
    pitchWheel.setBounds(pitchArea.reduced(12, 0).toNearestInt());

    auto modArea = wheelsArea;
    modLabel.setBounds(modArea.removeFromTop(14).toNearestInt());
    modWheel.setBounds(modArea.reduced(12, 0).toNearestInt());

    controlsArea.removeFromLeft(16);

    monoButton.setBounds(controlsArea.removeFromLeft(120).toNearestInt());
    controlsArea.removeFromLeft(12);

    legatoButton.setBounds(controlsArea.removeFromLeft(120).toNearestInt());
    controlsArea.removeFromLeft(20);

    auto portamentoArea = controlsArea.removeFromLeft(180);
    auto portaLabelArea = portamentoArea.removeFromTop(14);
    portamentoLabel.setBounds(portaLabelArea.toNearestInt());
    const float knobSizeP = juce::jmin(86.0f, portamentoArea.getWidth(), portamentoArea.getHeight());
    portamentoSlider.setBounds(portamentoArea.withSizeKeepingCentre(knobSizeP, knobSizeP).toNearestInt());
}

void RavelandAudioProcessorEditor::layoutPerformanceSection(juce::Rectangle<float> area)
{
    area.removeFromTop(26.0f);
    auto content = area.reduced(8, 8);

    auto wheelsArea = content.removeFromLeft(210.0f);
    auto pitchArea = wheelsArea.removeFromLeft(100.0f);
    pitchLabel.setBounds(pitchArea.removeFromTop(14).toNearestInt());
    pitchWheel.setBounds(pitchArea.reduced(12, 0).toNearestInt());

    auto modArea = wheelsArea;
    modLabel.setBounds(modArea.removeFromTop(14).toNearestInt());
    modWheel.setBounds(modArea.reduced(12, 0).toNearestInt());

    content.removeFromLeft(12.0f);

    auto rightArea = content;
    auto switches = rightArea.removeFromTop(32.0f);
    monoButton.setBounds(switches.removeFromLeft(110).toNearestInt());
    switches.removeFromLeft(8);
    legatoButton.setBounds(switches.removeFromLeft(110).toNearestInt());

    rightArea.removeFromTop(6.0f);
    auto knobsRow = rightArea.removeFromTop(120.0f);
    auto portaArea = knobsRow.removeFromLeft(knobsRow.getWidth() * 0.5f).reduced(4, 0);
    auto portaLabelArea = portaArea.removeFromTop(14);
    portamentoLabel.setBounds(portaLabelArea.toNearestInt());
    const float knobSizeP = juce::jmin(90.0f, portaArea.getWidth(), portaArea.getHeight());
    portamentoSlider.setBounds(portaArea.withSizeKeepingCentre(knobSizeP, knobSizeP).toNearestInt());

    auto curveArea = knobsRow.reduced(4, 0);
    auto curveLabelArea = curveArea.removeFromTop(14);
    glideCurveLabel.setBounds(curveLabelArea.toNearestInt());
    const float knobSizeC = juce::jmin(90.0f, curveArea.getWidth(), curveArea.getHeight());
    glideCurveSlider.setBounds(curveArea.withSizeKeepingCentre(knobSizeC, knobSizeC).toNearestInt());
}
