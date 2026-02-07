#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>
#include "SampleLayer.h"

enum class RavelandWaveform
{
    Saw = 0,
    Pulse = 1,
    Sine = 2,
    Triangle = 3,
    SuperSaw = 4,
    HyperSaw = 5
};

class UnisonOsc
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        baseInc = 0.0f;
        for (int i = 0; i < maxVoices; ++i)
            phases[i] = (float) i / (float) maxVoices;
        tableDirty = true;
    }

    void setFrequency(float hz)
    {
        freq = hz;
        baseInc = (sr > 0.0) ? (hz / (float) sr) : 0.0f;
    }

    void setDetune(float detuneAmount)
    {
        detune = detuneAmount;
        tableDirty = true;
    }

    void setNumVoices(int voices)
    {
        numVoices = juce::jlimit(1, maxVoices, voices);
        tableDirty = true;
    }

    void setGain(float g) { gain = g; }

    void setWaveform(RavelandWaveform wf)
    {
        waveform = wf;
        tableDirty = true;
    }

    float process()
    {
        if (sr <= 0.0 || numVoices <= 0)
            return 0.0f;

        if (tableDirty)
            updateDetuneTable();

        float acc = 0.0f;
        for (int i = 0; i < numVoices; ++i)
        {
            const float inc = baseInc * detuneRatios[i];
            float t = phases[i];
            t += inc;
            if (t >= 1.0f)
                t -= 1.0f;
            phases[i] = t;

            const float s = renderWave(t, inc);
            acc += s * ampWeights[i];
        }

        if (weightSum > 0.0f)
            acc /= weightSum;

        return acc * gain;
    }

private:
    static constexpr int maxVoices = 32;

    double sr { 0.0 };
    float freq { 440.0f };
    float detune { 0.0f };
    int numVoices { 8 };
    float gain { 0.7f };
    float baseInc { 0.0f };
    bool tableDirty { true };
    RavelandWaveform waveform { RavelandWaveform::Saw };

    std::array<float, maxVoices> phases {};
    std::array<float, maxVoices> detuneRatios {};
    std::array<float, maxVoices> ampWeights {};
    float weightSum { 1.0f };

    static float polyBlep(float t, float dt)
    {
        if (t < dt)
        {
            t /= dt;
            return t + t - t * t - 1.0f;
        }
        if (t > 1.0f - dt)
        {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        }
        return 0.0f;
    }

    float renderWave(float t, float dt) const
    {
        const float phase = t;
        const float pi2 = juce::MathConstants<float>::twoPi;

        switch (waveform)
        {
            case RavelandWaveform::Pulse:
            {
                const float pw = 0.5f;
                float s = (phase < pw) ? 1.0f : -1.0f;
                s += polyBlep(phase, dt);
                const float t2 = std::fmod(phase - pw + 1.0f, 1.0f);
                s -= polyBlep(t2, dt);
                return s;
            }
            case RavelandWaveform::Sine:
                return std::sin(pi2 * phase);
            case RavelandWaveform::Triangle:
                return 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;
            case RavelandWaveform::SuperSaw:
            case RavelandWaveform::HyperSaw:
            case RavelandWaveform::Saw:
            default:
            {
                float s = 2.0f * phase - 1.0f;
                s -= polyBlep(phase, dt);
                return s;
            }
        }
    }

    void updateDetuneTable()
    {
        tableDirty = false;
        weightSum = 0.0f;

        const float maxSpread = (waveform == RavelandWaveform::HyperSaw) ? 150.0f : 110.0f;
        const float spread = juce::jlimit(0.0f, maxSpread, detune * (waveform == RavelandWaveform::HyperSaw ? 1.35f : 1.0f));

        for (int i = 0; i < numVoices; ++i)
        {
            float pos = (numVoices == 1) ? 0.0f : (float) i / (float) (numVoices - 1);
            pos = pos * 2.0f - 1.0f; // -1..1

            float curve = pos;
            if (waveform == RavelandWaveform::SuperSaw)
                curve = std::sin(pos * juce::MathConstants<float>::halfPi);
            else if (waveform == RavelandWaveform::HyperSaw)
                curve = (pos >= 0.0f ? 1.0f : -1.0f) * pos * pos;

            const float cents = spread * curve;
            detuneRatios[i] = std::pow(2.0f, cents / 1200.0f);

            const float weight = 1.0f - 0.45f * std::abs(curve);
            ampWeights[i] = weight;
            weightSum += weight;
        }

        for (int i = numVoices; i < maxVoices; ++i)
        {
            detuneRatios[i] = 1.0f;
            ampWeights[i] = 0.0f;
        }
    }
};

class RavelandVoice : public juce::SynthesiserVoice
{
public:
    RavelandVoice() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sr = spec.sampleRate;
        for (auto& osc : oscs)
            osc.prepare(sr);

        adsr.setSampleRate(sr);
        juce::ADSR::Parameters p;
        p.attack = 0.002f;
        p.decay = 0.12f;
        p.sustain = 0.8f;
        p.release = 0.35f;
        adsr.setParameters(p);

        freqSmooth.reset(sr, 0.0);
    }

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr;
    }

    void updateParams(juce::AudioProcessorValueTreeState& vts,
                      const std::array<bool, 3>& layerEnabledIn,
                      const std::array<float, 3>& layerGainIn,
                      const std::array<int, 3>& layerStartRandIn,
                      const std::array<int, 3>& layerTuneIn,
                      const std::array<float, 3>& layerAttackMsIn,
                      const std::array<float, 3>& layerReleaseMsIn,
                      double sampleRate)
    {
        for (int i = 0; i < 3; ++i)
        {
            const auto prefix = "osc" + juce::String(i + 1);
            oscEnabled[i] = vts.getRawParameterValue(prefix + "Enabled")->load() > 0.5f;
            const int voices = (int) vts.getRawParameterValue(prefix + "Voices")->load();
            const float detune = vts.getRawParameterValue(prefix + "Detune")->load();
            const int semi = (int) vts.getRawParameterValue(prefix + "Semi")->load();
            const float fine = vts.getRawParameterValue(prefix + "Fine")->load();
            const float level = vts.getRawParameterValue(prefix + "Level")->load();
            const int wave = (int) vts.getRawParameterValue(prefix + "Wave")->load();

            oscs[i].setNumVoices(voices);
            oscs[i].setDetune(detune);
            oscs[i].setGain(level);
            oscs[i].setWaveform(static_cast<RavelandWaveform>(juce::jlimit(0, 5, wave)));

            const float semis = (float) semi + fine / 100.0f;
            oscTuneRatio[i] = std::pow(2.0f, semis / 12.0f);
            oscDetuneBase[i] = detune;
        }

        layerEnabled = layerEnabledIn;
        layerGain = layerGainIn;
        layerStartRand = layerStartRandIn;
        layerTune = layerTuneIn;
        layerAttackMs = layerAttackMsIn;
        layerReleaseMs = layerReleaseMsIn;

        for (int i = 0; i < 3; ++i)
        {
            layerAttackSamples[i] = (int) std::round(layerAttackMs[i] * 0.001f * (float) sampleRate);
            layerReleaseSamples[i] = (int) std::round(layerReleaseMs[i] * 0.001f * (float) sampleRate);
        }

        modMode = (int) vts.getRawParameterValue("modMode")->load();
        modRate = vts.getRawParameterValue("modRate")->load();
        modAmount = vts.getRawParameterValue("modAmount")->load();
        modTarget = (int) vts.getRawParameterValue("modTarget")->load();
        modPhaseInc = (float) (modRate / juce::jmax(1.0, sampleRate));

        legatoEnabled = vts.getRawParameterValue("legatoEnabled")->load() > 0.5f;
        const float port = vts.getRawParameterValue("portamento")->load();
        if (std::abs(port - lastPortamento) > 0.0001f)
        {
            const float portSeconds = port * 0.35f;
            freqSmooth.reset(sampleRate, portSeconds);
            lastPortamento = port;
        }
    }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        currentNote = midiNoteNumber;
        currentVelocity = velocity;
        const float targetFreq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        const bool isLegato = legatoEnabled && adsr.isActive();
        if (! isLegato)
            adsr.noteOn();

        if (! isLegato)
            freqSmooth.setCurrentAndTargetValue(targetFreq);
        else
            freqSmooth.setTargetValue(targetFreq);

        for (int i = 0; i < 3; ++i)
        {
            layerActive[i] = false;
            if (sampleLayers == nullptr)
                continue;
            if (! layerEnabled[i])
                continue;
            const int tunedNote = juce::jlimit(0, 127, midiNoteNumber + layerTune[i]);
            layerNote[i] = tunedNote;
            if (! (*sampleLayers)[i].hasNote(tunedNote))
                continue;

            const int length = (*sampleLayers)[i].getLengthInSamples(tunedNote, sr);
            if (length <= 0)
                continue;

            const int maxOffset = (int) std::round(length * (layerStartRand[i] / 100.0f));
            const int offset = rng.nextInt(juce::jmax(1, maxOffset + 1));
            layerPos[i] = juce::jlimit(0, length - 1, offset);
            layerLen[i] = length;
            layerActive[i] = true;
        }
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

        const int numChannels = outputBuffer.getNumChannels();
        temp.setSize(numChannels, numSamples, false, false, true);

        for (int i = 0; i < numSamples; ++i)
        {
            const float env = adsr.getNextSample();
            const float freq = freqSmooth.getNextValue();
            float modSignal = 0.0f;
            if (modMode == 0) // LFO
            {
                modSignal = std::sin(juce::MathConstants<float>::twoPi * modPhase);
                modPhase += modPhaseInc;
                if (modPhase >= 1.0f)
                    modPhase -= 1.0f;
            }
            else // ENV
            {
                modSignal = env;
            }
            const float detuneMod = modSignal * (modAmount * 0.2f);
            const int targetOsc = (modTarget >= 1 && modTarget <= 3) ? (modTarget - 1) : -1;
            float oscSample = 0.0f;
            for (int o = 0; o < 3; ++o)
            {
                if (! oscEnabled[o])
                    continue;
                if (o == targetOsc)
                    oscs[o].setDetune(juce::jlimit(0.0f, 100.0f, oscDetuneBase[o] + detuneMod));
                else
                    oscs[o].setDetune(oscDetuneBase[o]);
                oscs[o].setFrequency(freq * oscTuneRatio[o]);
                oscSample += oscs[o].process();
            }

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float layerSample = 0.0f;
                if (sampleLayers != nullptr)
                {
                    for (int l = 0; l < 3; ++l)
                    {
                        if (! layerActive[l])
                            continue;
                        float envLayer = 1.0f;
                        const int attackSamples = layerAttackSamples[l];
                        const int releaseSamples = layerReleaseSamples[l];
                        if (attackSamples > 0 && layerPos[l] < attackSamples)
                            envLayer = (float) layerPos[l] / (float) attackSamples;
                        if (releaseSamples > 0 && layerPos[l] >= (layerLen[l] - releaseSamples))
                        {
                            const int samplesLeft = juce::jmax(0, layerLen[l] - layerPos[l]);
                            envLayer = juce::jmin(envLayer, samplesLeft / (float) releaseSamples);
                        }

                        layerSample += (*sampleLayers)[l].getSample(layerNote[l], layerPos[l], ch, sr)
                                       * layerGain[l] * envLayer;
                    }
                }

                const float value = (oscSample + layerSample) * currentVelocity * env;
                temp.setSample(ch, i, value);
            }

            for (int l = 0; l < 3; ++l)
            {
                if (! layerActive[l])
                    continue;
                layerPos[l] += 1;
                if (layerPos[l] >= layerLen[l])
                    layerActive[l] = false;
            }
        }

        for (int ch = 0; ch < numChannels; ++ch)
            outputBuffer.addFrom(ch, startSample, temp, ch, 0, numSamples);
    }

    void setSampleLayers(const std::array<SampleLayer, 3>* layers)
    {
        sampleLayers = layers;
    }

private:
    double sr { 44100.0 };
    std::array<UnisonOsc, 3> oscs;
    std::array<bool, 3> oscEnabled { true, true, false };
    std::array<float, 3> oscTuneRatio { 1.0f, 1.0f, 1.0f };
    std::array<float, 3> oscDetuneBase { 0.0f, 0.0f, 0.0f };
    std::array<bool, 3> layerEnabled { true, true, false };
    std::array<float, 3> layerGain { 0.8f, 0.7f, 0.6f };
    std::array<int, 3> layerStartRand { 35, 45, 55 };
    std::array<int, 3> layerTune { 0, 0, 0 };
    std::array<float, 3> layerAttackMs { 12.0f, 12.0f, 12.0f };
    std::array<float, 3> layerReleaseMs { 420.0f, 420.0f, 420.0f };
    std::array<int, 3> layerAttackSamples { 0, 0, 0 };
    std::array<int, 3> layerReleaseSamples { 0, 0, 0 };
    std::array<int, 3> layerPos {};
    std::array<int, 3> layerLen {};
    std::array<bool, 3> layerActive { false, false, false };
    std::array<int, 3> layerNote { -1, -1, -1 };

    int modMode { 0 };
    int modTarget { 0 };
    float modRate { 1.0f };
    float modAmount { 0.0f };
    float modPhase { 0.0f };
    float modPhaseInc { 0.0f };

    const std::array<SampleLayer, 3>* sampleLayers { nullptr };
    juce::ADSR adsr;
    juce::AudioBuffer<float> temp;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> freqSmooth;
    juce::Random rng;

    bool legatoEnabled { false };
    int currentNote { -1 };
    float currentVelocity { 0.0f };
    float lastPortamento { -1.0f };
};
