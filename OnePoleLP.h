#pragma once
#include <cmath>

class OnePoleLP
{
    public:
        void Init(float sampleRate)
        {
            sampleRate_ = sampleRate;
            state_ = 0.0f;
            SetCutoff(1000.0f);
        }

        void SetCutoff(float hz) 
        {
            const float maxHz = 0.45 * sampleRate_;
            if (hz > maxHz) hz = maxHz;
            if (hz < 1.0f) hz = 1.0f;

            //algorithm for one pole LPF. values cause a lag behind the actual input, losing high freq content
            coeff_ = 1.0f - std::exp(-2.0f * 3.14159265f * hz / sampleRate_);
         }

         float Process(float in)
         {
            //LPF. multiplies gap between filter and input by the calculated coeff
            state_ += coeff_ * (in - state_);
            return state_;
         }

         void Reset() { state_ = 0.0f; }

    private:
         float sampleRate_ = 48000.0f;
         float coeff_ = 0.1f;
         float state_ = 0.0f;
};