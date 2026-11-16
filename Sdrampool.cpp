#include "SdramPool.h"
#include "daisy_seed.h"   // for DSY_SDRAM_BSS

namespace
{
    // 4M floats = 16 MB, a quarter of the Seed's SDRAM.
    const size_t kPoolFloats = 4u * 1024u * 1024u;

    // Lives in external SDRAM. Not zeroed at startup, so SdramAlloc() clears what it hands out.
    float DSY_SDRAM_BSS pool[kPoolFloats];

    size_t used = 0;
}

float* SdramAlloc(size_t numFloats)
{
    // Round up to a multiple of 4 floats so every block starts 16-byte aligned
    numFloats = (numFloats + 3u) & ~static_cast<size_t>(3u);

    if (used + numFloats > kPoolFloats)
        return nullptr;

    float* p = pool + used;
    used += numFloats;

    for (size_t i = 0; i < numFloats; i++)
        p[i] = 0.0f;

    return p;
}

size_t SdramFloatsFree()
{
    return kPoolFloats - used;
}