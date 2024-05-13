#pragma once

#include "Base/Math.cuh"
#include "Base/Map.cuh"
#include "SimulationData.cuh"

__global__ void cudaApplyFlowFieldSettings(SimulationData data);