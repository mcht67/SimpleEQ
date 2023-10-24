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
                                    juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);

    // draw circle area
    g.setColour(juce::Colours::blanchedalmond);
    g.fillEllipse(bounds);

    // draw circle border
    g.setColour(juce::Colours::midnightblue);
    g.drawEllipse(bounds, 1.f);

    // check if it is a rotarySliderWithLabels
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*> (&slider))
    {
        // draw hand
        juce::Point<float> center = bounds.getCentre();
        juce::Path p;

        juce::Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = juce::jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(juce::AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        // draw text box
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(juce::Colours::black);
        g.fillRect(r);

        g.setColour(juce::Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);

    }
}


// RotarySliderWithLabels
//==============================================================================

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    constexpr float startAng = juce::degreesToRadians(180.f + 45.f);
    constexpr float endAng = juce::degreesToRadians(180.f - 45.f) + juce::MathConstants<float>::twoPi;

    juce::Range<double> range = getRange();
    juce::Rectangle<int> sliderBounds = getSliderBounds();

    // Slider and Component Bounds
    //g.setColour(juce::Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(juce::Colours::yellow);
    //g.drawRect(sliderBounds);


    getLookAndFeel().drawRotarySlider(g,
                                    sliderBounds.getX(),
                                    sliderBounds.getY(),
                                    sliderBounds.getWidth(),
                                    sliderBounds.getHeight(),
                                    juce::jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                    startAng,
                                    endAng,
                                    *this);
    // draw min/max values
    juce::Point<float> center = sliderBounds.toFloat().getCentre();
    float radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(juce::Colours::honeydew);
    g.setFont(getTextHeight());


    int numChoices = labels.size();
    for (size_t i = 0; i < numChoices; i++)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = juce::jmap(pos, 0.f, 1.f, startAng, endAng);

        // get center of textbox
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        // place textbox
        juce::Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);

    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    // get min length
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    // substract text height
    size -= getTextHeight();

    // make square
    juce::Rectangle<int> r;
    r.setSize(size, size);
    // center in x
    r.setCentre(bounds.getCentreX(), 0);
    // move to top
    r.setY(2);

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        return choiceParam->getCurrentChoiceName();
    }

    bool addK = false;
    juce::String str;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();

        if (val > 999.99f)
        {
            addK = true;
            val /= 1000.f;
        }

        str = juce::String(val, (addK ? 2 : 0));


        if (suffix.isNotEmpty())
        {
            str << " ";

            if (addK)
                str << "k";

            str << suffix;
        }

        return str;
    }
    else
        jassertfalse;
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
    auto analysisArea = getAnalysisArea();
    auto renderArea = getRenderArea();

    // draw backgroundGrid
    g.drawImageAt(backgroundGrid, renderArea.getX(), renderArea.getY());

    //g.setColour(juce::Colours::yellow);
    //g.drawRect(getAnalysisArea());

    //g.setColour(juce::Colours::red);
    //g.drawRect(getRenderArea());

    // draw ResponseCurve
    int width = analysisArea.getWidth();

    std::vector<double> magnitudes(width, 1.f);

    updateMagnitudes(magnitudes, width);

    juce::Path responseCurve;

    const double outputMin = analysisArea.getBottom();
    const double outputMax = analysisArea.getY();

    // lambda to map input value y-coordinate
    auto map = [outputMin, outputMax](double input)
        {
            return juce::jmap(input, -24.0, 24.0, outputMin, outputMax);
        };

    responseCurve.startNewSubPath( analysisArea.getX(), map(magnitudes[0]) );

    for (size_t i = 1; i < magnitudes.size(); i++)
    {
        double y = map(magnitudes[i]);
        //if (y < renderArea.getBottom())
        responseCurve.lineTo( analysisArea.getX() + i, map(magnitudes[i]) );
    }

    g.setColour(juce::Colours::white);
    g.strokePath( responseCurve, juce::PathStrokeType(2.f) );

    g.setColour(juce::Colours::orange);
    g.drawRoundedRectangle(renderArea.toFloat(), 4.f, 2.f);

};

void ResponseCurveComponent::resized() {
    
    // preparation backgroundGrid    
    juce::Rectangle<float> analysisArea = getAnalysisArea().toFloat();
    juce::Rectangle<float> renderArea = getRenderArea().toFloat();

    backgroundGrid = juce::Image(juce::Image::PixelFormat::RGB, renderArea.getWidth(), renderArea.getHeight(), true);
    juce::Graphics g(backgroundGrid);

    std::vector<float> freqs = {
        20, 30, 40, 50, 100,
        200, 300, 400, 500, 1000,
        2000, 3000, 4000, 5000, 10000,
        20000
    };
    
    std::vector<float> gains = {
        -24.f, -12.f, 0.f, 12.f, 24.f
    };

    //auto left = analysisArea.getX();
    //auto right = analysisArea.getRight();
    //auto top = analysisArea.getY();
    //auto bottom = analysisArea.getBottom();
    //auto width = analysisArea.getWidth();
    
    // drawing backgroundGrid
    g.setColour(juce::Colours::steelblue);

    for (float f : freqs) {
        // map to normalised value
        float normX = juce::mapFromLog10(f, 20.f, 20000.f);

        // draw line
        g.drawVerticalLine(analysisArea.getWidth() * normX, 0.f, renderArea.getHeight());
    }
    auto y_offset = renderArea.getBottom() - analysisArea.getBottom();
    for (float gain : gains)
    {
        // normalise
        auto y = juce::jmap(gain, -24.f, 24.f, 0.f, analysisArea.getHeight());
        g.setColour(gain == 0 ? juce::Colours::greenyellow : juce::Colours::steelblue);
        g.drawHorizontalLine(y + y_offset, 0.f, renderArea.getWidth());
    }
};

juce::Rectangle<int> ResponseCurveComponent::getRenderArea() {
    auto renderArea = this->getBounds();
    renderArea.removeFromTop(15);
    renderArea.removeFromBottom(2);
    renderArea.removeFromLeft(20);
    renderArea.removeFromRight(20);
    return renderArea;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea() {

    auto analysisArea = getRenderArea();
    analysisArea.removeFromTop(5);
    analysisArea.removeFromBottom(5);

    return analysisArea;
}

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
    peakFilterGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
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

    // min-max values and their positions
    peakFilterQualitySlider.labels.add({ 0.f, "0.1" });
    peakFilterQualitySlider.labels.add({ 1.f, "10" });
    peakFilterGainSlider.labels.add({ 0.f, "-24dB" });
    peakFilterGainSlider.labels.add({ 1.f, "+24dB" });
    peakFilterFreqSlider.labels.add({ 0.f, "20Hz" });
    peakFilterFreqSlider.labels.add({ 1.f, "20kHz" });

    lowCutSlopeSlider.labels.add({ 0.f, "12db/Oct" });
    lowCutSlopeSlider.labels.add({ 1.f, "48db/Oct" });
    lowCutFreqSlider.labels.add({ 0.f, "20Hz" });
    lowCutFreqSlider.labels.add({ 1.f, "20kHz" });

    highCutSlopeSlider.labels.add({ 0.f, "12db/Oct" });
    highCutSlopeSlider.labels.add({ 1.f, "48db/Oct" });
    highCutFreqSlider.labels.add({ 0.f, "20Hz" });
    highCutFreqSlider.labels.add({ 1.f, "20kHz" });
    
    for (auto* component : SimpleEQAudioProcessorEditor::getComponents())
    {
        addAndMakeVisible(component);
    }

    const juce::Array<juce::AudioProcessorParameter*>& params = audioProcessor.getParameters();

    for (auto param : params)
    {
        param->addListener(this);
    }

    responseCurveComponent.updateFilters();

    startTimer(60);

    setSize (800, 500);
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
    auto hRatio = 37.f / 100.f;

    auto responseCurveArea = area.removeFromTop(area.getHeight() * hRatio);
    auto slidersArea = area;

    // margins
    slidersArea.removeFromTop(7);
    slidersArea.removeFromBottom(7);
   
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

