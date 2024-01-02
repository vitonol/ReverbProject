/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace ParamIDs
{

    inline constexpr auto size{"size"};
    inline constexpr auto damp{"damp"};
    inline constexpr auto width{"width"};
    inline constexpr auto mix{"mix"};
    inline constexpr auto freeze{"freeze"};
    inline constexpr auto diffFeedbck{"diffFeedbck"};

}

static juce::AudioProcessorValueTreeState::ParameterLayout createParamLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto percent = [](float val, int /*maxStringLength*/)
    {
        if (val < 10.f)
            return juce::String(val, 2) + "%";
        else if (val < 100.f)
            return juce::String(val, 1) + "%";
        else
            return juce::String(val, 0) + "%";
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamIDs::size, 1},
                                                           ParamIDs::size,
                                                           juce::NormalisableRange<float>{0.f, 100.f, 0.01f, 1.f},
                                                           50.f,
                                                           juce::String(),
                                                           juce::AudioProcessorParameter::genericParameter,
                                                           percent,
                                                           nullptr));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamIDs::damp, 1},
                                                           ParamIDs::damp,
                                                           juce::NormalisableRange<float>{0.0f, 100.0f, 0.01f, 1.0f},
                                                           50.0f,
                                                           juce::String(),
                                                           juce::AudioProcessorParameter::genericParameter,
                                                           percent,
                                                           nullptr));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamIDs::width, 1},
                                                           ParamIDs::width,
                                                           juce::NormalisableRange<float>{0.0f, 100.0f, 0.01f, 1.0f},
                                                           50.0f,
                                                           juce::String(),
                                                           juce::AudioProcessorParameter::genericParameter,
                                                           percent,
                                                           nullptr));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamIDs::mix, 1},
                                                           ParamIDs::mix,
                                                           juce::NormalisableRange<float>{0.0f, 100.0f, 0.01f, 1.0f},
                                                           50.0f,
                                                           juce::String(),
                                                           juce::AudioProcessorParameter::genericParameter,
                                                           percent,
                                                           nullptr));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamIDs::diffFeedbck, 1},
                                                           ParamIDs::diffFeedbck,
                                                           juce::NormalisableRange<float>{0.0f, 100.0f, 0.01f, 1.0f},
                                                           50.0f,
                                                           juce::String(),
                                                           juce::AudioProcessorParameter::genericParameter,
                                                           percent,
                                                           nullptr));

    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ParamIDs::freeze, 1},
                                                          ParamIDs::freeze,
                                                          false));

    juce::StringArray stringArray;
    juce::String str;
    // for (int i = 0; i < 3; i++)
    // {
    //     str << (1 + i);
    //     // str << " some Kind Of Value";
    //     stringArray.add(str);
    // }

    stringArray.add(" juce::dsp::Reverb ");
    stringArray.add(" juce::Reverb ");
    // stringArray.add(" 3 ");

    layout.add(std::make_unique<juce::AudioParameterChoice>("Reverb Version", "Reverb Version", stringArray, 0));

    return layout;
}

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
                         ),
#endif
      apvts(*this, &undoManager, "Parameters", createParamLayout())
{
    auto storeFloatParam = [&apvts = this->apvts](auto &param, const auto &paramID)
    {
        param = dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter(paramID));
        jassert(param != nullptr);
    };

    storeFloatParam(size, ParamIDs::size);
    storeFloatParam(damp, ParamIDs::damp);
    storeFloatParam(width, ParamIDs::width);
    storeFloatParam(mix, ParamIDs::mix);
    storeFloatParam(diffFeedbck, ParamIDs::diffFeedbck);

    auto storeBoolParam = [&apvts = this->apvts](auto &param, const auto &paramID)
    {
        param = dynamic_cast<juce::AudioParameterBool *>(apvts.getParameter(paramID));
        jassert(param != nullptr);
    };

    storeBoolParam(freeze, ParamIDs::freeze);
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

    juce::dsp::ProcessSpec specs;

    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    specs.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

#if MYVERS
    r3.setSampleRate(sampleRate);

#elif !MYVERS
    r2.setSampleRate(specs.sampleRate);
#endif

    // r1.prepare(specs);
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
    params.roomSize = size->get() * 0.01f;
    params.damping = damp->get() * 0.01f;
    params.width = width->get() * 0.01f;
    params.wetLevel = mix->get() * 0.01f;
    params.dryLevel = 1.0f - mix->get() * 0.01f;
    params.freezeMode = freeze->get();

#if MYVERS
    params.diffusionFeedback = diffFeedbck->get() * 0.01f;
    r3.setParameters(params);
#elif !MYVERS
    r2.setParameters(params);
#endif

    // r1.setParameters(params);
}

// Settings getSettings(juce::AudioProcessorValueTreeState &vts)
// {
//     Settings settings;

//     settings.R = static_cast<EfxOption>(vts.getRawParameterValue("Reverb Version")->load());

//     return settings;
// }

void ReverbProjectAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    updateReverbParams();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);

    // r1.process(ctx);

    const auto &inputBlock = ctx.getInputBlock();
    auto &outputBlock = ctx.getOutputBlock();
    const auto numInChannels = inputBlock.getNumChannels();
    const auto numOutChannels = outputBlock.getNumChannels();
    const auto numSamples = outputBlock.getNumSamples();

    jassert(inputBlock.getNumSamples() == numSamples);

    outputBlock.copyFrom(inputBlock);

    if (ctx.isBypassed)
        return;

    if (numInChannels == 1 && numOutChannels == 1)
    {
#if MYVERS
        r3.processMono(outputBlock.getChannelPointer(0), (int)numSamples);
#elif !MYVERS
        // r2.processMono(outputBlock.getChannelPointer(0), (int)numSamples);
#endif
    }
    else if (numInChannels == 2 && numOutChannels == 2)
    {
#if MYVERS
        r3.processStereo(outputBlock.getChannelPointer(0),
                         outputBlock.getChannelPointer(1),
                         (int)numSamples);
#elif !MYVERS
        r2.processStereo(outputBlock.getChannelPointer(0),
                         outputBlock.getChannelPointer(1),
                         (int)numSamples);
#endif
    }
    else
    {
        jassertfalse; // invalid channel configuration
    }
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
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void ReverbProjectAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{

    auto valueTree = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));

    if (valueTree.isValid())
        apvts.replaceState(valueTree);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbProjectAudioProcessor();
}
