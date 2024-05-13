#pragma once

#include "Base/Definitions.cuh"
#include "EngineInterface/GpuSettings.h"
#include "EngineInterface/MutationType.h"

class _TestKernelsLauncher
{
public:
    void testOnly_mutate(GpuSettings const& gpuSettings, SimulationData const& data, uint64_t cellId, MutationType mutationType);
};
