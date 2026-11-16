#include "reverb1.h"
#include <cassert>
#include <cmath>

namespace
{
    //Dattorro's values at 29761hz sample rate. Scaled to 48k in init func

    const float referenceRate = 29761.0f;

    const int kInputAllpass[4] = {142, 107, 379, 277};
    const float kInputAllpassCoeff[4] = {0.75f, 0.75f, 0.625f, 0.625f};

    const int kLeftAllpass1 = 672;
    const int kLeftDelay1 = 4453;
    const int kLeftAllpass2 = 1800;
    const int kLeftDelay2 = 3720;

    const int kRightAllpass1 = 908;
    const int kRightDelay1 = 4217;
    const int kRightAllpass2 = 2656;
    const int kRightDelay2 = 3163;

    const int kTapLeft[7] = {266, 2974, 1913, 1996, 1990, 187, 1066};
    const int kTapRight[7] = {353, 3627, 1228, 2673, 2111, 335, 121};

    const float kDecayDiffusion1 = 0.7f;
    const float kModDepth = 16.0f;
    const float kOutputScale = 0.6f;

    inline int scaled(int n, float scale)
    {
        return static_cast<int>(n*scale);
    }

    inline float clampf(float v, float lo, float hi)
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }
}

    void Reverb1::Init(float wetMix, float sr)
    {
        if (initialized) return;

        assert(wetMix >= 0.0f && wetMix <= 1.0f);
        assert(sr > 0.0f);

        sampleRate = sr;
        currentWet = wetMix;
        targetWet  = wetMix;

        const float scale = sampleRate/referenceRate;

        maxModDepth = modDepth*scale;
        const int excess = static_cast<int>(std::ceil(maxModDepth))+2;

        maxPreDelay = static_cast<int>(0.1f * sampleRate);
        preDelay.Init(maxPreDelay);

        for (int i = 0; i < 4; i++)
        {
           inputAllpassLength[i] = scaled(kInputAllpass[i], scale);
           inputAllpass[i].Init(inputAllpassLength[i]);
        }

        leftAllpass1Length = static_cast<float>(scaled(kLeftAllpass1, scale));
        leftDelay1Length = scaled(kLeftDelay1, scale);
        leftAllpass2Length = scaled(kLeftAllpass2, scale);
        leftDelay2Length = scaled(kLeftDelay2, scale);
        leftAllpass1.Init(static_cast<int>(leftAllpass1Length) + excess);
        leftDelay1.Init(leftDelay1Length);
        leftAllpass2.Init(leftAllpass2Length);
        leftDelay2.Init(leftDelay2Length);

        rightAllpass1Length = static_cast<float>(scaled(kRightAllpass1, scale));
        rightDelay1Length = scaled(kRightDelay1, scale);
        rightAllpass2Length = scaled(kRightAllpass2, scale);
        rightDelay2Length = scaled(kRightDelay2, scale);
        rightAllpass1.Init(static_cast<int>(rightAllpass1Length) + excess);
        rightDelay1.Init(rightDelay1Length);
        rightAllpass2.Init(rightAllpass2Length);
        rightDelay2.Init(rightDelay2Length);

        for (int i = 0; i < 7; i++)
        {
            tapLeft[i] = scaled(kTapLeft[i], scale);
            tapRight[i] = scaled(kTapRight[i], scale);
        }

        bandwidth.Init(sampleRate);
        leftDamp.Init(sampleRate);
        rightDamp.Init(sampleRate);

        lfoLeft.Init(sampleRate);
        lfoRight.Init(sampleRate);
        lfoLeft.SetRate(1.0f);
        lfoRight.SetRate(0.85f);
        lfoRight.SetPhase(0.5f);

        smoothCoeff = 1.0f - std::exp(-1.0f /  (0.030f*sampleRate));

        SetDecay(0.5f);
        SetDamping(8000.0f);
        SetBandwidth(10000.0f);
        SetPreDelay(0.0f);
        SetModDepth(maxModDepth*0.5);

        initialized = true;
    }

    float Reverb1::Allpass(DelayLine& dL1, int n, float g, float x)
    {
        float delayed = dL1.Read(n);
        float v = x - g*delayed;
        dL1.Write(v);
        return delayed + g*v;
    }

    float Reverb1::AllPassMod(DelayLine& dL1, int n, float g, float x)
    {
        float delayed = dL1.ReadFrac(n);
        float v = x - g*delayed;
        dL1.Write(v);
        return delayed + g*v;
    }

    void Reverb1::ProcessStereo(float input, float* outL, float* outR)
    {
        if (!initialized) 
        {
            *outL = input;
            *outR = input;
            return;
        }

        currentWet += smoothCoeff*(targetWet - currentWet);

        float x = input;

        if (preDelaySamples > 0)
            x = preDelay.Read(preDelaySamples);
        
        preDelay.Write(input);

        x = bandwidth.Process(x);

        for (int i = 0; i < 4; i++)
            x = Allpass(inputAllpass[i], inputAllpassLength[i], kInputAllpassCoeff[i], x);

        const float leftIn = x + decay*lastLeft;
        const float rightIn = x + decay*lastRight;

        float y = AllPassMod(leftAllpass1, leftAllpass1Length + lfoLeft.Process()*modDepth, kDecayDiffusion1, leftIn);
        float d = leftDelay1.Read(leftDelay1Length);
        leftDelay1.Write(y);
        y = leftDamp.Process(d);
        y = Allpass(leftAllpass2, leftAllpass2Length, decayDiff2, y);
        const float endLeft = leftDelay2.Read(leftDelay2Length);
        leftDelay2.Write(y);

        y = AllPassMod(rightAllpass1, rightAllpass1Length + lfoRight.Process()*modDepth, kDecayDiffusion1, rightIn);
        d = rightDelay1.Read(rightDelay1Length);
        rightDelay1.Write(y);
        y = rightDamp.Process(d);
        y = Allpass(rightAllpass2, rightAllpass2Length, decayDiff2, y);
        const float endRight = rightDelay2.Read(rightDelay2Length);
        rightDelay2.Write(y);

        lastLeft = endLeft;
        lastRight = endRight;

        const float wetLeft = kOutputScale*(
            rightDelay1.Read(tapLeft[0])
            + rightDelay1.Read(tapLeft[1])
            - rightAllpass2.Read(tapLeft[2])
            + rightDelay2.Read(tapLeft[3])
            - leftDelay1.Read(tapLeft[4])
            - leftAllpass2.Read(tapLeft[5])
            - leftDelay2.Read(tapLeft[6])
        );

        const float wetRight = kOutputScale*(
            leftDelay1.Read(tapRight[0])
            + leftDelay1.Read(tapRight[1])
            - leftAllpass2.Read(tapRight[2])
            + leftDelay2.Read(tapRight[3])
            - rightDelay1.Read(tapRight[4])
            - rightAllpass2.Read(tapRight[5])
            - rightDelay2.Read(tapRight[6])
        );

        *outL = (1.0f - currentWet)*input + currentWet*wetLeft; 
        *outR = (1.0f - currentWet)*input + currentWet*wetRight; 
    }

    float Reverb1::ProcessSample(float input)
    {
        float l, r;
        ProcessStereo(input, &l, &r);
        return 0.5f*(l + r);
    }

    void Reverb1::Reset()
    {
        if (!initialized) return;

        preDelay.Reset();
        bandwidth.Reset();
        for (int i = 0; i < 4; i++) inputAllpass[i].Reset();

        leftAllpass1.Reset(), leftDelay1.Reset(), leftAllpass2.Reset(), leftDelay2.Reset();
        rightAllpass1.Reset(), rightDelay1.Reset(), rightAllpass2.Reset(), rightDelay2.Reset();
        
        leftDamp.Reset(), rightDamp.Reset();

        lastLeft = 0.0f;
        lastRight = 0.0f;

        currentWet = targetWet;
    }

    void Reverb1::SetDecay(float newDecay)
    {
        decay = clampf(newDecay, 0.0f, 0.98f);
        decayDiff2 = clampf(newDecay + 0.15, 0.25f, 0.5f);
    }

    void Reverb1::SetDamping(float cutoffHz)
    {
        leftDamp.SetCutoff(cutoffHz);
        rightDamp.SetCutoff(cutoffHz);
    }

    void Reverb1::SetBandwidth(float cutoffHz)
    {
        bandwidth.SetCutoff(cutoffHz);
    }

    void Reverb1::SetPreDelay(float ms)
    {
        int n = static_cast<int>(ms*0.001f*sampleRate);
        preDelaySamples = n < 0 ? 0 : (n > maxPreDelay ? maxPreDelay : n);
    }
    
    void Reverb1::SetModDepth(float samples)
    {
        modDepth = clampf(samples, 0.0f, maxModDepth);
    }

    void Reverb1::SetWetMix(float newWetMix)
    {
        targetWet = clampf(newWetMix, 0.0f, 1.0f);
    }