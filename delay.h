#pragma once
#include  <vector>
#include "Effect.h"
#include "OnePoleLP.h"

class Delay : public Effect
{
    public:

        Delay(float maxDelaySize, float delaySize, float wetMix, float sampleRate);

        //Effect class
        void  Process(float* buf, size_t n) override;
        void  Reset() override;
        int   NumParams() const override { return 3; }
        void  SetParam(int index, float value01) override;
        const char* Name() const override { return "Delay"; }
        
        //Delay class
        float ProcessSample(float input);
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