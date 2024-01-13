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

// #include <immintrin.h> // @TODO optimize proceesing with SIMD
#include <JuceHeader.h>

//==============================================================================
/**
    Performs a reverb effect on a stream of audio data.

    This is a a modified version of simple JUCE stereo reverb, based on the technique and tunings used in FreeVerb.

*/
class ReverbFX
{
public:
    //==============================================================================
    ReverbFX()
    {
        setParameters(Parameters());
        setSampleRate(44100.0);
    }

    //==============================================================================

    // enum E_Color
    // {
    //     Bright,
    //     Dark,
    //     70s,
    //     80s
    // };

    /** Holds the parameters being used by a Reverb object. */
    struct Parameters
    {
        float roomSize = 0.5f;   /**< Room size, 0 to 1.0, where 1.0 is big, 0 is small. */
        float damping = 0.5f;    /**< Damping, 0 to 1.0, where 0 is not damped, 1.0 is fully damped. */
        float wetLevel = 0.33f;  /**< Wet level, 0 to 1.0 */
        float dryLevel = 0.4f;   /**< Dry level, 0 to 1.0 */
        float width = 1.0f;      /**< Reverb width, 0 to 1.0, where 1.0 is very wide. */
        float freezeMode = 0.0f; /**< Freeze mode - values < 0.5 are "normal" mode, values > 0.5
                                      put the reverb into a continuous feedback loop. */

        // Diffusion parameters
        float diffusionFeedback = 0.5f; /**< Diffusion feedback level, 0 to 1.0 */

        // E_Color color{Bright};
    };

    //==============================================================================
    /** Returns the reverb's current parameters. */
    const Parameters &getParameters() const noexcept { return parameters; }

    /** Applies a new set of parameters to the reverb.
        Note that this doesn't attempt to lock the reverb, so if you call this in parallel with
        the process method, you may get artifacts.
    */
    void setParameters(const Parameters &newParams)
    {
        const float wetScaleFactor = 3.0f;
        const float dryScaleFactor = 2.0f;

        const float wet = newParams.wetLevel * wetScaleFactor;
        dryGain.setTargetValue(newParams.dryLevel * dryScaleFactor);
        wetGain1.setTargetValue(0.5f * wet * (1.0f + newParams.width));
        wetGain2.setTargetValue(0.5f * wet * (1.0f - newParams.width));

        diffusionFeedback.setTargetValue(newParams.diffusionFeedback);

        gain = isFrozen(newParams.freezeMode) ? 0.0f : 0.015f;
        parameters = newParams;
        updateDamping();
    }

    //==============================================================================
    /** Sets the sample rate that will be used for the reverb.
        You must call this before the process methods, in order to tell it the correct sample rate.
    */
    void setSampleRate(const double sampleRate)
    {
        jassert(sampleRate > 0);

        static const short combTunings[] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617}; // (at 44100Hz)
        static const short allPassTunings[] = {556, 441, 341, 225};
        const int stereoSpread = 43;
        const int intSampleRate = (int)sampleRate;

        for (int i = 0; i < numCombs; ++i)
        {
            comb[0][i].setSize((intSampleRate * combTunings[i]) / 44100);
            comb[1][i].setSize((intSampleRate * (combTunings[i] + stereoSpread)) / 44100);
        }

        for (int i = 0; i < numAllPasses; ++i)
        {
            allPass[0][i].setSize((intSampleRate * allPassTunings[i]) / 44100);
            allPass[1][i].setSize((intSampleRate * (allPassTunings[i] + stereoSpread)) / 44100);
        }

        const short diffusionTunings[] = {
            116,
            208,
            301,
            353,
            420,
            585,
            666,
            750,
            999,
            1103,
            1200,
            1313,
            1535,
            1609,
            1685,
            1700,
        }; // Adjust these values based on experimentation

        for (int i = 0; i < numDiffusionCombs; ++i)
        {
            diffusion[0][i].setSize((intSampleRate * diffusionTunings[i]) / 44100);
            diffusion[1][i].setSize((intSampleRate * (diffusionTunings[i] + stereoSpread)) / 44100);
        }

        const double smoothTime = 0.01;
        damping.reset(sampleRate, smoothTime);
        feedback.reset(sampleRate, smoothTime);
        dryGain.reset(sampleRate, smoothTime);
        wetGain1.reset(sampleRate, smoothTime);
        wetGain2.reset(sampleRate, smoothTime);
    }

    /** Clears the reverb's buffers. */
    void reset()
    {
        for (int j = 0; j < numChannels; ++j)
        {
            for (int i = 0; i < numCombs; ++i)
                comb[j][i].clear();

            for (int i = 0; i < numAllPasses; ++i)
                allPass[j][i].clear();

            for (int i = 0; i < numDiffusionCombs; ++i)
                diffusion[j][i].clear();
        }
    }

    //==============================================================================
    /** Applies the reverb to two stereo channels of audio data. */
    void processStereo(float *const left, float *const right, const int numSamples) noexcept
    {
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC(6011)
        jassert(left != nullptr && right != nullptr);

        for (int i = 0; i < numSamples; ++i)
        {
            // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
            const float input = (left[i] + right[i]) * gain;
            float outL = 0, outR = 0;

            float diffOutL = 0, diffOutR = 0;
            const float diffFeedbck = 0.55f;

            const float damp = damping.getNextValue();
            const float feedbck = feedback.getNextValue();

            // Comb Filters
            for (int j = 0; j < numCombs; ++j)
            {
                outL += comb[0][j].process(input, damp, feedbck);
                outR += comb[1][j].process(input, damp, feedbck);
            }

            // All-Pass Filters
            for (int j = 0; j < numAllPasses; ++j)
            {
                outL = allPass[0][j].process(outL);
                outR = allPass[1][j].process(outR);
            }

            // Diffusion Filters
            for (int j = 0; j < numDiffusionCombs; ++j)
            {
                diffOutL += diffusion[0][j].process(input, diffFeedbck);
                diffOutR += diffusion[1][j].process(input, diffFeedbck);
            }

            const float dry = dryGain.getNextValue();
            const float wet1 = wetGain1.getNextValue();
            const float wet2 = wetGain2.getNextValue();

            const float WeightRatio = diffusionFeedback.getNextValue();

            const float combWeight = WeightRatio;          // Adjust as needed
            const float diffusionWeight = 1 - WeightRatio; // Adjust as needed

            // Weighted Summation:
            left[i] = (outL * combWeight + diffOutL * diffusionWeight) * wet1 + (outR * combWeight + diffOutR * diffusionWeight) * wet2 + left[i] * dry;
            right[i] = (outR * combWeight + diffOutR * diffusionWeight) * wet1 + (outL * combWeight + diffOutL * diffusionWeight) * wet2 + right[i] * dry;

            // // No diffusion
            // left[i] = outL * wet1 + outR * wet2 + left[i] * dry;
            // right[i] = outR * wet1 + outL * wet2 + right[i] * dry;
        }
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

    /** Applies the reverb to a single mono channel of audio data. */
    // For the time being mono does not use diffusion network for processing
    void processMono(float *const samples, const int numSamples) noexcept
    {
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC(6011)
        jassert(samples != nullptr);

        for (int i = 0; i < numSamples; ++i)
        {
            const float input = samples[i] * gain;
            float output = 0;

            const float damp = damping.getNextValue();
            const float feedbck = feedback.getNextValue();

            for (int j = 0; j < numCombs; ++j) // accumulate the comb filters in parallel
                output += comb[0][j].process(input, damp, feedbck);

            for (int j = 0; j < numAllPasses; ++j) // run the allpass filters in series
                output = allPass[0][j].process(output);

            const float dry = dryGain.getNextValue();
            const float wet1 = wetGain1.getNextValue();

            samples[i] = output * wet1 + samples[i] * dry;
        }
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

private:
    //==============================================================================
    static bool isFrozen(const float freezeMode) noexcept { return freezeMode >= 0.5f; }

    void updateDamping() noexcept
    {
        const float roomScaleFactor = 0.28f;
        const float roomOffset = 0.7f;
        const float dampScaleFactor = 0.4f;

        if (isFrozen(parameters.freezeMode))
            setDamping(0.0f, 1.0f);
        else
            setDamping(parameters.damping * dampScaleFactor,
                       parameters.roomSize * roomScaleFactor + roomOffset);
    }

    void setDamping(const float dampingToUse, const float roomSizeToUse) noexcept
    {
        damping.setTargetValue(dampingToUse);
        feedback.setTargetValue(roomSizeToUse);
    }

private:
    //==============================================================================
    class DiffusionFilter
    {
    public:
        DiffusionFilter() noexcept {}

        void setSize(const int size)
        {
            if (size != bufferSize)
            {
                bufferIndex = 0;
                buffer.malloc(size);
                bufferSize = size;
            }

            clear();
        }

        void clear() noexcept
        {
            buffer.clear((size_t)bufferSize);
        }

        float process(const float input, const float feedbackLevel) noexcept
        {
            const float output = buffer[bufferIndex];
            float temp = input + (output * feedbackLevel);

            // JUCE_UNDENORMALISE(temp);
            buffer[bufferIndex] = temp;
            bufferIndex = (bufferIndex + 1) % bufferSize;
            return output;
        }

    private:
        HeapBlock<float> buffer;
        int bufferSize = 0, bufferIndex = 0;

        JUCE_DECLARE_NON_COPYABLE(DiffusionFilter)
    };

    //==============================================================================
    class CombFilter
    {
    public:
        CombFilter() noexcept {}

        void setSize(const int size)
        {
            if (size != bufferSize)
            {
                bufferIndex = 0;
                buffer.malloc(size);
                bufferSize = size;
            }

            clear();
        }

        void clear() noexcept
        {
            last = 0;
            buffer.clear((size_t)bufferSize);
        }

        float process(const float input, const float damp, const float feedbackLevel) noexcept
        {
            const float output = buffer[bufferIndex];
            last = (output * (1.0f - damp)) + (last * damp);
            JUCE_UNDENORMALISE(last);

            float temp = input + (last * feedbackLevel);
            JUCE_UNDENORMALISE(temp);
            buffer[bufferIndex] = temp;
            bufferIndex = (bufferIndex + 1) % bufferSize;
            return output;
        }

    private:
        HeapBlock<float> buffer;
        int bufferSize = 0, bufferIndex = 0;
        float last = 0.0f;

        JUCE_DECLARE_NON_COPYABLE(CombFilter)
    };

    //==============================================================================
    class AllPassFilter
    {
    public:
        AllPassFilter() noexcept {}

        void setSize(const int size)
        {
            if (size != bufferSize)
            {
                bufferIndex = 0;
                buffer.malloc(size);
                bufferSize = size;
            }

            clear();
        }

        void clear() noexcept
        {
            buffer.clear((size_t)bufferSize);
        }

        float process(const float input) noexcept
        {
            const float bufferedValue = buffer[bufferIndex];
            float temp = input + (bufferedValue * 0.5f);
            JUCE_UNDENORMALISE(temp);
            buffer[bufferIndex] = temp;
            bufferIndex = (bufferIndex + 1) % bufferSize;
            return bufferedValue - input;
        }

    private:
        HeapBlock<float> buffer;
        int bufferSize = 0, bufferIndex = 0;

        JUCE_DECLARE_NON_COPYABLE(AllPassFilter)
    };

    //==============================================================================
    enum
    {
        numCombs = 8,
        numAllPasses = 4,
        numChannels = 2,
        numDiffusionCombs = 16
    };

    Parameters parameters;
    float gain;

    DiffusionFilter diffusion[numChannels][numDiffusionCombs];

    CombFilter comb[numChannels][numCombs];

    AllPassFilter allPass[numChannels][numAllPasses];

    SmoothedValue<float> damping, feedback, dryGain, wetGain1, wetGain2, diffusionFeedback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbFX)
};
