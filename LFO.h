#pragma once
#include <cmath>

class LFO
{
    public:
        enum Shape { Sine, Triangle, Saw, Square };

        void Init(float sampleRate)
        {

            sampleRate_ = sampleRate;
            phase_ = 0.0f;
            SetRate(1.0f);
        }

        void SetRate(float hz)
        {
            if (hz < 0.0f) hz = 0.0f;
            if (hz > 0.45f * sampleRate_) hz = 0.45f * sampleRate_;
            phaseInc_ = hz / sampleRate_;
        }

        void SetShape(Shape s) { shape_ = s; }
        void SetPhase(float phase01) { phase_ = phase01 - std::floor(phase01); }
        void Reset() { phase_ = 0.0f; }

        //bipolar output, -1..1
        float Process()
    {
        float out;
        switch (shape_)
        {
            case Sine:     out = std::sin(6.28318530718f * phase_);        break;
            case Triangle: out = 1.0f - 4.0f * std::fabs(phase_ - 0.5f);   break;
            case Saw:      out = 2.0f * phase_ - 1.0f;                     break;
            case Square:   out = (phase_ < 0.5f) ? 1.0f : -1.0f;           break;
            default:       out = 0.0f;                                     break;
        }
 
        phase_ += phaseInc_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
 
        return out;
    }
 
    // unipolar output, 0..1
    float ProcessUnipolar()
    {
        return 0.5f * (Process() + 1.0f);
    }
 
private:
    float sampleRate_ = 48000.0f;
    float phase_ = 0.0f;
    float phaseInc_ = 0.0f;
    Shape shape_ = Sine;
};