#pragma once

#include "../Base/Base.cuh"
#include "Base/Definitions.cuh"

struct AddConnectionPairOperation {
    bool addTokens;
    Cell* cell;
    Cell* otherCell;
};

struct DelConnectionOperation
{
    Cell* connectedCell;
};

struct DelCellOperation
{
    uint64_t cellIndex;
};

union StructureOperationData
{
    AddConnectionPairOperation addConnection;
    DelConnectionOperation delConnection;
    DelCellOperation delCell;
};

struct StructuralOperation
{
    enum class Type : int
    {
        AddConnectionPair,
        DelConnection,
        DelCell,
    };
    Type type;
    StructureOperationData data;
    int nextOperationIndex; //linked list, = -1 end
};

struct CellFunctionOperation
{
    Cell* cell;
};
