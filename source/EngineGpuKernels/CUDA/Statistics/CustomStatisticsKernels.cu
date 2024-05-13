#include "CustomStatisticsKernels.cuh"


__global__ void cudaUpdateHeatmap_substep1(SimulationData data, SimulationStatistics statistics){
    statistics.addExternalEnergy(*data.externalEnergy);
}
__global__ void cudaUpdateHeatmap_substep2(SimulationData data, SimulationMapStatistics statistics){
}
