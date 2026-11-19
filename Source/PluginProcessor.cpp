#include "PluginProcessor.h"
#include "PluginEditor.h"

WoodBodyOneProcessor::WoodBodyOneProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    const char* names[] = { "Spruce_wav", "Mahogany_wav", "Rosewood_wav" };
    juce::WavAudioFormat wav;
    for (int t = 0; t < 3; ++t)
    {
        int size = 0;
        const char* data = BinaryData::getNamedResource (names[t], size);
        if (data == nullptr) continue;
        std::unique_ptr<juce::AudioFormatReader> reader (
            wav.createReaderFor (new juce::MemoryInputStream (data, (size_t) size, false), true));
        if (reader == nullptr) continue;
        auto& ir = irs[(size_t) t];
        ir.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
        reader->read (&ir, 0, ir.getNumSamples(), 0, true, true);
        irRate = reader->sampleRate;
        // Normalise so the IR is unity at 1 kHz (keeps loudness steady when Body changes)
        const double w = juce::MathConstants<double>::twoPi * 1000.0 / irRate;
        double re = 0, im = 0;
        auto* d = ir.getReadPointer (0);
        for (int k = 0; k < ir.getNumSamples(); ++k) { re += d[k] * std::cos (w * k); im -= d[k] * std::sin (w * k); }
        const double mag = std::sqrt (re * re + im * im);
        if (mag > 1.0e-6) ir.applyGain ((float) (1.0 / mag));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout WoodBodyOneProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "wood", 1 }, "Wood",
                                                  NormalisableRange<float> (0.0f, 100.0f, 0.1f), 40.0f,
                                                  AudioParameterFloatAttributes().withLabel ("%")));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "woodType", 1 }, "Wood Type",
                                                   StringArray { "Spruce", "Mahogany", "Rosewood" }, 1));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "stereo", 1 }, "Stereo Mode",
                                                   StringArray { "Mono", "X-Y", "Spaced" }, 1));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "output", 1 }, "Output",
                                                  NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
                                                  AudioParameterFloatAttributes().withLabel ("dB")));
    return l;
}

bool WoodBodyOneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo()) return false;
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

// One knob -> five processes. Must match the UI macro display.
Macro WoodBodyOneProcessor::computeMacro (float w, int type)
{
    static const float warmF[]   = { 0.7f, 1.1f, 1.0f };
    static const float grainF[]  = { 0.8f, 1.0f, 1.1f };
    static const float attackF[] = { 1.2f, 0.9f, 1.0f };
    type = juce::jlimit (0, 2, type);
    const float t = juce::jlimit (0.0f, 1.0f, w);
    Macro m;
    m.body      = juce::jmin (1.0f, t * 1.1f);
    m.warmthDb  = juce::jmin (1.0f, t * 0.85f * warmF[type]) * 6.0f;
    m.grain     = juce::jlimit (0.0f, 1.0f, (t - 0.25f) / 0.75f * 0.8f * grainF[type]);
    m.deharshDb = juce::jmin (1.0f, 0.25f + t * 0.6f) * 5.0f;
    const float a = t < 0.5f ? t * 0.6f : 0.6f - (t - 0.5f) * 0.4f;
    m.attack    = juce::jlimit (0.0f, 1.0f, a * attackF[type]);
    return m;
}

void WoodBodyOneProcessor::loadIR (int type)
{
    type = juce::jlimit (0, 2, type);
    if (irs[(size_t) type].getNumSamples() == 0) return;
    juce::AudioBuffer<float> copy (irs[(size_t) type]);
    convolver.loadImpulseResponse (std::move (copy), irRate,
                                   juce::dsp::Convolution::Stereo::yes,
                                   juce::dsp::Convolution::Trim::no,
                                   juce::dsp::Convolution::Normalise::no);
    loadedWood = type;
}

void WoodBodyOneProcessor::handleAsyncUpdate() { loadIR (wantedWood.load()); }

void WoodBodyOneProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    convolver.prepare (spec);
    loadedWood = -1;
    wantedWood = (int) apvts.getRawParameterValue ("woodType")->load();
    loadIR (wantedWood.load());

    juce::dsp::ProcessSpec mono { sampleRate, (juce::uint32) samplesPerBlock, 1 };
    for (int c = 0; c < 2; ++c)
    {
        for (auto* f : { &rumble[c], &warmth[c], &grainHP[c], &grainLP[c], &dcBlock[c] }) { f->prepare (mono); f->reset(); }
        rumble[c].coefficients  = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, 40.0, 0.707);
        grainHP[c].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, 150.0, 0.707);
        grainLP[c].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass  (sr, 600.0, 0.707);
        dcBlock[c].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, 20.0, 0.707);
        warmth[c].coefficients  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sr, 250.0, 0.8, 1.0);
        harshEnv[c] = 0;
    }
    harshBand.prepare (spec);
    harshBand.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    harshBand.setCutoffFrequency (3200.0f);
    harshBand.setResonance (1.0f);

    spacedDelay.prepare (spec);
    spacedDelay.setDelay ((float) (0.0006 * sr));

    dryBuf.setSize (2, samplesPerBlock);
    wetBuf.setSize (2, samplesPerBlock);
    fastEnv = slowEnv = 0; attackGain = 1.0f;

    outGain.reset (sr, 0.05); bodySm.reset (sr, 0.05); grainSm.reset (sr, 0.05);
    outGain.setCurrentAndTargetValue (1.0f);
    bodySm.setCurrentAndTargetValue (0.0f);
    grainSm.setCurrentAndTargetValue (0.0f);
    setLatencySamples (0);
}

void WoodBodyOneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    if (getTotalNumInputChannels() == 1 || buffer.getNumChannels() < 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, n);

    const int type   = (int) apvts.getRawParameterValue ("woodType")->load();
    const int stereo = (int) apvts.getRawParameterValue ("stereo")->load();
    const auto m     = computeMacro (apvts.getRawParameterValue ("wood")->load() / 100.0f, type);
    outGain.setTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("output")->load()));
    bodySm.setTargetValue (m.body);
    grainSm.setTargetValue (m.grain);
    if (type != loadedWood && type != wantedWood.load()) { wantedWood = type; triggerAsyncUpdate(); }

    meterIn.store (juce::jmax (buffer.getMagnitude (0, 0, n), buffer.getMagnitude (1, 0, n)));

    if (dryBuf.getNumSamples() < n) { dryBuf.setSize (2, n, false, false, true); wetBuf.setSize (2, n, false, false, true); }

    // 1) rumble cut
    for (int c = 0; c < 2; ++c)
    {
        auto* x = buffer.getWritePointer (c);
        for (int i = 0; i < n; ++i) x[i] = rumble[c].processSample (x[i]);
        dryBuf.copyFrom (c, 0, buffer, c, 0, n);
        wetBuf.copyFrom (c, 0, buffer, c, 0, n);
    }

    // 2) body IR (convolution), blended with dry
    {
        juce::dsp::AudioBlock<float> blk (wetBuf.getArrayOfWritePointers(), 2, (size_t) n);
        convolver.process (juce::dsp::ProcessContextReplacing<float> (blk));
        for (int i = 0; i < n; ++i)
        {
            const float b = bodySm.getNextValue();
            for (int c = 0; c < 2; ++c)
                buffer.setSample (c, i, dryBuf.getSample (c, i) * (1.0f - b) + wetBuf.getSample (c, i) * b);
        }
    }

    // 3) warmth + 4) grain (woody low-mid saturation, even harmonics)
    const auto warmCoef = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sr, 250.0, 0.8, juce::Decibels::decibelsToGain (m.warmthDb));
    for (int c = 0; c < 2; ++c)
    {
        *warmth[c].coefficients = *warmCoef;
        auto* x = buffer.getWritePointer (c);
        for (int i = 0; i < n; ++i)
        {
            float s = warmth[c].processSample (x[i]);
            const float g = grainSm.getCurrentValue();
            if (g > 0.001f)
            {
                const float band  = grainLP[c].processSample (grainHP[c].processSample (s));
                const float drive = 1.0f + 6.0f * g;
                const float bias  = 0.25f;
                const float sat   = (std::tanh (drive * band + bias) - std::tanh (bias)) / drive;
                s += g * 1.6f * (sat - band * (1.0f / (1.0f + g)));
                s = dcBlock[c].processSample (s);
            }
            x[i] = s;
        }
    }
    grainSm.skip (n);

    // 5) de-harsh: dynamic cut around 3.2 kHz
    {
        const float atk = std::exp (-1.0f / (0.002f * (float) sr));
        const float rel = std::exp (-1.0f / (0.060f * (float) sr));
        const float thrDb = -30.0f;
        for (int c = 0; c < 2; ++c)
        {
            auto* x = buffer.getWritePointer (c);
            for (int i = 0; i < n; ++i)
            {
                const float band = harshBand.processSample (c, x[i]);
                const float a = std::abs (band);
                harshEnv[c] = a > harshEnv[c] ? atk * harshEnv[c] + (1 - atk) * a : rel * harshEnv[c] + (1 - rel) * a;
                const float envDb = juce::Decibels::gainToDecibels (harshEnv[c] + 1.0e-9f);
                const float cut = juce::jlimit (0.0f, m.deharshDb, (envDb - thrDb) * 0.5f);
                x[i] -= band * (1.0f - juce::Decibels::decibelsToGain (-cut));
            }
        }
    }

    // 6) attack (transient shaper, linked)
    {
        const float fa = std::exp (-1.0f / (0.001f * (float) sr)), fr = std::exp (-1.0f / (0.030f * (float) sr));
        const float sa = std::exp (-1.0f / (0.020f * (float) sr)), srl = std::exp (-1.0f / (0.150f * (float) sr));
        const float gs = std::exp (-1.0f / (0.002f * (float) sr));
        auto* L = buffer.getWritePointer (0); auto* R = buffer.getWritePointer (1);
        for (int i = 0; i < n; ++i)
        {
            const float a = juce::jmax (std::abs (L[i]), std::abs (R[i]));
            fastEnv = a > fastEnv ? fa * fastEnv + (1 - fa) * a : fr * fastEnv + (1 - fr) * a;
            slowEnv = a > slowEnv ? sa * slowEnv + (1 - sa) * a : srl * slowEnv + (1 - srl) * a;
            const float ratio = fastEnv / (slowEnv + 1.0e-6f);
            const float target = 1.0f + m.attack * juce::jlimit (0.0f, 2.0f, ratio - 1.0f) * 0.5f;
            attackGain = gs * attackGain + (1 - gs) * target;
            L[i] *= attackGain; R[i] *= attackGain;
        }
    }

    // 7) stereo mode + output
    auto* L = buffer.getWritePointer (0); auto* R = buffer.getWritePointer (1);
    for (int i = 0; i < n; ++i)
    {
        float l = L[i], r = R[i];
        if (stereo == 0) { l = r = 0.5f * (l + r); }
        else if (stereo == 2)
        {
            spacedDelay.pushSample (0, r);
            r = spacedDelay.popSample (0);
        }
        const float g = outGain.getNextValue();
        L[i] = l * g; R[i] = r * g;
    }

    meterOut.store (juce::jmax (buffer.getMagnitude (0, 0, n), buffer.getMagnitude (1, 0, n)));
}

void WoodBodyOneProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("tips", tipsOn.load(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void WoodBodyOneProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        auto state = juce::ValueTree::fromXml (*xml);
        if (state.isValid())
        {
            tipsOn = (bool) state.getProperty ("tips", true);
            apvts.replaceState (state);
        }
    }
}

juce::AudioProcessorEditor* WoodBodyOneProcessor::createEditor() { return new WoodBodyOneEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new WoodBodyOneProcessor(); }
