#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "WaveformDisplay.h"
#include "FancyKnob.h"

class RavelandAudioProcessor;

class RavelandLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                           bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText, const juce::Drawable* icon,
                           const juce::Colour* textColour) override;
};

class RavelandAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::Timer
{
public:
    explicit RavelandAudioProcessorEditor(RavelandAudioProcessor&);
    ~RavelandAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override { repaint(); }

private:
    RavelandAudioProcessor& processor;
    RavelandLookAndFeel lookAndFeel;

    // Animated glow effect
    float glowPhase = 0.0f;

    // Logos
    std::unique_ptr<juce::Drawable> ravelandLogoDrawable;
    std::unique_ptr<juce::Drawable> nsAudioLogoDrawable;

    // Waveform displays
    std::array<std::unique_ptr<WaveformDisplay>, 3> oscWaveforms;
    std::array<std::unique_ptr<WaveformDisplay>, 3> layerWaveforms;
    std::unique_ptr<WaveformDisplay> modWaveform;
    juce::Label oscWaveformLabel1, oscWaveformLabel2, oscWaveformLabel3;
    juce::Label layerWaveformLabel1, layerWaveformLabel2, layerWaveformLabel3;
    juce::Label modWaveformLabel;

    // Preset browser
    juce::ComboBox presetCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;
    juce::Label presetLabel;
    juce::TextButton randomizeButton { "Randomize" };
    juce::TextButton resetButton { "Reset" };
    juce::TextButton modShapeButton { "Change Shape" };
    juce::TextButton modTriggerButton { "Trigger" };
    juce::ComboBox modModeCombo;
    juce::ComboBox modTargetCombo;
    FancyKnob modRateKnob, modAmountKnob;
    juce::Label modModeLabel, modRateLabel, modAmountLabel, modTargetLabel;
    juce::Label modHintLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modTargetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modAmountAttachment;
    juce::Label routingLabel;

    // Master
    FancyKnob masterGainSlider;
    juce::Label masterGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterGainAttachment;

    // Oscillators (3)
    struct OscControls
    {
        juce::ToggleButton enabled;
        juce::ComboBox waveform;
        juce::Label waveformLabel;
        FancyKnob voices, detune, semi, fine, level;
        juce::Label voicesLabel, detuneLabel, semiLabel, fineLabel, levelLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabledAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> voicesAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> semiAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fineAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttachment;
    };
    std::array<OscControls, 3> oscControls;

    // Layers (3)
    struct LayerControls
    {
        juce::ToggleButton enabled;
        juce::ComboBox sampleSet;
        juce::Label sampleSetLabel;
        FancyKnob tune, startRand;
        juce::Label tuneLabel, startRandLabel;
        juce::Slider attack, release;
        juce::Label attackLabel, releaseLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabledAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuneAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> startRandAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    };
    std::array<LayerControls, 3> layerControls;

    // FX
    juce::Slider reverbMixSlider, delayMixSlider, distMixSlider;
    FancyKnob chorusMixSlider;
    FancyKnob reverbSizeSlider, reverbDampSlider, delayTimeSlider, delayFeedbackSlider;
    FancyKnob chorusRateSlider, chorusDepthSlider, distDriveSlider, distToneSlider;
    FancyKnob filterCutoffSlider, filterResoSlider;
    juce::Label reverbMixLabel, delayMixLabel, chorusMixLabel, distMixLabel;
    juce::Label reverbSizeLabel, reverbDampLabel, delayTimeLabel, delayFeedbackLabel;
    juce::Label chorusRateLabel, chorusDepthLabel, distDriveLabel, distToneLabel;
    juce::Label filterCutoffLabel, filterResoLabel;
    juce::Label filterTitleLabel, chorusTitleLabel, delayTitleLabel, reverbTitleLabel, distTitleLabel, routingTitleLabel;
    juce::Label filterTypeLabel, reverbModeLabel, distTypeLabel;
    juce::ToggleButton filterEnabled, chorusEnabled, delayEnabled, reverbEnabled, distEnabled;
    juce::ComboBox filterTypeCombo, reverbModeCombo, distTypeCombo;
    juce::Label routingListLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDampAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distToneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> filterEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> chorusEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> delayEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverbEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> distEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> reverbModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResoAttachment;

    // Mono/Legato
    juce::ToggleButton monoButton, legatoButton;
    FancyKnob portamentoSlider;
    juce::Label portamentoLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> legatoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> portamentoAttachment;
    juce::Slider pitchWheel, modWheel;
    juce::Label pitchLabel, modLabel;
    FancyKnob glideCurveSlider;
    juce::Label glideCurveLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glideCurveAttachment;

    void setupToggle(juce::ToggleButton& button);
    void loadLogos();
    void drawNeonGlow(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawPanelWithGlow(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& title);
    void drawRaveLandLogo(juce::Graphics& g, juce::Rectangle<float> bounds);
    void refreshSampleSetCombos();
    void layoutPerformanceSection(juce::Rectangle<float> area);

    // Layout functions
    void layoutLayerSection(juce::Rectangle<float> area);
    void layoutModSection(juce::Rectangle<float> area);
    void layoutFXSection(juce::Rectangle<float> area);
    void layoutOscillatorSection(juce::Rectangle<float> area);
    void layoutFooterSection(juce::Rectangle<float> bounds);

    juce::Rectangle<float> leftPanelBounds;
    juce::Rectangle<float> perfPanelBounds;
    juce::Rectangle<float> modPanelBounds;
    juce::Rectangle<float> fxPanelBounds;
    juce::Rectangle<float> rightPanelBounds;
    juce::Rectangle<float> modLeftBounds;
    juce::Rectangle<float> modRightBounds;
    std::array<juce::Rectangle<float>, 6> fxBlockBounds;
    std::array<juce::Rectangle<float>, 3> layerCardBounds;
    std::array<juce::Rectangle<float>, 3> oscCardBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RavelandAudioProcessorEditor)
};
