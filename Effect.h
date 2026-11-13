#pragma once
#include <cstddef>

class Effect
{
public:

    virtual ~Effect() {}
    virtual void Process(float* buf, size_t n) = 0;
    virtual void Reset() = 0;
    virtual int NumParams() const = 0;
    virtual void SetParam(int index, float value01) = 0;
    virtual const char* Name() const = 0;
};