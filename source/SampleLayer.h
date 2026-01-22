#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <array>

/** Per-key sample data for one layer stack. */
class SampleLayer
{
public:
    SampleLayer() = default;
    ~SampleLayer() = default;

    bool loadFromFolder(const juce::File& folder, double sampleRate)
    {
        if (!folder.isDirectory())
            return false;

        juce::AudioFormatManager manager;
        manager.registerBasicFormats();

        // MIDI note 0 (C-1) to 127 (G9) = 128 notes
        // We'll map to 0-127 for simplicity
        for (int note = 0; note < 128; ++note)
        {
            // Try common naming: C-1.wav, C#-1.wav, etc.
            // Or numeric: 000.wav, 001.wav, etc.
            juce::String noteName = juce::MidiMessage::getMidiNoteName(note, true, true, 4);
            noteName = noteName.replaceCharacter(' ', '-');

            juce::File wavFile = folder.getChildFile(noteName + ".wav");
            if (!wavFile.existsAsFile())
            {
                // Try numeric
                wavFile = folder.getChildFile(juce::String::formatted("%03d.wav", note));
            }

            if (wavFile.existsAsFile())
            {
                std::unique_ptr<juce::AudioFormatReader> reader(manager.createReaderFor(wavFile));
                if (reader != nullptr)
                {
                    juce::AudioBuffer<float> buffer(static_cast<int>(reader->numChannels),
                                                    static_cast<int>(reader->lengthInSamples));
                    reader->read(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), 0, buffer.getNumSamples());

                    samples[note] = std::move(buffer);
                    sampleRates[note] = reader->sampleRate;
                }
            }
        }

        return true;
    }

    float getSample(int note, int sampleIndex, int channel, double targetSampleRate) const
    {
        if (note < 0 || note >= 128)
            return 0.0f;

        const auto& buffer = samples[note];
        if (buffer.getNumSamples() == 0)
            return 0.0f;

        // Simple linear interpolation for pitch shifting
        const double sourceRate = sampleRates[note];
        if (sourceRate <= 0.0)
            return 0.0f;

        const double ratio = sourceRate / targetSampleRate;
        const double sourcePos = sampleIndex * ratio;

        const int idx0 = static_cast<int>(sourcePos);
        const int idx1 = idx0 + 1;
        const float frac = static_cast<float>(sourcePos - idx0);

        if (idx0 >= buffer.getNumSamples())
            return 0.0f;

        const float s0 = buffer.getSample(channel % buffer.getNumChannels(), idx0);
        const float s1 = (idx1 < buffer.getNumSamples()) ? buffer.getSample(channel % buffer.getNumChannels(), idx1) : 0.0f;

        return s0 + frac * (s1 - s0);
    }

    int getLengthInSamples(int note, double targetSampleRate) const
    {
        if (note < 0 || note >= 128)
            return 0;

        const auto& buffer = samples[note];
        if (buffer.getNumSamples() == 0)
            return 0;

        const double sourceRate = sampleRates[note];
        if (sourceRate <= 0.0)
            return 0;

        return static_cast<int>(buffer.getNumSamples() * (targetSampleRate / sourceRate));
    }

    bool hasNote(int note) const
    {
        if (note < 0 || note >= 128)
            return false;
        return samples[note].getNumSamples() > 0;
    }

private:
    std::array<juce::AudioBuffer<float>, 128> samples;
    std::array<double, 128> sampleRates {};
};
