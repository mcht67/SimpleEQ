/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    peakFilterQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakFilterQualitySlider),
    peakFilterGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakFilterGainSlider),
    peakFilterFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFilterFreqSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* component : SimpleEQAudioProcessorEditor::getComponents())
    {
        addAndMakeVisible(component);
    }

    setSize (400, 300);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));


}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto area = getLocalBounds();

    auto curveArea = area.removeFromTop(area.getHeight() * 0.33);
    auto slidersArea = area;


    float FilterAreaSize = area.getWidth() * 0.33;
    auto lowCutArea = slidersArea.removeFromLeft(FilterAreaSize);
    auto peakFilterArea = slidersArea.removeFromLeft(FilterAreaSize);
    auto highCutArea = slidersArea;

    auto lowCutSlopeSliderArea = lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5);
    auto lowCutFreqSliderArea = lowCutArea;

    float peakFilterSliderAreaSize = peakFilterArea.getHeight() * 0.33;
    auto peakFilterQualityArea = peakFilterArea.removeFromTop(peakFilterSliderAreaSize);
    auto peakFilterGainArea = peakFilterArea.removeFromTop(peakFilterSliderAreaSize);
    auto peakFilterFreqArea = peakFilterArea;

    auto highCutSlopeSliderArea = highCutArea.removeFromTop(highCutArea.getHeight() * 0.5);
    auto highCutFreqSliderArea = highCutArea;

    lowCutSlopeSlider.setBounds(lowCutSlopeSliderArea);
    lowCutFreqSlider.setBounds(lowCutFreqSliderArea);

    peakFilterQualitySlider.setBounds(peakFilterQualityArea);
    peakFilterGainSlider.setBounds(peakFilterGainArea);
    peakFilterFreqSlider.setBounds(peakFilterFreqArea);

    highCutSlopeSlider.setBounds(highCutSlopeSliderArea);
    highCutFreqSlider.setBounds(highCutFreqSliderArea);
}

void SimpleEQAudioProcessorEditor::parameterValueChanged (int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void SimpleEQAudioProcessorEditor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting) { };

void SimpleEQAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        // getSettings

        // updateCurve

        // repaint
    }
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComponents() {
    return
    {
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &peakFilterQualitySlider,
        &peakFilterGainSlider,
        &peakFilterFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}

