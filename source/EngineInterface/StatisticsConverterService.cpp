#include "StatisticsConverterService.h"

#include "Base/Definitions.h"

namespace
{
    DataPoint getDataPointForExternalEnergy(double const& value)
    {
        DataPoint result;
        for (int i = 0; i < MAX_COLORS; ++i) {
            result.values[i] = value;
        }
        return result;
    }

    template <typename T>
    DataPoint getDataPointForTimestepProperty(ColorVector<T> const& values)
    {
        DataPoint result;
        result.summedValues = 0;
        for (int i = 0; i < MAX_COLORS; ++i) {
            result.values[i] = toDouble(values[i]);
            result.summedValues += result.values[i];
        }
        return result;
    }

    DataPoint getDataPointForAverageGenomeNodes(ColorVector<uint64_t> const& numGenomeNodes, ColorVector<int> const& numSelfReplicators)
    {
        DataPoint result;
        auto sumNumGenomeNodes = 0.0;
        auto sumNumSelfReplicators = 0.0;
        for (int i = 0; i < MAX_COLORS; ++i) {
            result.values[i] = toDouble(numGenomeNodes[i]);
            sumNumGenomeNodes += result.values[i];
            sumNumSelfReplicators += numSelfReplicators[i];
            if (numSelfReplicators[i] > 0) {
                result.values[i] /= numSelfReplicators[i];
            }
        }
        result.summedValues = sumNumSelfReplicators > 0 ? sumNumGenomeNodes / sumNumSelfReplicators : sumNumGenomeNodes;
        return result;
    }

    DataPoint getDataPointForProcessProperty(
        ColorVector<uint64_t> const& values,
        ColorVector<uint64_t> const& lastValues,
        ColorVector<int> const& numCells,
        double deltaTimesteps)
    {
        DataPoint result;
        result.summedValues = 0;
        auto sumNumCells = 0;
        for (int i = 0; i < MAX_COLORS; ++i) {
            if (lastValues[i] > values[i] || numCells[i] == 0) {
                result.values[i] = 0;
            } else {
                result.values[i] = toDouble(values[i] - lastValues[i]) / deltaTimesteps / toDouble(numCells[i]);
                result.summedValues += toDouble(values[i] - lastValues[i]) / deltaTimesteps;
                sumNumCells += numCells[i];
            }
        }
        result.summedValues /= toDouble(sumNumCells);
        return result;
    }


}

DataPointCollection StatisticsConverterService::convert(
    TimelineStatistics const& data,
    uint64_t timestep,
    double time,
    std::optional<TimelineStatistics> const& lastData,
    std::optional<uint64_t> lastTimestep)
{
    DataPointCollection result;
    result.time = time;
    result.numCells = getDataPointForTimestepProperty(data.timestep.numCells);
    result.numSelfReplicators = getDataPointForTimestepProperty(data.timestep.numSelfReplicators);
    result.numViruses = getDataPointForTimestepProperty(data.timestep.numViruses);
    result.numConnections = getDataPointForTimestepProperty(data.timestep.numConnections);
    result.numParticles = getDataPointForTimestepProperty(data.timestep.numParticles);
    result.averageGenomeCells = getDataPointForAverageGenomeNodes(data.timestep.numGenomeCells, data.timestep.numSelfReplicators);
    result.totalEnergy = getDataPointForTimestepProperty(data.timestep.totalEnergy);
    result.externalEnergy = getDataPointForExternalEnergy(data.timestep.externalEnergy);


    auto deltaTimesteps = lastTimestep ? toDouble(timestep) - toDouble(*lastTimestep) : 1.0;
    if (deltaTimesteps < NEAR_ZERO) {
        deltaTimesteps = 1.0;
    }

    auto lastDataValue = lastData.value_or(data);
    result.numCreatedCells =
        getDataPointForProcessProperty(data.accumulated.numCreatedCells, lastDataValue.accumulated.numCreatedCells, data.timestep.numCells, deltaTimesteps);
    result.numAttacks =
        getDataPointForProcessProperty(data.accumulated.numAttacks, lastDataValue.accumulated.numAttacks, data.timestep.numCells, deltaTimesteps);
    result.numMuscleActivities = getDataPointForProcessProperty(
        data.accumulated.numMuscleActivities, lastDataValue.accumulated.numMuscleActivities, data.timestep.numCells, deltaTimesteps);
    result.numDefenderActivities = getDataPointForProcessProperty(
        data.accumulated.numDefenderActivities, lastDataValue.accumulated.numDefenderActivities, data.timestep.numCells, deltaTimesteps);
    result.numTransmitterActivities = getDataPointForProcessProperty(
        data.accumulated.numTransmitterActivities, lastDataValue.accumulated.numTransmitterActivities, data.timestep.numCells, deltaTimesteps);
    result.numInjectionActivities = getDataPointForProcessProperty(
        data.accumulated.numInjectionActivities, lastDataValue.accumulated.numInjectionActivities, data.timestep.numCells, deltaTimesteps);
    result.numCompletedInjections = getDataPointForProcessProperty(
        data.accumulated.numCompletedInjections, lastDataValue.accumulated.numCompletedInjections, data.timestep.numCells, deltaTimesteps);
    result.numNervePulses =
        getDataPointForProcessProperty(data.accumulated.numNervePulses, lastDataValue.accumulated.numNervePulses, data.timestep.numCells, deltaTimesteps);
    result.numNeuronActivities = getDataPointForProcessProperty(
        data.accumulated.numNeuronActivities, lastDataValue.accumulated.numNeuronActivities, data.timestep.numCells, deltaTimesteps);
    result.numSensorActivities = getDataPointForProcessProperty(
        data.accumulated.numSensorActivities, lastDataValue.accumulated.numSensorActivities, data.timestep.numCells, deltaTimesteps);
    result.numSensorMatches =
        getDataPointForProcessProperty(data.accumulated.numSensorMatches, lastDataValue.accumulated.numSensorMatches, data.timestep.numCells, deltaTimesteps);
    result.numReconnectorCreated = getDataPointForProcessProperty(
        data.accumulated.numReconnectorCreated, lastDataValue.accumulated.numReconnectorCreated, data.timestep.numCells, deltaTimesteps);
    result.numReconnectorRemoved = getDataPointForProcessProperty(
        data.accumulated.numReconnectorRemoved, lastDataValue.accumulated.numReconnectorRemoved, data.timestep.numCells, deltaTimesteps);
    result.numDetonations =
        getDataPointForProcessProperty(data.accumulated.numDetonations, lastDataValue.accumulated.numDetonations, data.timestep.numCells, deltaTimesteps);

    return result;
}
