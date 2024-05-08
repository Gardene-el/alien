#pragma once

#include "EngineInterface/Statistics/RawStatisticsData.h"


#include "../Base.cuh"
#include "../Definitions.cuh"

class SimulationMapStatistics{

public:
    __host__ void init(int2 const& worldSize, int slotSize)
    {    
        CudaMemoryManager::getInstance().acquireMemory<MapStatisticsData>(1, _data);
        _data->mapSize={worldSize.x/slotSize,worldSize.y/slotSize};
        _data->slotSize=slotSize;

        CudaMemoryManager::getInstance().acquireMemory<int>(_data->mapSize.x * _data->mapSize.y, _data->cellsMap);
    }

    __host__ void free()
    {
        CudaMemoryManager::getInstance().freeMemory(_data->cellsMap);
        CudaMemoryManager::getInstance().freeMemory(_data);
    }

    __host__ MapStatisticsData getStatistics()
    {
        MapStatisticsData result;
        CHECK_FOR_CUDA_ERROR(cudaMemcpy(&result, _data, sizeof(MapStatisticsData), cudaMemcpyDeviceToHost));
        result.cellsMap = new int[result.mapSize.x * result.mapSize.y];
        CHECK_FOR_CUDA_ERROR(cudaMemcpy(&result.cellsMap, _data->cellsMap, sizeof(int)*_data->mapSize.x * _data->mapSize.y, cudaMemcpyDeviceToHost));
        return result;
    }

private:
    MapStatisticsData* _data;
};