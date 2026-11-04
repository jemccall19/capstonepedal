#include  <vector>

class Delay 
{
    public:

        Delay(float delaySize, float wetMix, float sampleRate);

        float ProcessSample(float input);

        void reset();

        float getDelaySize() { return delaySize; }
        float getWetMix() { return wetMix; }
        float getSampleRate() { return sampleRate; }

        void setDelaySize(float newDelaySize);
        void setWetMix(float newWetMix);

    private:

        size_t delaySize;
        float wetMix;
        float sampleRate;

        std::vector<float> delayBuffer;
        size_t writeIndex = 0;
};