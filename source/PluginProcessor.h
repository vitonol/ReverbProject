/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

// #include <JuceHeader.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 */
class ReverbProjectAudioProcessor : public juce::AudioProcessor
{
public:
  //==============================================================================
  ReverbProjectAudioProcessor();
  ~ReverbProjectAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
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
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

private:
  juce::AudioProcessorValueTreeState apvts;

  juce::AudioParameterFloat *size{nullptr};
  juce::AudioParameterFloat *damp{nullptr};
  juce::AudioParameterFloat *width{nullptr};
  juce::AudioParameterFloat *mix{nullptr};
  juce::AudioParameterBool *freeze{nullptr};

  void updateReverbParams();

  juce::dsp::Reverb::Parameters params;
  // juce::dsp::Reverb rvrb;

  juce::Reverb r2;

  juce::UndoManager undoManager;

  // HeapBlock<float> buffer;
  // std::vector<float> dnBuffer;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbProjectAudioProcessor)
};
