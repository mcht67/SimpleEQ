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

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;

    CustomRotarySlider lowCutFreqSlider,
        highCutFreqSlider,
        peakFilterQualitySlider,
        peakFilterGainSlider,
        peakFilterFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment lowCutSlopeSliderAttachment,
                lowCutFreqSliderAttachment,
                peakFilterQualitySliderAttachment,
                peakFilterGainSliderAttachment,
                peakFilterFreqSliderAttachment,
                highCutSlopeSliderAttachment,
                highCutFreqSliderAttachment;

// definition in header?
    std::vector<juce::Component*> getComponents();
    //{
    //    return
    //    {
    //        &lowCutFreqSlider,
    //        &highCutFreqSlider,
    //        &peakFilterQualitySlider,
    //        &peakFilterGainSlider,
    //        &peakFilterFreqSlider,
    //        &lowCutSlopeSlider,
    //        &highCutSlopeSlider
    //    };
    //}





    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleEQAudioProcessorEditor)
};
