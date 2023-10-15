/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override {};
};

struct RotarySliderWithLabels : juce::Slider {
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String unitSuffix) : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
        juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    void paint(juce::Graphics& g) override {};
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;

private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
    
};

struct ResponseCurveComponent : juce::Component {
    public:
        ResponseCurveComponent(SimpleEQAudioProcessor&);
        ~ResponseCurveComponent();
        
        void updateFilters();

    private:
        SimpleEQAudioProcessor& audioProcessor;
        Coefficients peakCoefficients;
        CoefficientsArray lowCutCoefficients;
        CoefficientsArray highCutCoefficients;
        void paint(juce::Graphics& g) override;
        void updateMagnitudes (std::vector<double>& magnitudes, int width);
        //void updateMagnitudeByCutCoefficients(double& mag, CoefficientsArray cutCoefficients);
};

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            juce::AudioProcessorParameter::Listener,
                                            juce::Timer
{
public:
    SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================


private:

    void paint(juce::Graphics&) override;
    void resized() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    void timerCallback() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged { false };

    RotarySliderWithLabels lowCutFreqSlider,
        highCutFreqSlider,
        peakFilterQualitySlider,
        peakFilterGainSlider,
        peakFilterFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    SliderAttachment lowCutSlopeSliderAttachment,
                    lowCutFreqSliderAttachment,
                    peakFilterQualitySliderAttachment,
                    peakFilterGainSliderAttachment,
                    peakFilterFreqSliderAttachment,
                    highCutSlopeSliderAttachment,
                    highCutFreqSliderAttachment;

    ResponseCurveComponent responseCurveComponent;

    //struct ResponseCurveComponent : juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer 
    //{
    //    ResponseCurveComponent(SimpleEQAudioProcessor&);
    //    ~ResponseCurveComponent();
    //};

    std::vector<juce::Component*> getComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleEQAudioProcessorEditor)
};
