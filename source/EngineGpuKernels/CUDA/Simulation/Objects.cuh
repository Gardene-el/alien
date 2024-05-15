#pragma once

#include "EngineInterface/GpuSettings.h"

#include "../Base/Base.cuh"
#include "../Base/Definitions.cuh"
#include "../Base/Array.cuh"

struct Objects
{
    Array<Cell*> cellPointers;
    Array<Particle*> particlePointers;

    Array<Cell> cells;
    Array<Particle> particles;

    RawMemory auxiliaryData;

    void init();
    void free();

    __device__ void saveNumEntries();
};

