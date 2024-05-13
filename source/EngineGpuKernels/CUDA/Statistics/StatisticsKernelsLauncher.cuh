#pragma once

#include "EngineInterface/GpuSettings.h"

#include "../Base/Base.cuh"
#include "../Base/Definitions.cuh"
#include "../Base/Macros.cuh"

class _StatisticsKernelsLauncher
{
public:
    void updateStatistics(GpuSettings const& gpuSettings, SimulationData const& data, SimulationStatistics const& simulationStatistics);
    void updateCustomStatistics(GpuSettings const& gpuSettings, SimulationData const& data, SimulationStatistics const& simulationStatistics,SimulationMapStatistics const&simulationMapStatistics);
private:
};
