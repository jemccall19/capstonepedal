#pragma once
#include <cassert>
#include <cmath>
#include "OnePoleLP.h"
#include "LFO.h"
#include "Sdrampool.h"

class Reverb1
{
    public:

    Reverb1() {}

    void Init(float wetMix, float sampleRate);
    float ProcessSample(float input); //mono out
    void ProcessStereo(float input, float* outL, float* outR); //stereo out
    void Reset();
    void SetDecay(float newDecay); //0.0-0.98
    void SetDamping(float cutoffHz); //lowpass on wet
    void SetBandwidth(float cutoffHz); //lowpass on input
    void SetPreDelay(float ms); // <=100ms
    void SetModDepth(float samples); //dattorro's "excursion", max ~25.8 samples
    void SetWetMix(float newWetMix); //0.0-1.0 dry/wet

    private:

    struct DelayLine
    {
        void Init(int maxDelay)
        {
            size = maxDelay + 4; // +4 to account for LFO swing and interpolation
            buffer = SdramAlloc(static_cast<size_t>(size)); //allocate buffer size in SDRAM
            writeIndex = 0;
            assert(buffer != nullptr);
        }

        void Write(float input)
        {
            buffer[writeIndex] = input; //write input
            if (++writeIndex >= size) writeIndex = 0; //wrap if needed
        }

        float Read(int p) const
        {
            int i = writeIndex - p; //follow behind write by p samples
            if (i < 0) i+=size; //wrap around if needed
            return buffer[i];
        }

        float ReadFrac(float d) const
        {
            int whole = static_cast<int>(d);
            float frac = d - static_cast<float>(whole);

            //interpolate between the two values
            int i0 = writeIndex - whole;
            if (i0 < 0) i0+=size;
            int i1 = i0 - 1;
            if (i1 < 0) i1+=size;

            return buffer[i0] + frac*(buffer[i1] - buffer[i0]);
        }

        void Reset()
        {
            if (buffer == nullptr) return;
            for (int i = 0; i < size; i++) buffer[i] = 0.0f;
            writeIndex = 0;
        }

        float* buffer = nullptr;
        int size = 1;
        int writeIndex = 0;
    };

    static float Allpass(DelayLine& dL1, int n, float g, float x);
    static float AllPassMod(DelayLine& dL1, int n, float g, float x);

    bool initialized = false;

    float sampleRate = 48000.0f;
    float currentWet = 0.3f;
    float targetWet = 0.3f;
    float smoothCoeff = 0.01f;
    float decay = 0.5f;
    float decayDiff2 = 0.5f;
    float modDepth = 0.0f;
    float maxModDepth = 0.0f;

    int preDelaySamples = 0;
    int maxPreDelay = 0;
    
    DelayLine preDelay;
    OnePoleLP bandwidth;

    //input diffusion
    DelayLine inputAllpass[4];
    int inputAllpassLength[4] = {0, 0, 0, 0};

    //left
    DelayLine leftAllpass1, leftDelay1, leftAllpass2, leftDelay2;
    float leftAllpass1Length = 0.0f;
    int leftDelay1Length = 0, leftAllpass2Length = 0, leftDelay2Length = 0;
    OnePoleLP leftDamp;
    LFO lfoLeft;

    //right
    DelayLine rightAllpass1, rightDelay1, rightAllpass2, rightDelay2;
    float rightAllpass1Length = 0.0f;
    int rightDelay1Length = 0, rightAllpass2Length = 0, rightDelay2Length = 0;
    OnePoleLP rightDamp;
    LFO lfoRight;

    //output taps
    int tapLeft[7] = {0, 0, 0, 0, 0, 0, 0};
    int tapRight[7] = {0, 0, 0, 0, 0, 0, 0};

    //last value in loop, feed into other side
    float lastLeft = 0.0f;
    float lastRight = 0.0f;
};