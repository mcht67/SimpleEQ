/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider {
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
    
    }
};

struct ResponseCurveComponent : juce::Component {
    public:
        ResponseCurveComponent(SimpleEQAudioProcessor& audioProcessor);
        ~ResponseCurveComponent();

        double sampleRate;
        void ResponseCurveComponent::updateFilterCoefficients(const ChainSettings& chainSettings);

    private:
        Coefficients peakCoefficients;
        CoefficientsArray lowCutCoefficients;
        CoefficientsArray highCutCoefficients;
        void paint(juce::Graphics& g) override;
        void updateMagnitudes (std::vector<double>& magnitudes, int width);
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
    void paint(juce::Graphics&) override;
    void resized() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged { false };

    CustomRotarySlider lowCutFreqSlider,
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
