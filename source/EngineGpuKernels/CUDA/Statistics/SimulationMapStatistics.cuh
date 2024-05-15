#pragma once

#include "EngineInterface/Statistics/RawStatisticsData.h"


#include "../Base/Base.cuh"
#include "../Base/Definitions.cuh"

class SimulationMapStatistics{

public:
    __host__ void init(int2 const& worldSize, int slotSize)
    {    
         MapStatisticsData mapStatisticsData;
         mapStatisticsData.slotSize=slotSize;
         mapStatisticsData.mapSize={worldSize.x/slotSize,worldSize.y/slotSize};
         mapStatisticsData.cellsMap=nullptr;


        CudaMemoryManager::getInstance().acquireMemory<MapStatisticsData>(1, _data);
        CHECK_FOR_CUDA_ERROR(cudaMemcpy(_data,&mapStatisticsData, sizeof(MapStatisticsData),cudaMemcpyHostToDevice));
        
        int* cellsMap;
        CudaMemoryManager::getInstance().acquireMemory<int>(mapStatisticsData.mapSize.x*mapStatisticsData.mapSize.y, cellsMap);
        CHECK_FOR_CUDA_ERROR(cudaMemcpy(&(_data->cellsMap), &cellsMap, sizeof(int*), cudaMemcpyHostToDevice));
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

    __inline__ __device__ void resetMaps(){
    for (int i = 0; i < _data->mapSize.x * _data->mapSize.y; i++) {
        _data->cellsMap[i] = 0;
    }
    }

    __inline__ __device__ void incNumCells(float2 pos) { 
        int2 targetPos={static_cast<int>(pos.x/_data->slotSize),static_cast<int>(pos.y/_data->slotSize)};
        atomicAdd(&(_data->cellsMap[targetPos.x+targetPos.y*_data->mapSize.y]), 1); }


private:
    MapStatisticsData* _data;
};