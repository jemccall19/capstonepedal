#include "Delay.h"
#include <algorithm>
#include <cassert>

Delay::Delay(float delaySize, float wetMix, float sampleRate)
    : delaySize(delaySize)
    , wetMix(wetMix)
    , sampleRate(sampleRate)
    , writeIndex(0)
{ 
    assert(delaySize > 0.0f);
    assert(wetMix >= 0.0f && wetMix <= 1.0f);
    assert(sampleRate > 0.0f);

    delayBuffer.resize(static_cast<int>(delaySize), 0.0f);
}

float Delay::ProcessSample(float input)
{
    const float delayedSample = delayBuffer[writeIndex];
    delayBuffer[writeIndex] = input;
    writeIndex = (writeIndex + 1) % delaySize;

    return (1.0f - wetMix) * input + wetMix * delayedSample;
}

void Delay::reset()
{
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
    writeIndex = 0;
}

void Delay::setDelaySize(float newDelaySize)
{
    assert(newDelaySize > 0.0f);
    delaySize = newDelaySize;
    delayBuffer.resize(delaySize, 0.0f);
}

void Delay::setWetMix(float newWetMix)
{
    assert(newWetMix >= 0.0f && newWetMix <= 1.0f);
    wetMix = newWetMix;
}
