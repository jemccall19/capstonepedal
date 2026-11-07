#include "Delay.h"
#include <algorithm>
#include <cassert>
#include <cmath>


//simulation of std::clamp in newer version of C++
static inline float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

Delay::Delay(float maxDelaySize, float delaySize, float wetMix, float sampleRate)
    : writeIndex(0)
    , sampleRate(sampleRate)
    , currentDelay(delaySize)
    , targetDelay(delaySize)
    , currentWet(wetMix)
    , targetWet(wetMix)
    , currentFB(0.0f)
    , targetFB(0.0f)
{ 
    assert(maxDelaySize > 1.0f);
    assert(delaySize >= 1.0f && delaySize < maxDelaySize);
    assert(wetMix >= 0.0f && wetMix <= 1.0f);
    assert(sampleRate > 0.0f);

    //create buffer. +2 for interpolation and write
    bufferSize = static_cast<int>(std::ceil(maxDelaySize)) + 2;
    delayBuffer.assign(bufferSize, 0.0f);

    smooth = 1.0f - std::exp(-1.0f / (0.010f * sampleRate));

    feedbackLP.Init(sampleRate);
    feedbackLP.SetCutoff(4000.0f);
}

float Delay::ProcessSample(float input)
{
    
    //smooth values
    currentDelay += smooth * (targetDelay - currentDelay);
    currentWet += smooth * (targetWet - currentWet);
    currentFB += smooth * (targetFB - currentFB);

    //read follows write by [delay size] num of samples
    float readPos = static_cast<float>(writeIndex) - currentDelay;
    if (readPos < 0.0f)
        readPos += bufferSize;

    //linear interpolation. don't ask me how it works
    int i0 = static_cast<int>(readPos);
    int i1 = (i0 + 1) % bufferSize;
    float frac = readPos - static_cast<float>(i0);
    float delayedSample = delayBuffer[i0] + frac * (delayBuffer[i1] - delayBuffer[i0]);

   
    //run feedback through LPF. tanh is for limiting (also don't ask me how that works)
    float filtered = feedbackLP.Process(delayedSample);
    float feedbackSample = std::tanh(filtered * currentFB);

    //write into the buffer, then wrap around if needed
    delayBuffer[writeIndex] = input + feedbackSample;
    writeIndex = (writeIndex + 1) % bufferSize;

    //output current sample, based on wet vs dry values
    return (1.0f - currentWet) * input + currentWet * delayedSample;
}

void Delay::reset()
{
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
    writeIndex = 0;
    feedbackLP.Reset();
    currentDelay = targetDelay;
    currentWet = targetWet;
    currentFB = targetFB;
}

void Delay::setDelaySize(float newDelaySize)
{
    assert(newDelaySize >= 1.0f && newDelaySize < bufferSize - 2);
    targetDelay = clampf(newDelaySize, 1.0f, (float)(bufferSize - 3));
}

void Delay::setWetMix(float newWetMix)
{
    assert(newWetMix >= 0.0f && newWetMix <= 1.0f);
    targetWet = clampf(newWetMix, 0.0f, 1.0f);
}

void Delay::setFeedback(float newFeedback)
{
    targetFB = clampf(newFeedback, 0.0f, 0.95f);
}

void Delay::setLP(float cutoffHz)
{
    feedbackLP.SetCutoff(cutoffHz);
}
