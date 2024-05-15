#pragma once

#include "EngineInterface/GpuSettings.h"
#include "EngineInterface/ShallowUpdateSelectionData.h"
#include "EngineInterface/Settings.h"

#include "../Base/Base.cuh"
#include "../Base/Definitions.cuh"
#include "../Base/Macros.cuh"

#include "../DataAccessKernels.cuh"
#include "../GarbageCollectorKernelsLauncher.cuh"

class _RenderingKernelsLauncher
{
public:
    void drawImage(
        Settings const& settings,
        float2 rectUpperLeft,
        float2 rectLowerRight,
        int2 imageSize,
        float zoom,
        SimulationData data,
        RenderingData renderingData);
};
