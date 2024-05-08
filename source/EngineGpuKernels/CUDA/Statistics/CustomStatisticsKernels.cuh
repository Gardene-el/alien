#pragma once

#include "cuda_runtime_api.h"
#include "sm_60_atomic_functions.h"

#include "../SimulationData.cuh"
#include "SimulationStatistics.cuh"

__global__ void cudaUpdateHeatmap_substep1(SimulationData data, SimulationStatistics statistics);
__global__ void cudaUpdateHeatmap_substep2(SimulationData data, SimulationStatistics statistics);
