#pragma once
#include <cstddef>

// Simple bump allocator over a block of external SDRAM (the Seed has 64 MB).
//
// IMPORTANT: SDRAM only works after hw.Init() has run, so only call SdramAlloc()
// from code that runs after that (for example an Init() method called from main()),
// never from a constructor of a global object.

// Returns `numFloats` floats of zero-filled SDRAM, or nullptr if the pool is full.
float* SdramAlloc(size_t numFloats);

// How many floats are still available in the pool.
size_t SdramFloatsFree();