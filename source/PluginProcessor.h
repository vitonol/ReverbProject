/*
  ==============================================================================

   Copyright 2023, 2024 Vitalii Voronkin

   Reverb Project is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Reverb Project is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Simple Reverb. If not, see <http://www.gnu.org/licenses/>.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReverbFX.h"

// @TODO remove JuceHeader and only add classes that you will need:
// #include <juce_audio_processors/juce_audio_processors.h>
// #include <juce_dsp/juce_dsp.h>
// #include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_gui_basics/juce_gui_basics.h>

#define MYVERS 1 // toggles between juce::reverb and my modified version, for comparasing

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
  juce::AudioParameterFloat *diffFeedbck{nullptr};

  juce::AudioParameterFloat *highCutFreq{nullptr};
  juce::AudioParameterFloat *LowCutFreq{nullptr};

  // juce::AudioParameterChoice *color{nullptr};

  void updateReverbParams();

#if MYVERS
  using Parameters = ReverbFX::Parameters;
  Parameters params;
  ReverbFX r3;
#elif !MYVERS

  juce::dsp::Reverb::Parameters params;
  juce::Reverb r2;
#endif

  juce::UndoManager undoManager;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbProjectAudioProcessor)
};
