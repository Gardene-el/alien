#pragma once

#include "TOs.cuh"
#include "Base.cuh"
#include "Map.cuh"
#include "ObjectFactory.cuh"
#include "SpotCalculator.cuh"

class CellFunctionProcessor
{
public:
    __inline__ __device__ static void collectCellFunctionOperations(SimulationData& data);
    __inline__ __device__ static void resetFetchedActivities(SimulationData& data);

    __inline__ __device__ static Activity calcInputActivity(Cell* cell);
    __inline__ __device__ static void setActivity(Cell* cell, Activity const& newActivity);

    struct ReferenceAndActualAngle
    {
        float referenceAngle;
        float actualAngle;
    };
    __inline__ __device__ static ReferenceAndActualAngle calcLargestGapReferenceAndActualAngle(SimulationData& data, Cell* cell, float angleDeviation);

    __inline__ __device__ static float2 calcSignalDirection(SimulationData& data, Cell* cell);
};

/************************************************************************/
/* Implementation                                                       */
/************************************************************************/

__inline__ __device__ void CellFunctionProcessor::collectCellFunctionOperations(SimulationData& data)
{
    auto& cells = data.objects.cellPointers;
    auto partition = calcAllThreadsPartition(cells.getNumEntries());

    auto executionOrderNumber = toInt(data.timestep % cudaSimulationParameters.cellNumExecutionOrderNumbers);
    for (int index = partition.startIndex; index <= partition.endIndex; ++index) {
        auto& cell = cells.at(index);
        if (cell->cellFunction != CellFunction_None && cell->executionOrderNumber == executionOrderNumber) {
            if (cell->cellFunction == CellFunction_Detonator && cell->cellFunctionData.detonator.state == DetonatorState_Activated) {
                data.cellFunctionOperations[cell->cellFunction].tryAddEntry(CellFunctionOperation{cell});
            } else if (cell->livingState == LivingState_Ready && cell->activationTime == 0) {
                data.cellFunctionOperations[cell->cellFunction].tryAddEntry(CellFunctionOperation{cell});
            }
        }
    }
}

__inline__ __device__ void CellFunctionProcessor::resetFetchedActivities(SimulationData& data)
{
    auto& cells = data.objects.cellPointers;
    auto partition = calcAllThreadsPartition(cells.getNumEntries());
    auto executionOrderNumber = data.timestep % cudaSimulationParameters.cellNumExecutionOrderNumbers;

    for (int index = partition.startIndex; index <= partition.endIndex; ++index) {
        auto& cell = cells.at(index);
        if (cell->cellFunction == CellFunction_None) {
            continue;
        }
        int maxOtherExecutionOrderNumber = -1;
        if (!cell->outputBlocked) {
            for (int i = 0, j = cell->numConnections; i < j; ++i) {
                auto otherExecutionOrderNumber = cell->connections[i].cell->executionOrderNumber;
                auto otherInputExecutionOrderNumber = cell->connections[i].cell->inputExecutionOrderNumber;
                if (otherInputExecutionOrderNumber == cell->executionOrderNumber) {
                    if (maxOtherExecutionOrderNumber == -1) {
                        maxOtherExecutionOrderNumber = otherExecutionOrderNumber;
                    } else {
                        if ((maxOtherExecutionOrderNumber > cell->executionOrderNumber
                             && (otherExecutionOrderNumber > maxOtherExecutionOrderNumber || otherExecutionOrderNumber < cell->executionOrderNumber))
                            || (maxOtherExecutionOrderNumber < cell->executionOrderNumber && otherExecutionOrderNumber > maxOtherExecutionOrderNumber
                                && otherExecutionOrderNumber < cell->executionOrderNumber)) {
                            maxOtherExecutionOrderNumber = otherExecutionOrderNumber;
                        }
                    }
                }
            }
        }
        if ((maxOtherExecutionOrderNumber == -1
             && executionOrderNumber == (cell->executionOrderNumber + 1) % cudaSimulationParameters.cellNumExecutionOrderNumbers)
            || (maxOtherExecutionOrderNumber != -1 && maxOtherExecutionOrderNumber == executionOrderNumber)) {
            for (int i = 0; i < MAX_CHANNELS; ++i) {
                cell->activity.channels[i] = 0;
            }
        }
    }
}

__inline__ __device__ Activity CellFunctionProcessor::calcInputActivity(Cell* cell)
{
    Activity result;

    for (int i = 0; i < MAX_CHANNELS; ++i) {
        result.channels[i] = 0;
    }

    if (cell->inputExecutionOrderNumber == -1 || cell->inputExecutionOrderNumber == cell->executionOrderNumber) {
        return result;
    }

    for (int i = 0, j = cell->numConnections; i < j; ++i) {
        auto connectedCell = cell->connections[i].cell;
        if (connectedCell->outputBlocked || connectedCell->livingState != LivingState_Ready ) {
            continue;
        }
        if (connectedCell->executionOrderNumber == cell->inputExecutionOrderNumber) {
            for (int i = 0; i < MAX_CHANNELS; ++i) {
                result.channels[i] += connectedCell->activity.channels[i];
                result.channels[i] = max(-1000000.0f, min(1000000.0f, result.channels[i])); //truncate value to avoid overflow
            }
        }
    }
    return result;
}

__inline__ __device__ void CellFunctionProcessor::setActivity(Cell* cell, Activity const& newActivity)
{
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        cell->activity.channels[i] = newActivity.channels[i];
    }
}

__inline__ __device__ CellFunctionProcessor::ReferenceAndActualAngle
CellFunctionProcessor::calcLargestGapReferenceAndActualAngle(SimulationData& data, Cell* cell, float angleDeviation)
{
    if (0 == cell->numConnections) {
        return ReferenceAndActualAngle{0, data.numberGen1.random()*360};
    }
    auto displacement = cell->connections[0].cell->pos - cell->pos;
    data.cellMap.correctDirection(displacement);
    auto angle = Math::angleOfVector(displacement);
    int index = 0;
    float largestAngleGap = 0;
    float angleOfLargestAngleGap = 0;
    auto numConnections = cell->numConnections;
    for (int i = 1; i <= numConnections; ++i) {
        auto angleDiff = cell->connections[i % numConnections].angleFromPrevious;
        if (angleDiff > largestAngleGap) {
            largestAngleGap = angleDiff;
            index = i % numConnections;
            angleOfLargestAngleGap = angle;
        }
        angle += angleDiff;
    }

    auto angleFromPrev = cell->connections[index].angleFromPrevious;
    for (int i = 0; i < numConnections - 1; ++i) {
        if (angleDeviation > angleFromPrev / 2) {
            angleDeviation -= angleFromPrev / 2;
            index = (index + 1) % numConnections;
            angleOfLargestAngleGap += angleFromPrev; 
            angleFromPrev = cell->connections[index].angleFromPrevious;
            angleDeviation = angleDeviation - angleFromPrev / 2;
        }
        if (angleDeviation < -angleFromPrev / 2) {
            angleDeviation += angleFromPrev / 2;
            index = (index + numConnections - 1) % numConnections;
            angleFromPrev = cell->connections[index].angleFromPrevious;
            angleDeviation = angleDeviation + angleFromPrev / 2;
            angleOfLargestAngleGap -= angleFromPrev;
        }
    }
    auto angleFromPreviousConnection = angleFromPrev / 2 + angleDeviation;

    if (angleFromPreviousConnection > 360.0f) {
        angleFromPreviousConnection -= 360;
    }
    angleFromPreviousConnection = max(30.0f, min(angleFromPrev - 30.0f, angleFromPreviousConnection));

    return ReferenceAndActualAngle{angleFromPreviousConnection, angleOfLargestAngleGap + angleFromPreviousConnection};
}

__inline__ __device__ float2 CellFunctionProcessor::calcSignalDirection(SimulationData& data, Cell* cell)
{
    float2 result{0, 0};
    for (int i = 0; i < cell->numConnections; ++i) {
        auto& connectedCell = cell->connections[i].cell;
        if (connectedCell->executionOrderNumber == cell->inputExecutionOrderNumber && !connectedCell->outputBlocked) {
            auto directionDelta = cell->pos - connectedCell->pos;
            data.cellMap.correctDirection(directionDelta);
            result = result + Math::normalized(directionDelta);
        }
    }
    return Math::normalized(result);
}
