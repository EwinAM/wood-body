#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class WoodLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WoodLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float start, float end, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int w, int h) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String& text, juce::Point<int> pos, juce::Rectangle<int> parent) override;
};

// Base-size content; the editor scales it to the window size
class WoodBodyContent : public juce::Component, private juce::Timer
{
public:
    explicit WoodBodyContent (WoodBodyOneProcessor&);
    ~WoodBodyContent() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    static constexpr int W = 460, H = 640;

private:
    void timerCallback() override;
    void applyTips();
    void syncButtons();

    WoodBodyOneProcessor& proc;
    WoodLookAndFeel lnf;

    juce::Slider wood, output;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> woodAtt, outAtt;
    juce::TextButton woodBtn[3], stBtn[3], tipsBtn;
    juce::Label woodVal, outVal;

    float meterIn = 0, meterOut = 0;
    juce::Rectangle<int> displayArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WoodBodyContent)
};

class WoodBodyOneEditor : public juce::AudioProcessorEditor
{
public:
    explicit WoodBodyOneEditor (WoodBodyOneProcessor&);
    ~WoodBodyOneEditor() override = default;
    void paint (juce::Graphics& g) override { g.fillAll (juce::Colour (0xff2a2a2c)); }
    void resized() override;

private:
    WoodBodyContent content;
    juce::TooltipWindow tooltip { this, 500 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WoodBodyOneEditor)
};
