/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& audioProcessor)
    : audioProcessor(audioProcessor)
{

}

ResponseCurveComponent::~ResponseCurveComponent()
{

}

void ResponseCurveComponent::paint(juce::Graphics& g) 
{
    ////using namespace juce;

    int width = getWidth();

    std::vector<double> magnitudes(width, 1.f);

    updateMagnitudes(magnitudes, width);

    // DRAWING
    
    juce::Path responseCurve;

    const double outputMin = getBottom();
    const double outputMax = getY();

    // lambda to map input value y-coordinate
    auto map = [outputMin, outputMax](double input)
        {
            return juce::jmap(input, -24.0, 24.0, outputMin, outputMax);
        };

    responseCurve.startNewSubPath( getX(), map(magnitudes[0]) );

    for (size_t i = 1; i < magnitudes.size(); i++)
    {
        responseCurve.lineTo( getX() + i, map(magnitudes[i]) );
    }

    g.setColour(juce::Colours::orange);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.f, 1.f);

    g.setColour(juce::Colours::white);
    g.strokePath( responseCurve, juce::PathStrokeType(2.f) );

};

void ResponseCurveComponent::updateMagnitudes(std::vector<double>& magnitudes, int width)
{
    ChainSettings chainSettings = getChainSettings(audioProcessor.apvts);
    double sampleRate = audioProcessor.getSampleRate();

    for (int i = 0; i < width; i++)
    {
        double mag = 1.f;

        double freq = juce::mapToLog10(double(i) / double(width), 20.0, 20000.0);

        this->peakCoefficients = makePeakFilter(chainSettings, sampleRate);

        double peakCoeff = peakCoefficients->getMagnitudeForFrequency(freq,  sampleRate);

        mag *= peakCoeff; 

        this->lowCutCoefficients = makeLowCutFilter(chainSettings, sampleRate);

        switch (chainSettings.lowCutSlope)
        {

            case _48dB:
            {
                double lowCut3Coeff = lowCutCoefficients[3]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= lowCut3Coeff;
            }
            case _36dB:
            {
                double lowCut2Coeff = lowCutCoefficients[2]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= lowCut2Coeff;
            }
            case _24dB:
            {
                double lowCut1Coeff = lowCutCoefficients[1]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= lowCut1Coeff;
            }
            case _12dB:
            {
                double lowCut0Coeff = lowCutCoefficients[0]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= lowCut0Coeff;
            }
        }
        this->highCutCoefficients = makeHighCutFilter(chainSettings, sampleRate);
        
        switch (chainSettings.highCutSlope)
        {

            case _48dB:
            {
                double highCut3Coeff = highCutCoefficients[3]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= highCut3Coeff;
            }
            case _36dB:
            {
                double highCut2Coeff = highCutCoefficients[2]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= highCut2Coeff;
            }
            case _24dB:
            {
                double highCut1Coeff = highCutCoefficients[1]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= highCut1Coeff;
            }
            case _12dB:
            {
                double highCut0Coeff = highCutCoefficients[0]->getMagnitudeForFrequency(freq, sampleRate);
                mag *= highCut0Coeff;
            }
        }
        magnitudes[i] = juce::Decibels::gainToDecibels(mag);
    }
}


//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    peakFilterQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakFilterQualitySlider),
    peakFilterGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakFilterGainSlider),
    peakFilterFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFilterFreqSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    responseCurveComponent(audioProcessor)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* component : SimpleEQAudioProcessorEditor::getComponents())
    {
        addAndMakeVisible(component);
    }

    const juce::Array<juce::AudioProcessorParameter*>& params = audioProcessor.getParameters();

    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimer(60);

    setSize (400, 300);
}



SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    const juce::Array<juce::AudioProcessorParameter*>& params = audioProcessor.getParameters();

    for (auto param : params)
    {
        param->removeListener(this);
    }
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

    auto responseCurveArea = area.removeFromTop(area.getHeight() * 0.33);
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

    responseCurveComponent.setBounds(responseCurveArea);

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
        responseCurveComponent.repaint();
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
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}

