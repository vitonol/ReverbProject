/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReverbProjectAudioProcessor::ReverbProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
}

ReverbProjectAudioProcessor::~ReverbProjectAudioProcessor()
{
}

//==============================================================================
const juce::String ReverbProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReverbProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ReverbProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ReverbProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ReverbProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReverbProjectAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int ReverbProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReverbProjectAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String ReverbProjectAudioProcessor::getProgramName(int index)
{
    return {};
}

void ReverbProjectAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void ReverbProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // rvrb.setSampleRate(sampleRate);
    juce::dsp::ProcessSpec specs;
    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    specs.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    rvrb.prepare(specs);
}

void ReverbProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void ReverbProjectAudioProcessor::updateReverbParams()
{
    // params.roomSize   = size->get() * 0.01f;
    // params.damping    = damp->get() * 0.01f;
    // params.width      = width->get() * 0.01f;
    // params.wetLevel   = mix->get() * 0.01f;
    // params.dryLevel   = 1.0f - mix->get() * 0.01f;
    // params.freezeMode = freeze->get();

    // rvrb.setParameters (params);
}

void ReverbProjectAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    rvrb.process(ctx);

    // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //     buffer.clear(i, 0, buffer.getNumSamples());

    // dnBuffer.resize(buffer.getNumChannels(), 0.f);

    // for (int channel = 0; channel < totalNumInputChannels; ++channel)
    // {
    //     auto *channelData = buffer.getWritePointer(channel);
    //     rvrb.processStereo(&channelData[0], &channelData[1], buffer.getNumSamples());

    //     for (auto i = 0; i < buffer.getNumSamples(); i++)
    //     {
    //         const auto inputSample = channelData[i];

    //         // const auto proccessedSample = rvrb.processSample(i) * inputSample + dnBuffer[channel];

    //         // dnBuffer[channel] = inputSample * proccessedSample;

    //         // const auto rvrbOutput = 0.5f * inputSample * proccessedSample;

    //         // channelData[i] = rvrbOutput;
    //     }

    //     // ..do something to the data...
    // }
}

//==============================================================================
bool ReverbProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *ReverbProjectAudioProcessor::createEditor()
{
    // return new ReverbProjectAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ReverbProjectAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ReverbProjectAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbProjectAudioProcessor();
}
