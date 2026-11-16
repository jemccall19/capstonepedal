#pragma once
#include <cmath>
#include "OnePoleLP.h"

// Bitcrusher: reduces bit depth (quantization) and sample rate (sample-and-hold),
// with an input gain stage before the crush and a lowpass + output gain after it.
//
// Signal path:
//   in -> input gain -> clip to +/-1 -> sample-and-hold -> quantize
//      -> lowpass -> output gain -> (mixed with the untouched dry input)
class Bitcrusher
{
public:
    Bitcrusher() { SetBits(16.0f); }

    // Call once before audio starts
    void Init(float sampleRate)
    {
        sampleRate_ = sampleRate;
        lp_.Init(sampleRate);
        SetTone(8000.0f);
        Reset();
    }

    // bits: 1 (harsh) to 16 (clean). Fractional values work, so a knob can sweep smoothly.
    void SetBits(float bits)
    {
        if (bits < 1.0f)  bits = 1.0f;
        if (bits > 16.0f) bits = 16.0f;
        steps_ = std::exp2(bits - 1.0f);   // quantization levels per side of zero
    }

    // factor: 1 = no reduction, 2 = hold each sample for 2 samples (half rate), etc.
    // Fractional values work.
    void SetDownsample(float factor)
    {
        if (factor < 1.0f) factor = 1.0f;
        factor_ = factor;
    }

    // Gain applied before the crush (linear, 1.0 = unchanged). Above 1.0 pushes the signal
    // harder into the quantizer and clips at +/-1, which adds drive.
    void SetInputGain(float gain)
    {
        inGain_ = gain < 0.0f ? 0.0f : gain;
    }

    // Gain applied to the crushed signal (linear, 1.0 = unchanged), after the lowpass.
    // Use it to bring the level back down after boosting the input.
    void SetOutputGain(float gain)
    {
        outGain_ = gain < 0.0f ? 0.0f : gain;
    }

    // Post lowpass cutoff in Hz. Lower = darker and less harsh.
    // A value near or above the Nyquist limit turns the filter off entirely.
    // Call once per block, not per sample (it computes exp() internally).
    void SetTone(float hz)
    {
        lpOn_ = hz < 0.45f * sampleRate_;
        if (lpOn_) lp_.SetCutoff(hz);
    }

    // 0 = dry only, 1 = crushed only
    void SetMix(float mix)
    {
        if (mix < 0.0f) mix = 0.0f;
        if (mix > 1.0f) mix = 1.0f;
        mix_ = mix;
    }

    float ProcessSample(float in)
    {
        // Sample-rate reduction: grab a new sample only every `factor_` samples,
        // otherwise keep repeating the last one.
        counter_ += 1.0f;
        if (counter_ >= factor_)
        {
            counter_ -= factor_;

            // Input gain, then clip so the quantizer never sees values beyond +/-1
            float x = in * inGain_;
            if (x >  1.0f) x =  1.0f;
            if (x < -1.0f) x = -1.0f;

            // Bit-depth reduction: snap the sample to the nearest of a limited set of levels
            held_ = std::round(x * steps_) / steps_;
        }

        // Post lowpass smooths the staircase and tames the harsh high frequencies
        float wet = lpOn_ ? lp_.Process(held_) : held_;
        wet *= outGain_;

        // The dry path is the original, unprocessed input
        return (1.0f - mix_) * in + mix_ * wet;
    }

    void Reset()
    {
        counter_ = 0.0f;
        held_    = 0.0f;
        lp_.Reset();
    }

private:
    float sampleRate_ = 48000.0f;
    float steps_      = 32768.0f;
    float factor_     = 1.0f;
    float inGain_     = 1.0f;
    float outGain_    = 1.0f;
    float mix_        = 1.0f;
    float counter_    = 0.0f;
    float held_       = 0.0f;
    bool  lpOn_       = true;
    OnePoleLP lp_;
};