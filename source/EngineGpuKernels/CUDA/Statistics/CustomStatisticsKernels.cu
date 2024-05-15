#include "CustomStatisticsKernels.cuh"


__global__ void cudaUpdateHeatmap_substep1(SimulationData data, SimulationStatistics statistics){
    statistics.addExternalEnergy(*data.externalEnergy);
}
__global__ void cudaUpdateHeatmap_substep2(SimulationData data, SimulationMapStatistics statistics){

    statistics.resetMaps();
    {
        auto& cells = data.objects.cellPointers;
        auto const partition = calcAllThreadsPartition(cells.getNumEntries());

        for (int index = partition.startIndex; index <= partition.endIndex; ++index) {
            auto& cell = cells.at(index);
            statistics.incNumCells(cell->pos);
        }
    }
}
__global__ void cudaUpdateHeatmap_substep3(SimulationData data, SimulationMapStatistics statistics){
}