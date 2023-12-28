/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReverbProjectAudioProcessorEditor::ReverbProjectAudioProcessorEditor(ReverbProjectAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);
}

ReverbProjectAudioProcessorEditor::~ReverbProjectAudioProcessorEditor()
{
}

//==============================================================================
void ReverbProjectAudioProcessorEditor::paint(juce::Graphics &g)
{
  // (Our component is opaque, so we must completely fill the background with a solid colour)
  // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  g.fillAll(juce::Colours::aquamarine.brighter().brighter());

  g.setColour(juce::Colours::aquamarine.darker());
  g.setFont(20.0f);
  g.drawFittedText("Reverbbberations", getLocalBounds(), juce::Justification::centredTop, 1);
}

void ReverbProjectAudioProcessorEditor::resized()
{
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}
