#pragma once
#include <JuceHeader.h>

// Macro amounts derived from the single WOOD knob (0..1 each)
struct Macro
{
    float body = 0, warmthDb = 0, grain = 0, deharshDb = 0, attack = 0;
};

class WoodBodyOneProcessor : public juce::AudioProcessor, private juce::AsyncUpdater
{
public:
    WoodBodyOneProcessor();
    ~WoodBodyOneProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Wood Body ONE"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.35; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    static Macro computeMacro (float wood01, int woodType);

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<bool> tipsOn { true };
    std::atomic<float> meterIn { 0.0f }, meterOut { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void loadIR (int woodType);
    void handleAsyncUpdate() override;
    std::array<juce::AudioBuffer<float>, 3> irs;
    double irRate = 48000.0;
    std::atomic<int> wantedWood { 1 };

    double sr = 48000.0;
    int loadedWood = -1;

    juce::dsp::Convolution convolver { juce::dsp::Convolution::Latency { 0 } };
    juce::AudioBuffer<float> dryBuf, wetBuf;

    using IIR = juce::dsp::IIR::Filter<float>;
    IIR rumble[2], warmth[2], grainHP[2], grainLP[2], dcBlock[2];
    juce::dsp::StateVariableTPTFilter<float> harshBand;

    float harshEnv[2] {}, fastEnv = 0, slowEnv = 0, attackGain = 1.0f;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> spacedDelay { 4800 };

    juce::SmoothedValue<float> outGain, bodySm, grainSm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WoodBodyOneProcessor)
};
