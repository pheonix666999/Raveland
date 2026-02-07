#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthVoice.h"
#include <cmath>

namespace
{
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // Master
        params.push_back(std::make_unique<juce::AudioParameterFloat>("masterGain", "Master Gain",
                                                                     juce::NormalisableRange<float>(-24.0f, 6.0f), 0.0f));

        // Oscillators (3 oscillators)
        for (int i = 1; i <= 3; ++i)
        {
            auto prefix = "osc" + juce::String(i);
            params.push_back(std::make_unique<juce::AudioParameterBool>(prefix + "Enabled", prefix + " Enabled", i <= 2));
            params.push_back(std::make_unique<juce::AudioParameterChoice>(prefix + "Wave", prefix + " Wave",
                                                                          juce::StringArray { "Saw", "Pulse", "Sine", "Triangle", "SuperSaw", "HyperSaw" }, 0));
            params.push_back(std::make_unique<juce::AudioParameterInt>(prefix + "Voices", prefix + " Voices", 1, 32, i == 1 ? 16 : 12));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Detune", prefix + " Detune",
                                                                         juce::NormalisableRange<float>(0.0f, 100.0f), i == 1 ? 55.0f : 45.0f));
            params.push_back(std::make_unique<juce::AudioParameterInt>(prefix + "Semi", prefix + " Semi", -24, 24, 0));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Fine", prefix + " Fine",
                                                                         juce::NormalisableRange<float>(-100.0f, 100.0f), 0.0f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Level", prefix + " Level",
                                                                         juce::NormalisableRange<float>(0.0f, 1.0f), i == 1 ? 0.85f : 0.75f));
        }

        // Layers (3 sample layers)
        for (int i = 1; i <= 3; ++i)
        {
            auto prefix = "layer" + juce::String(i);
            params.push_back(std::make_unique<juce::AudioParameterBool>(prefix + "Enabled", prefix + " Enabled", i <= 2));
            params.push_back(std::make_unique<juce::AudioParameterInt>(prefix + "Tune", prefix + " Tune", -12, 12, 0));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Gain", prefix + " Gain",
                                                                         juce::NormalisableRange<float>(0.0f, 1.0f), i == 1 ? 0.8f : 0.7f));
            params.push_back(std::make_unique<juce::AudioParameterInt>(prefix + "StartRand", prefix + " Start Rand", 0, 100, i == 1 ? 35 : 45));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Attack", prefix + " Attack",
                                                                         juce::NormalisableRange<float>(0.0f, 500.0f), 12.0f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Release", prefix + " Release",
                                                                         juce::NormalisableRange<float>(0.0f, 2000.0f), 420.0f));
        }

        // Mod (LFO / ENV)
        params.push_back(std::make_unique<juce::AudioParameterChoice>("modMode", "Mod Mode",
                                                                      juce::StringArray { "LFO", "ENV" }, 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("modRate", "Mod Rate",
                                                                     juce::NormalisableRange<float>(0.05f, 32.0f), 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("modAmount", "Mod Amount",
                                                                     juce::NormalisableRange<float>(0.0f, 100.0f), 35.0f));
        params.push_back(std::make_unique<juce::AudioParameterChoice>("modTarget", "Mod Target",
                                                                      juce::StringArray { "None", "OSC1 Detune", "OSC2 Detune", "OSC3 Detune",
                                                                                          "Filter Cutoff", "Dist Drive", "Layer A Start Rand" }, 0));

        // FX
        params.push_back(std::make_unique<juce::AudioParameterBool>("filterEnabled", "Filter Enabled", true));
        params.push_back(std::make_unique<juce::AudioParameterChoice>("filterType", "Filter Type",
                                                                      juce::StringArray { "Lowpass", "Highpass", "Bandpass" }, 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("filterCutoff", "Filter Cutoff",
                                                                     juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f), 12000.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("filterReso", "Filter Reso",
                                                                     juce::NormalisableRange<float>(0.0f, 100.0f), 15.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>("chorusEnabled", "Chorus Enabled", true));
        params.push_back(std::make_unique<juce::AudioParameterBool>("delayEnabled", "Delay Enabled", true));
        params.push_back(std::make_unique<juce::AudioParameterBool>("reverbEnabled", "Reverb Enabled", true));
        params.push_back(std::make_unique<juce::AudioParameterBool>("distEnabled", "Distortion Enabled", true));
        params.push_back(std::make_unique<juce::AudioParameterChoice>("distType", "Distortion Type",
                                                                      juce::StringArray { "Soft", "Hard", "Tape" }, 0));
        params.push_back(std::make_unique<juce::AudioParameterChoice>("reverbMode", "Reverb Mode",
                                                                      juce::StringArray { "Hall", "Room" }, 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("reverbMix", "Reverb Mix",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.22f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("reverbSize", "Reverb Size",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("reverbDamp", "Reverb Damp",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("delayMix", "Delay Mix",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.18f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTime", "Delay Time",
                                                                     juce::NormalisableRange<float>(1.0f, 800.0f), 250.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("delayFeedback", "Delay Feedback",
                                                                     juce::NormalisableRange<float>(0.0f, 0.95f), 0.3f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("chorusMix", "Chorus Mix",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.30f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("chorusRate", "Chorus Rate",
                                                                     juce::NormalisableRange<float>(0.05f, 10.0f), 1.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("chorusDepth", "Chorus Depth",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("distMix", "Distortion Mix",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.26f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("distDrive", "Distortion Drive",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.4f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("distTone", "Distortion Tone",
                                                                     juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

        // Mono/Legato
        params.push_back(std::make_unique<juce::AudioParameterBool>("monoEnabled", "Mono Enabled", false));
        params.push_back(std::make_unique<juce::AudioParameterBool>("legatoEnabled", "Legato Enabled", false));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("portamento", "Portamento",
                                                                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>("glideCurve", "Glide Curve",
                                                                     juce::NormalisableRange<float>(-100.0f, 100.0f), 0.0f));

        return { params.begin(), params.end() };
    }
}

RavelandAudioProcessor::RavelandAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMS", createParameterLayout())
{
    struct SimpleSound : public juce::SynthesiserSound
    {
        bool appliesToNote (int) override      { return true; }
        bool appliesToChannel (int) override   { return true; }
    };

    constexpr int numVoices = 16;
    for (int i = 0; i < numVoices; ++i)
    {
        auto* voice = new RavelandVoice();
        voice->setSampleLayers(&sampleLayers);
        synth.addVoice(voice);
    }

    synth.addSound(new SimpleSound());
    synth.setNoteStealingEnabled(true);

    createFactoryPresets();
    scanSampleLibrary();
    if (sampleSetNames.size() > 0)
    {
        for (int i = 0; i < 3; ++i)
            loadSampleSet(i, juce::jmin(i, sampleSetNames.size() - 1));
    }
    loadPreset(0);
}

void RavelandAudioProcessor::createFactoryPresets()
{
    presetNames.clear();
    presetNames.add("INIT - Clean Saw Lead");
    presetNames.add("Rave - Wide SuperSaw Stack");
    presetNames.add("Trance - Tight JP-ish Pluck");
    presetNames.add("Hard Dance - Aggressive Stack");
}

int RavelandAudioProcessor::getNumPrograms()
{
    return presetNames.size();
}

int RavelandAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex;
}

void RavelandAudioProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < presetNames.size())
        loadPreset(index);
}

const juce::String RavelandAudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < presetNames.size())
        return presetNames[index];
    return {};
}

void RavelandAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    if (index >= 0 && index < presetNames.size())
        presetNames.set(index, newName);
}

juce::StringArray RavelandAudioProcessor::getPresetNames() const
{
    return presetNames;
}

int RavelandAudioProcessor::getSampleSetIndex(int layerIndex) const
{
    if (layerIndex < 0 || layerIndex >= (int) currentSampleSetIndex.size())
        return 0;
    return currentSampleSetIndex[(size_t) layerIndex];
}

void RavelandAudioProcessor::setSampleSetIndex(int layerIndex, int setIndex)
{
    if (layerIndex < 0 || layerIndex >= (int) currentSampleSetIndex.size())
        return;
    if (sampleSetNames.isEmpty())
        return;

    const int clamped = juce::jlimit(0, sampleSetNames.size() - 1, setIndex);

    auto loadFn = [this, layerIndex, clamped]()
    {
        loadSampleSet(layerIndex, clamped);
    };

    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        loadFn();
    else
        juce::MessageManager::callAsync(loadFn);
}

void RavelandAudioProcessor::loadPreset(int index)
{
    currentPresetIndex = juce::jlimit(0, presetNames.size() - 1, index);

    // Shared defaults for every preset
    for (int i = 1; i <= 3; ++i)
    {
        auto prefix = "osc" + juce::String(i);
        *parameters.getRawParameterValue(prefix + "Semi") = 0.0f;
        *parameters.getRawParameterValue(prefix + "Fine") = 0.0f;
    }

    for (int i = 1; i <= 3; ++i)
    {
        auto prefix = "layer" + juce::String(i);
        *parameters.getRawParameterValue(prefix + "Tune") = 0.0f;
        *parameters.getRawParameterValue(prefix + "Attack") = 12.0f;
        *parameters.getRawParameterValue(prefix + "Release") = 420.0f;
    }

    *parameters.getRawParameterValue("modMode") = 0.0f;
    *parameters.getRawParameterValue("modRate") = 1.0f;
    *parameters.getRawParameterValue("modAmount") = 35.0f;
    *parameters.getRawParameterValue("modTarget") = 0.0f;

    *parameters.getRawParameterValue("filterEnabled") = 1.0f;
    *parameters.getRawParameterValue("filterType") = 0.0f;
    *parameters.getRawParameterValue("filterCutoff") = 12000.0f;
    *parameters.getRawParameterValue("filterReso") = 15.0f;
    *parameters.getRawParameterValue("chorusEnabled") = 1.0f;
    *parameters.getRawParameterValue("delayEnabled") = 1.0f;
    *parameters.getRawParameterValue("reverbEnabled") = 1.0f;
    *parameters.getRawParameterValue("distEnabled") = 1.0f;
    *parameters.getRawParameterValue("distType") = 0.0f;
    *parameters.getRawParameterValue("reverbMode") = 0.0f;

    // Factory presets
    if (index == 0) // INIT
    {
        *parameters.getRawParameterValue("masterGain") = 0.0f;
        *parameters.getRawParameterValue("osc1Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc1Wave") = 0.0f;
        *parameters.getRawParameterValue("osc1Voices") = 16.0f;
        *parameters.getRawParameterValue("osc1Detune") = 50.0f;
        *parameters.getRawParameterValue("osc1Level") = 0.85f;
        *parameters.getRawParameterValue("osc2Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc2Wave") = 0.0f;
        *parameters.getRawParameterValue("osc2Voices") = 12.0f;
        *parameters.getRawParameterValue("osc2Detune") = 45.0f;
        *parameters.getRawParameterValue("osc2Level") = 0.75f;
        *parameters.getRawParameterValue("osc3Enabled") = 0.0f;
        *parameters.getRawParameterValue("osc3Wave") = 0.0f;
        *parameters.getRawParameterValue("reverbMix") = 0.22f;
        *parameters.getRawParameterValue("delayMix") = 0.18f;
        *parameters.getRawParameterValue("chorusMix") = 0.30f;
        *parameters.getRawParameterValue("glideCurve") = 0.0f;
    }
    else if (index == 1) // Rave
    {
        *parameters.getRawParameterValue("masterGain") = 0.0f;
        *parameters.getRawParameterValue("osc1Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc1Wave") = 4.0f;
        *parameters.getRawParameterValue("osc1Voices") = 24.0f;
        *parameters.getRawParameterValue("osc1Detune") = 72.0f;
        *parameters.getRawParameterValue("osc1Level") = 0.88f;
        *parameters.getRawParameterValue("osc2Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc2Wave") = 4.0f;
        *parameters.getRawParameterValue("osc2Voices") = 24.0f;
        *parameters.getRawParameterValue("osc2Detune") = 78.0f;
        *parameters.getRawParameterValue("osc2Level") = 0.78f;
        *parameters.getRawParameterValue("osc3Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc3Wave") = 5.0f;
        *parameters.getRawParameterValue("osc3Voices") = 16.0f;
        *parameters.getRawParameterValue("osc3Detune") = 60.0f;
        *parameters.getRawParameterValue("osc3Level") = 0.62f;
        *parameters.getRawParameterValue("reverbMix") = 0.28f;
        *parameters.getRawParameterValue("delayMix") = 0.26f;
        *parameters.getRawParameterValue("chorusMix") = 0.55f;
        *parameters.getRawParameterValue("glideCurve") = 0.0f;
    }
    else if (index == 2) // Trance
    {
        *parameters.getRawParameterValue("masterGain") = 0.0f;
        *parameters.getRawParameterValue("osc1Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc1Wave") = 1.0f;
        *parameters.getRawParameterValue("osc1Voices") = 12.0f;
        *parameters.getRawParameterValue("osc1Detune") = 40.0f;
        *parameters.getRawParameterValue("osc1Level") = 0.80f;
        *parameters.getRawParameterValue("osc2Enabled") = 0.0f;
        *parameters.getRawParameterValue("osc3Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc3Wave") = 2.0f;
        *parameters.getRawParameterValue("osc3Voices") = 8.0f;
        *parameters.getRawParameterValue("osc3Detune") = 18.0f;
        *parameters.getRawParameterValue("osc3Level") = 0.55f;
        *parameters.getRawParameterValue("reverbMix") = 0.14f;
        *parameters.getRawParameterValue("delayMix") = 0.18f;
        *parameters.getRawParameterValue("chorusMix") = 0.25f;
        *parameters.getRawParameterValue("monoEnabled") = 1.0f;
        *parameters.getRawParameterValue("legatoEnabled") = 1.0f;
        *parameters.getRawParameterValue("portamento") = 0.55f;
        *parameters.getRawParameterValue("glideCurve") = -20.0f;
    }
    else if (index == 3) // Hard Dance
    {
        *parameters.getRawParameterValue("masterGain") = 2.0f;
        *parameters.getRawParameterValue("osc1Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc1Wave") = 4.0f;
        *parameters.getRawParameterValue("osc1Voices") = 20.0f;
        *parameters.getRawParameterValue("osc1Detune") = 65.0f;
        *parameters.getRawParameterValue("osc1Level") = 0.90f;
        *parameters.getRawParameterValue("osc2Enabled") = 1.0f;
        *parameters.getRawParameterValue("osc2Wave") = 4.0f;
        *parameters.getRawParameterValue("osc2Voices") = 18.0f;
        *parameters.getRawParameterValue("osc2Detune") = 70.0f;
        *parameters.getRawParameterValue("osc2Level") = 0.85f;
        *parameters.getRawParameterValue("osc3Enabled") = 0.0f;
        *parameters.getRawParameterValue("osc3Wave") = 0.0f;
        *parameters.getRawParameterValue("reverbMix") = 0.20f;
        *parameters.getRawParameterValue("delayMix") = 0.15f;
        *parameters.getRawParameterValue("chorusMix") = 0.40f;
        *parameters.getRawParameterValue("distMix") = 0.35f;
        *parameters.getRawParameterValue("glideCurve") = 0.0f;
    }
}

void RavelandAudioProcessor::scanSampleLibrary()
{
    sampleSetNames.clear();
    sampleRoot = {};

    auto envPath = juce::SystemStats::getEnvironmentVariable("RAVELAND_SAMPLE_PATH", {});
    if (envPath.isNotEmpty())
    {
        juce::File envDir(envPath);
        if (envDir.isDirectory())
            sampleRoot = envDir;
    }

    if (! sampleRoot.isDirectory())
    {
        auto commonDir = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
                             .getChildFile("NS Audio")
                             .getChildFile("RaveLand")
                             .getChildFile("Samples");
        if (commonDir.isDirectory())
            sampleRoot = commonDir;
    }

    if (! sampleRoot.isDirectory())
    {
        auto userDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                           .getChildFile("NS Audio")
                           .getChildFile("RaveLand")
                           .getChildFile("Samples");
        if (userDir.isDirectory())
            sampleRoot = userDir;
    }

    if (! sampleRoot.isDirectory())
    {
        auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                          .getParentDirectory()
                          .getChildFile("Samples");
        if (exeDir.isDirectory())
            sampleRoot = exeDir;
    }

    if (! sampleRoot.isDirectory())
        return;

    auto folders = sampleRoot.findChildFiles(juce::File::findDirectories, false);
    folders.sort();
    for (auto& f : folders)
        sampleSetNames.add(f.getFileName());
}

void RavelandAudioProcessor::loadSampleSet(int layerIndex, int setIndex)
{
    if (layerIndex < 0 || layerIndex >= (int) sampleLayers.size())
        return;
    if (sampleSetNames.isEmpty())
        return;

    const int clamped = juce::jlimit(0, sampleSetNames.size() - 1, setIndex);
    auto folder = sampleRoot.getChildFile(sampleSetNames[clamped]);
    if (! folder.isDirectory())
        return;

    const double rate = spec.sampleRate > 0.0 ? spec.sampleRate : 44100.0;
    juce::ScopedWriteLock lock(sampleLayerLock);
    sampleLayers[(size_t) layerIndex].loadFromFolder(folder, rate);
    currentSampleSetIndex[(size_t) layerIndex] = clamped;
}

void RavelandAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<RavelandVoice*>(synth.getVoice(i)))
            v->prepare(spec);

    chorus.prepare(spec);
    delay.prepare(spec);
    filter.prepare(spec);
    filter.reset();
    distortion.prepare(spec);

    juce::Reverb::Parameters params;
    params.roomSize = 0.5f;
    params.damping = 0.35f;
    params.wetLevel = 0.22f;
    params.dryLevel = 1.0f;
    params.width = 0.85f;
    reverb.setSampleRate(sampleRate);
    reverb.setParameters(params);
}

void RavelandAudioProcessor::releaseResources()
{
}

bool RavelandAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void RavelandAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    // Update layer params from APVTS
    for (int i = 0; i < 3; ++i)
    {
        auto prefix = "layer" + juce::String(i + 1);
        layerEnabled[i] = parameters.getRawParameterValue(prefix + "Enabled")->load() > 0.5f;
        layerGain[i] = parameters.getRawParameterValue(prefix + "Gain")->load();
        layerStartRand[i] = (int) parameters.getRawParameterValue(prefix + "StartRand")->load();
        layerTune[i] = (int) parameters.getRawParameterValue(prefix + "Tune")->load();
        layerAttackMs[i] = parameters.getRawParameterValue(prefix + "Attack")->load();
        layerReleaseMs[i] = parameters.getRawParameterValue(prefix + "Release")->load();
    }

    const int modMode = (int) parameters.getRawParameterValue("modMode")->load();
    const float modRate = parameters.getRawParameterValue("modRate")->load();
    const float modAmount = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue("modAmount")->load() / 100.0f);
    const int modTarget = (int) parameters.getRawParameterValue("modTarget")->load();
    float modSignal = 0.0f;

    if (modMode == 0)
    {
        modSignal = std::sin(juce::MathConstants<float>::twoPi * modPhase);
        modPhase += (float) (modRate * (double) buffer.getNumSamples() / juce::jmax(1.0, spec.sampleRate));
        modPhase -= std::floor(modPhase);
    }
    else
    {
        modSignal = juce::jlimit(0.0f, 1.0f, modEnv);
    }

    const float modUni = (modMode == 0) ? 0.5f * (modSignal + 1.0f) : modSignal;
    if (modTarget == 6)
    {
        const float base = (float) layerStartRand[0];
        const float modded = base + modUni * (modAmount * 60.0f);
        layerStartRand[0] = (int) juce::jlimit(0.0f, 100.0f, modded);
    }

    // Optional mono/legato processing
    juce::MidiBuffer midiToProcess;
    const bool monoEnabled = parameters.getRawParameterValue("monoEnabled")->load() > 0.5f;
    if (monoEnabled)
    {
        for (const auto metadata : midi)
        {
            const auto msg = metadata.getMessage();
            const int samplePos = metadata.samplePosition;

            if (msg.isNoteOn())
            {
                const int note = msg.getNoteNumber();
                // Remove existing instance to maintain stack order
                heldNotes.removeAllInstancesOf(note);
                heldNotes.add(note);

                if (currentMonoNote != -1 && currentMonoNote != note)
                    midiToProcess.addEvent(juce::MidiMessage::noteOff(msg.getChannel(), currentMonoNote), samplePos);

                midiToProcess.addEvent(msg, samplePos);
                currentMonoNote = note;
            }
            else if (msg.isNoteOff())
            {
                const int note = msg.getNoteNumber();
                heldNotes.removeAllInstancesOf(note);
                if (note == currentMonoNote)
                {
                    midiToProcess.addEvent(msg, samplePos);
                    if (heldNotes.size() > 0)
                    {
                        const int newNote = heldNotes.getLast();
                        midiToProcess.addEvent(juce::MidiMessage::noteOn(msg.getChannel(), newNote, msg.getVelocity()), samplePos);
                        currentMonoNote = newNote;
                    }
                    else
                    {
                        currentMonoNote = -1;
                    }
                }
            }
            else
            {
                midiToProcess.addEvent(msg, samplePos);
            }
        }
    }
    else
    {
        midiToProcess = midi;
        heldNotes.clear();
        currentMonoNote = -1;
    }

    // Push updated params into voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<RavelandVoice*>(synth.getVoice(i)))
            v->updateParams(parameters, layerEnabled, layerGain, layerStartRand,
                            layerTune, layerAttackMs, layerReleaseMs, spec.sampleRate);
    }

    // Render synth oscillators + sample layers
    {
        juce::ScopedReadLock lock(sampleLayerLock);
        synth.renderNextBlock(buffer, midiToProcess, 0, buffer.getNumSamples());
    }

    // FX
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    const bool filterOn = parameters.getRawParameterValue("filterEnabled")->load() > 0.5f;
    const int filterType = (int) parameters.getRawParameterValue("filterType")->load();
    float filterCutoff = parameters.getRawParameterValue("filterCutoff")->load();
    const float filterReso = parameters.getRawParameterValue("filterReso")->load();

    if (modTarget == 4)
        filterCutoff = juce::jlimit(20.0f, 20000.0f, filterCutoff + modUni * (modAmount * (20000.0f - filterCutoff)));

    if (filterOn)
    {
        switch (filterType)
        {
            case 1: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
            case 2: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
            case 0:
            default: filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); break;
        }
        filter.setCutoffFrequency(filterCutoff);
        filter.setResonance(juce::jlimit(0.05f, 0.98f, filterReso / 100.0f));
        filter.process(context);
    }

    // Chorus
    const bool chorusOn = parameters.getRawParameterValue("chorusEnabled")->load() > 0.5f;
    if (chorusOn)
    {
        chorus.setMix(parameters.getRawParameterValue("chorusMix")->load());
        chorus.setRate(parameters.getRawParameterValue("chorusRate")->load());
        chorus.setDepth(parameters.getRawParameterValue("chorusDepth")->load());
        chorus.process(context);
    }

    // Delay + Distortion
    const bool delayOn = parameters.getRawParameterValue("delayEnabled")->load() > 0.5f;
    const bool distOn = parameters.getRawParameterValue("distEnabled")->load() > 0.5f;
    const auto delayMix = parameters.getRawParameterValue("delayMix")->load();
    const auto delayTime = parameters.getRawParameterValue("delayTime")->load();
    const auto delayFeedback = parameters.getRawParameterValue("delayFeedback")->load();
    const auto distMix = parameters.getRawParameterValue("distMix")->load();
    float distDrive = parameters.getRawParameterValue("distDrive")->load();
    const auto distTone = parameters.getRawParameterValue("distTone")->load();
    const int distType = (int) parameters.getRawParameterValue("distType")->load();

    if (modTarget == 5)
        distDrive = juce::jlimit(0.0f, 1.0f, distDrive + modUni * (modAmount * 0.6f));

    // Calculate delay in samples (simplified - using fixed delay for now)
    const int delaySamples = static_cast<int>(spec.sampleRate * delayTime / 1000.0f);
    const int maxDelaySamples = 44100; // 1 second at 44.1kHz
    const int clampedDelaySamples = juce::jlimit(1, maxDelaySamples, delaySamples);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = data[i];
            float delayed = 0.0f;
            if (delayOn)
            {
                delayed = delay.popSample(channel, clampedDelaySamples) * delayFeedback;
                delay.pushSample(channel, dry + delayed);
            }
            else
            {
                delay.pushSample(channel, dry);
            }

            const float delayOut = delayOn ? (dry * (1.0f - delayMix) + delayed * delayMix) : dry;

            float distOut = delayOut;
            if (distOn)
            {
                auto distInput = delayOut * (1.0f + distDrive * 4.0f);
                float dist = 0.0f;
                switch (distType)
                {
                    case 1: // Hard
                        dist = juce::jlimit(-1.0f, 1.0f, distInput);
                        break;
                    case 2: // Tape-ish
                        dist = std::atan(distInput) * 1.15f;
                        break;
                    case 0:
                    default:
                        dist = distortion.processSample(distInput);
                        break;
                }
                dist = dist * (1.0f + distTone * 0.5f) + dist * distTone * 0.3f;
                distOut = delayOut * (1.0f - distMix) + dist * distMix;
            }

            data[i] = distOut;
        }
    }

    // Reverb
    const bool reverbOn = parameters.getRawParameterValue("reverbEnabled")->load() > 0.5f;
    if (buffer.getNumChannels() >= 2)
    {
        auto mix = parameters.getRawParameterValue("reverbMix")->load();
        auto size = parameters.getRawParameterValue("reverbSize")->load();
        auto damp = parameters.getRawParameterValue("reverbDamp")->load();
        const int mode = (int) parameters.getRawParameterValue("reverbMode")->load();

        auto params = reverb.getParameters();
        params.wetLevel = reverbOn ? mix : 0.0f;
        params.roomSize = (mode == 0) ? juce::jlimit(0.2f, 0.98f, size) : juce::jlimit(0.1f, 0.7f, size * 0.7f);
        params.damping = (mode == 0) ? damp : juce::jlimit(0.0f, 0.8f, damp * 0.6f);
        params.width = (mode == 0) ? 0.9f : 0.7f;
        reverb.setParameters(params);
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    }

    if (modMode == 1)
    {
        const float rms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
        modEnv = modEnv * 0.88f + rms * 0.12f;
    }

    const auto masterGainDb = parameters.getRawParameterValue("masterGain")->load();
    const auto gain = juce::Decibels::decibelsToGain(masterGainDb);
    buffer.applyGain(gain);
}

juce::AudioProcessorEditor* RavelandAudioProcessor::createEditor()
{
    return new RavelandAudioProcessorEditor(*this);
}

void RavelandAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    for (int i = 0; i < 3; ++i)
    {
        juce::String propName = "layer" + juce::String(i + 1) + "SampleSet";
        if (sampleSetNames.isEmpty())
            state.setProperty(propName, juce::String(), nullptr);
        else
        {
            const int idx = juce::jlimit(0, sampleSetNames.size() - 1, currentSampleSetIndex[(size_t) i]);
            state.setProperty(propName, sampleSetNames[idx], nullptr);
        }
    }

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void RavelandAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        auto state = juce::ValueTree::fromXml(*xml);
        parameters.replaceState(state);

        for (int i = 0; i < 3; ++i)
        {
            juce::String propName = "layer" + juce::String(i + 1) + "SampleSet";
            auto name = state.getProperty(propName).toString();
            if (name.isNotEmpty() && ! sampleSetNames.isEmpty())
            {
                const int idx = sampleSetNames.indexOf(name);
                if (idx >= 0)
                    setSampleSetIndex(i, idx);
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RavelandAudioProcessor();
}

