#pragma once

#include "cuda_runtime_api.h"
#include "sm_60_atomic_functions.h"

#include "../SimulationData.cuh"
#include "SimulationStatistics.cuh"
#include "SimulationMapStatistics.cuh"


__global__ void cudaUpdateHeatmap_substep1(SimulationData data, SimulationStatistics statistics);
__global__ void cudaUpdateHeatmap_substep2(SimulationData data, SimulationMapStatistics statistics);
__global__ void cudaUpdateHeatmap_substep3(SimulationData data, SimulationMapStatistics statistics);