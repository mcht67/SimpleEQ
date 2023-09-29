/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
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


    float FilterAreaSize = area.getWidth() / 3;
    auto lowCutArea = slidersArea.removeFromLeft(FilterAreaSize);
    auto peakFilterArea = slidersArea.removeFromLeft(FilterAreaSize);
    auto highCutArea = slidersArea;

    float peakFilterSliderAreaSize = peakFilterArea.getHeight() / 3;
    auto peakFilterQualityArea = peakFilterArea.removeFromTop(peakFilterSliderAreaSize);
    auto peakFilterGainArea = peakFilterArea.removeFromTop(peakFilterSliderAreaSize);
    auto peakFilterFreqArea = peakFilterArea;

    lowCutFreqSlider.setBounds(lowCutArea);

    peakFilterQualitySlider.setBounds(peakFilterQualityArea);
    peakFilterGainSlider.setBounds(peakFilterGainArea);
    peakFilterFreqSlider.setBounds(peakFilterFreqArea);

    highCutFreqSlider.setBounds(highCutArea);
}

