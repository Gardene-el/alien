#pragma once

#include "EngineInterface/EngineConstants.h"
#include "EngineInterface/Colors.h"


struct TimestepStatistics
{
    double externalEnergy=0.0f;

    ColorVector<int> numCells = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<int> numSelfReplicators = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<int> numViruses = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<int> numConnections = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<int> numParticles = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numGenomeCells = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<float> totalEnergy = {0, 0, 0, 0, 0, 0, 0};
};

struct AccumulatedStatistics
{
    ColorVector<uint64_t> numCreatedCells = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numAttacks = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numMuscleActivities = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numDefenderActivities = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numTransmitterActivities = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numInjectionActivities = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numCompletedInjections = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numNervePulses = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numNeuronActivities = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numSensorActivities = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numSensorMatches = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numReconnectorCreated = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numReconnectorRemoved = {0, 0, 0, 0, 0, 0, 0};
    ColorVector<uint64_t> numDetonations = {0, 0, 0, 0, 0, 0, 0};
};

struct TimelineStatistics
{
    TimestepStatistics timestep;
    AccumulatedStatistics accumulated;
};

struct HistogramData
{
    int maxValue = 0;
    int numCellsByColorBySlot[MAX_COLORS][MAX_HISTOGRAM_SLOTS];
};

struct RawStatisticsData
{
    TimelineStatistics timeline;
    HistogramData histogram;
};
