#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/** Simple supersaw-style oscillator used per voice.
    This is deliberately lightweight to be realistic for a small team. */
class SupersawOsc
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        phaseBase = 0.0;
        for (auto& p : detunedPhase)
            p = 0.0;
    }

    void setFrequency(float hz) { freq = hz; }
    void setDetuneCents(float cents) { detuneCents = cents; }
    void setNumVoices(int voices) { numVoices = juce::jlimit(1, maxVoices, voices); }
    void setGain(float g) { gain = g; }

    float processSample()
    {
        if (sr <= 0.0)
            return 0.0f;

        const double baseInc = (2.0 * juce::MathConstants<double>::pi * freq) / sr;
        const double maxSpread = 0.012; // modest spread to keep in tune
        const double spread = (detuneCents / 100.0) * maxSpread;

        float acc = 0.0f;
        for (int i = 0; i < numVoices; ++i)
        {
            const double offset = spread * ((double) i / (numVoices - 1) - 0.5);
            detunedPhase[i] += baseInc * (1.0 + offset);
            if (detunedPhase[i] > 2.0 * juce::MathConstants<double>::pi)
                detunedPhase[i] -= 2.0 * juce::MathConstants<double>::pi;

            const float s = std::sin((float) detunedPhase[i]);
            acc += s;
        }

        const float norm = 1.0f / (float) numVoices;
        return acc * norm * gain;
    }

private:
    static constexpr int maxVoices = 32;

    double sr { 0.0 };
    double phaseBase { 0.0 };
    std::array<double, maxVoices> detunedPhase {};

    float freq { 440.0f };
    float detuneCents { 0.0f };
    int numVoices { 8 };
    float gain { 0.7f };
};

/** One synth voice that mixes sampled layer stubs and a supersaw oscillator.
    For now, the "sample layers" are placeholders (no file I/O) but the architecture
    is in place to hook up your per-key WAV stacks later. */
class RavelandVoice : public juce::SynthesiserVoice
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        (void) spec;
        osc.prepare(spec.sampleRate);
        adsr.setSampleRate(spec.sampleRate);

        juce::ADSR::Parameters p;
        p.attack = 0.002f;
        p.decay = 0.12f;
        p.sustain = 0.8f;
        p.release = 0.35f;
        adsr.setParameters(p);
    }

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        auto freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        osc.setFrequency(freq);
        currentVelocity = velocity;
        adsr.noteOn();
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (allowTailOff)
            adsr.noteOff();
        else
        {
            clearCurrentNote();
            adsr.reset();
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (! adsr.isActive())
            return;

        temp.setSize(1, numSamples, false, false, true);

        for (int i = 0; i < numSamples; ++i)
            temp.setSample(0, i, osc.processSample() * currentVelocity);

        adsr.applyEnvelopeToBuffer(temp, 0, numSamples);

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            outputBuffer.addFrom(ch, startSample, temp, 0, 0, numSamples);
    }

private:
    SupersawOsc osc;
    juce::ADSR adsr;
    juce::AudioBuffer<float> temp;
    float currentVelocity { 0.0f };
};

