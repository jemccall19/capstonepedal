#pragma once
#include  <vector>
#include "OnePoleLP.h"

class Delay 
{
    public:

        Delay(float maxDelaySize, float delaySize, float wetMix, float sampleRate);

        float ProcessSample(float input);

        void reset();

        float getDelaySize() { return targetDelay; }
        float getWetMix() { return targetWet; }
        float getSampleRate() { return sampleRate; }

        void setDelaySize(float newDelaySize);
        void setWetMix(float newWetMix);
        void setFeedback(float newFeedback);
        void setLP(float cutoffHz);

    private:

        std::vector<float> delayBuffer;
        size_t bufferSize;
        size_t writeIndex;
        float sampleRate;

        float currentDelay, targetDelay;
        float currentWet, targetWet;
        float currentFB, targetFB;
        float smooth;

        OnePoleLP feedbackLP;

};