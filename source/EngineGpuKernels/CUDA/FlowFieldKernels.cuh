#pragma once

#include "Base/Math.cuh"
#include "Simulation/Map.cuh"
#include "SimulationData.cuh"

__global__ void cudaApplyFlowFieldSettings(SimulationData data);