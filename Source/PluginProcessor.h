/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = Filter::CoefficientsPtr;
using CoefficientsArray = juce::ReferenceCountedArray<Coefficients>; // unfortunately doesnt work needs more investigation

enum FilterSlope
{
    _12dB,
    _24dB,
    _36dB,
    _48dB,
};

struct ChainSettings // Initialisierung in header???
{
    float peakFreq { 0 }, peakGainInDecibel { 0 }, peakQuality { 0 };
    float lowCutFreq { 0 }, highCutFreq { 0 };
    FilterSlope lowCutSlope { FilterSlope::_12dB }, highCutSlope { FilterSlope::_12dB };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

class SimpleEQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    
    Coefficients makePeakFilter(const ChainSettings& chainSettings);
    auto makeLowCutFilter(const ChainSettings& chainSettings);
    auto makeHighCutFilter(const ChainSettings& chainSettings);

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() }; // why initialized in header? this way it possibly gets copied and duplicated which causes a linking error

private:

    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;

    enum ChainPositions
    {
        LowCut,
        Peak,
        HighCut
    };

    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

    // why has index to be part of the template?
    template <int index, typename ChainElementType, typename CoefficientsType>
    void updateCutFilterElement(ChainElementType& cutFilter, const CoefficientsType& cutCoefficients)
    {
        updateCoefficients(cutFilter.get<index>().coefficients, cutCoefficients[index]);
        cutFilter.setBypassed<index>(false);
    }

    template <typename ChainElementType, typename CoefficientType>
    void updateCutFilter(ChainElementType& cutFilter,
                          const CoefficientType& cutCoefficients,
                          const FilterSlope& filterSlope)
    {
        cutFilter.setBypassed<0>(true);
        cutFilter.setBypassed<1>(true);
        cutFilter.setBypassed<2>(true);
        cutFilter.setBypassed<3>(true);

        switch ( filterSlope )
        {

        case _48dB:
        {
            updateCutFilterElement<3>(cutFilter, cutCoefficients);
        }
        case _36dB:
        {
            updateCutFilterElement<2>(cutFilter, cutCoefficients);
        }
        case _24dB:
        {
            updateCutFilterElement<1>(cutFilter, cutCoefficients);
        }
        case _12dB:
        {
            updateCutFilterElement<0>(cutFilter, cutCoefficients);
        }
        }
    }

    void updatePeakFilter(const ChainSettings& chainSettings);

    void updateLowCutFilter(const ChainSettings& chainSettings);

    void updateHighCutFilter(const ChainSettings& chainSettings);

    void updateFilters();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
