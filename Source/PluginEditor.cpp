/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// LookAndFeel
//==============================================================================
void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    float sliderPosProportional,
                                    float rotaryStartAngle,
                                    float rotaryEndAngle,
                                    juce::Slider&)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);

    // draw circle area
    g.setColour(juce::Colours::aliceblue);
    g.fillEllipse(bounds);

    // draw circle border
    g.setColour(juce::Colours::magenta);
    g.drawEllipse(bounds, 1.f);

    // draw hand
    juce::Point<float> center = bounds.getCentre();
    juce::Path p;

    juce::Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());

    p.addRectangle(r);

    jassert(rotaryStartAngle < rotaryEndAngle);

    auto sliderAngRad = juce::jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

    p.applyTransform(juce::AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

    g.fillPath(p);
}


// RotarySliderWithLabels
//==============================================================================

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    constexpr float startAng = juce::degreesToRadians(180.f + 45.f);
    constexpr float endAng = juce::degreesToRadians(180.f - 45.f) + juce::MathConstants<float>::twoPi;

    juce::Range<double> range = getRange();
    juce::Rectangle<int> sliderBounds = getSliderBounds();

    g.setColour(juce::Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(juce::Colours::yellow);
    g.drawRect(sliderBounds);


    getLookAndFeel().drawRotarySlider(g,
                                    sliderBounds.getX(),
                                    sliderBounds.getY(),
                                    sliderBounds.getWidth(),
                                    sliderBounds.getHeight(),
                                    juce::jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                    startAng,
                                    endAng,
                                    *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight();

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

// ResponseCurveComponent
//==============================================================================
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
    double sampleRate = audioProcessor.getSampleRate();

    for (int i = 0; i < width; i++)
    {
        double mag = 1.f;

        double freq = juce::mapToLog10(double(i) / double(width), 20.0, 20000.0);

        if (peakCoefficients != nullptr)
        { 
            double peakCoeff = peakCoefficients->getMagnitudeForFrequency(freq, sampleRate);
            mag *= peakCoeff;
        }

        for (auto filter : lowCutCoefficients)
        {
            if (filter != nullptr)
            {
                mag *= filter->getMagnitudeForFrequency(freq, sampleRate);
            }
        }

        for (auto filter : highCutCoefficients)
        {
            if (filter != nullptr)
            {
                mag *= filter->getMagnitudeForFrequency(freq, sampleRate);
            }
        }
        
        magnitudes[i] = juce::Decibels::gainToDecibels(mag);
    }
}

//void ResponseCurveComponent::updateMagnitudeByCutCoefficients(double& mag, CoefficientsArray cutCoefficients)
//{
//    for (auto filter : cutCoefficients)
//    {
//        if (filter != nullptr)
//        {
//            mag *= filter->getMagnitudeForFrequency(freq, sampleRate);
//        }
//    }
//}

void ResponseCurveComponent::updateFilters()
{
    ChainSettings chainSettings = getChainSettings(audioProcessor.apvts);
    double sampleRate = audioProcessor.getSampleRate();
    this->peakCoefficients = makePeakFilter(chainSettings, sampleRate);
    this->lowCutCoefficients = makeLowCutFilter(chainSettings, sampleRate);
    this->highCutCoefficients = makeHighCutFilter(chainSettings, sampleRate);
}


//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    // Slider
    peakFilterFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakFilterGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "Hz"),
    peakFilterQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "db/Oct"),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "db/Oct"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),

    // response curve
    responseCurveComponent(audioProcessor),

    // Slider Attachments
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
        responseCurveComponent.updateFilters();
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

