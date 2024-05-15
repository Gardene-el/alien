﻿#pragma once

#include "EngineInterface/GpuSettings.h"

#include "../Base/Definitions.cuh"
#include "../Base/Macros.cuh"
#include "../Base/Base.cuh"
#include "GarbageCollectorKernels.cuh"

class _GarbageCollectorKernelsLauncher
{
public:
    _GarbageCollectorKernelsLauncher();
    ~_GarbageCollectorKernelsLauncher();

    void cleanupAfterTimestep(GpuSettings const& gpuSettings, SimulationData const& simulationData);
    void cleanupAfterDataManipulation(GpuSettings const& gpuSettings, SimulationData const& simulationData);
    void copyArrays(GpuSettings const& gpuSettings, SimulationData const& simulationData);
    void swapArrays(GpuSettings const& gpuSettings, SimulationData const& simulationData);

private:
    //gpu memory
    bool* _cudaBool;
};
