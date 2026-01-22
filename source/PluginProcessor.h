#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SampleLayer.h"

class RavelandAudioProcessor : public juce::AudioProcessor
{
public:
    RavelandAudioProcessor();
    ~RavelandAudioProcessor() override = default;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return "RaveLand"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

    // Preset management
    void loadPreset(int index);
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::StringArray getPresetNames() const;

private:
    juce::AudioProcessorValueTreeState parameters;

    juce::Synthesiser synth;

    juce::dsp::ProcessSpec spec{};
    juce::dsp::Chorus<float> chorus;
    juce::dsp::DelayLine<float> delay { 44100 };
    juce::Reverb reverb;
    juce::dsp::WaveShaper<float> distortion { [] (float x) { return std::tanh(x); } };

    // Sample layers (up to 3)
    std::array<SampleLayer, 3> sampleLayers;
    std::array<bool, 3> layerEnabled { true, true, false };
    std::array<float, 3> layerGain { 0.8f, 0.7f, 0.6f };
    std::array<int, 3> layerStartRand { 35, 45, 55 };

    int currentPresetIndex = 0;
    juce::StringArray presetNames;

    void createFactoryPresets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RavelandAudioProcessor)
};
