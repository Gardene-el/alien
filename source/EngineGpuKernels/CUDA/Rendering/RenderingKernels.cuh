#pragma once

#include "EngineInterface/Colors.h"
#include "EngineInterface/ZoomLevels.h"

#include "../TOs.cuh"
#include "../Base/Base.cuh"
#include "../Base/GarbageCollectorKernels.cuh"
#include "../Base/ObjectFactory.cuh"
#include "../Base/Map.cuh"
#include "../SimulationData.cuh"
#include "RenderingData.cuh"

#include <cuda_runtime_api.h>
#include <cuda_runtime.h>

__global__ void cudaDrawBackground(uint64_t* imageData, int2 imageSize, int2 worldSize, float zoom, float2 rectUpperLeft, float2 rectLowerRight);
__global__ void cudaDrawCells(
    uint64_t timestep,
    int2 worldSize,
    float2 rectUpperLeft,
    float2 rectLowerRight,
    Array<Cell*> cells,
    uint64_t* imageData,
    int2 imageSize,
    float zoom);
__global__ void cudaDrawParticles(int2 worldSize, float2 rectUpperLeft, float2 rectLowerRight, Array<Particle*> particles, uint64_t* imageData, int2 imageSize, float zoom);
__global__ void cudaDrawRadiationSources(uint64_t* targetImage, float2 rectUpperLeft, int2 worldSize, int2 imageSize, float zoom);
__global__ void cudaDrawRepetition(int2 worldSize, int2 imageSize, float2 rectUpperLeft, float2 rectLowerRight, uint64_t* imageData, float zoom);
