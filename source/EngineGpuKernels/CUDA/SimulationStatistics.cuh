#pragma once

#include "EngineInterface/RawStatisticsData.h"

#include "Base.cuh"
#include "Definitions.cuh"

class SimulationStatistics
{
public:

    __host__ void init()
    {
        CudaMemoryManager::getInstance().acquireMemory<RawStatisticsData>(1, _data);
        CHECK_FOR_CUDA_ERROR(cudaMemset(_data, 0, sizeof(RawStatisticsData)));
    }

    __host__ void free()
    {
        CudaMemoryManager::getInstance().freeMemory(_data);
    }

    __host__ RawStatisticsData getStatistics()
    {
        RawStatisticsData result;
        CHECK_FOR_CUDA_ERROR(cudaMemcpy(&result, _data, sizeof(RawStatisticsData), cudaMemcpyDeviceToHost));
        return result;
    }

    //timestep statistics
    __inline__ __device__ void resetTimestepData()
    {
        _data->timeline.timestep = TimestepStatistics();
    }

    __inline__ __device__ void incNumCells(int color) { atomicAdd(&(_data->timeline.timestep.numCells[color]), 1); }
    __inline__ __device__ void incNumReplicator(int color) { atomicAdd(&_data->timeline.timestep.numSelfReplicators[color], 1); }
    __inline__ __device__ void incNumViruses(int color) { atomicAdd(&_data->timeline.timestep.numViruses[color], 1); }
    __inline__ __device__ void incNumConnections(int color, int numConnections) { atomicAdd(&_data->timeline.timestep.numConnections[color], numConnections); }
    __inline__ __device__ void incNumParticles(int color) { atomicAdd(&_data->timeline.timestep.numParticles[color], 1); }
    __inline__ __device__ void addEnergy(int color, float valueToAdd) { atomicAdd(&_data->timeline.timestep.totalEnergy[color], valueToAdd); }
    __inline__ __device__ void addNumGenomeNodes(int color, uint64_t valueToAdd)
    {
        alienAtomicAdd64(&_data->timeline.timestep.numGenomeCells[color], valueToAdd);
    }
    __inline__ __device__ void halveNumConnections()
    {
        for (int i = 0; i < MAX_COLORS; ++i) {
            _data->timeline.timestep.numConnections[i] /= 2;
        }
    }

    //accumulated statistics
    __host__ void resetAccumulatedStatistics()
    {
        RawStatisticsData hostData;
        CHECK_FOR_CUDA_ERROR(cudaMemcpy(&hostData, _data, sizeof(RawStatisticsData), cudaMemcpyDeviceToHost));
        hostData.timeline.accumulated = AccumulatedStatistics();
        CHECK_FOR_CUDA_ERROR(cudaMemcpy(_data, &hostData, sizeof(RawStatisticsData), cudaMemcpyHostToDevice));
    }

    __inline__ __device__ void incNumCreatedCells(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numCreatedCells[color], uint64_t(1)); }
    __inline__ __device__ void incNumAttacks(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numAttacks[color], uint64_t(1)); }
    __inline__ __device__ void incNumMuscleActivities(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numMuscleActivities[color], uint64_t(1)); }
    __inline__ __device__ void incNumDefenderActivities(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numDefenderActivities[color], uint64_t(1)); }
    __inline__ __device__ void incNumTransmitterActivities(int color)
    {
        alienAtomicAdd64(&_data->timeline.accumulated.numTransmitterActivities[color], uint64_t(1));
    }
    __inline__ __device__ void incNumInjectionActivities(int color)
    {
        alienAtomicAdd64(&_data->timeline.accumulated.numInjectionActivities[color], uint64_t(1));
    }
    __inline__ __device__ void incNumCompletedInjections(int color)
    {
        alienAtomicAdd64(&_data->timeline.accumulated.numCompletedInjections[color], uint64_t(1));
    }
    __inline__ __device__ void incNumNervePulses(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numNervePulses[color], uint64_t(1)); }
    __inline__ __device__ void incNumNeuronActivities(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numNeuronActivities[color], uint64_t(1)); }
    __inline__ __device__ void incNumSensorActivities(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numSensorActivities[color], uint64_t(1)); }
    __inline__ __device__ void incNumSensorMatches(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numSensorMatches[color], uint64_t(1)); }
    __inline__ __device__ void incNumReconnectorCreated(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numReconnectorCreated[color], uint64_t(1)); }
    __inline__ __device__ void incNumReconnectorRemoved(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numReconnectorRemoved[color], uint64_t(1)); }
    __inline__ __device__ void incNumDetonations(int color) { alienAtomicAdd64(&_data->timeline.accumulated.numDetonations[color], uint64_t(1)); }

    //histogram
    __inline__ __device__ void resetHistogramData()
    {
        _data->histogram.maxValue = 0;
        for (int i = 0; i < MAX_COLORS; ++i) {
            for (int j = 0; j < MAX_HISTOGRAM_SLOTS; ++j) {
                _data->histogram.numCellsByColorBySlot[i][j] = 0;
            }
        }
    }
    __inline__ __device__ void incNumCells(int color, int slot) { atomicAdd(&(_data->histogram.numCellsByColorBySlot[color][slot]), 1); }
    __inline__ __device__ void maxValue(int value) { atomicMax(&_data->histogram.maxValue, value); }
    __inline__ __device__ int getMaxValue() const { return _data->histogram.maxValue; }

private:
    RawStatisticsData* _data;
};

