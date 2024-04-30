#pragma once

#include "Base/Definitions.h"
#include "Definitions.h"
#include "EngineGpuKernels/TOs.h"
#include "EngineInterface/ArraySizes.h"
#include "EngineInterface/GpuSettings.h"

class _AccessDataTOCache
{
public:
    _AccessDataTOCache();
    ~_AccessDataTOCache();

    DataTO getDataTO(ArraySizes const& arraySizes);

private:
    bool fits(ArraySizes const& left, ArraySizes const& right) const;
    ArraySizes getArraySizes(DataTO const& dataTO) const;
    void deleteDataTO(DataTO const& dataTO);

    std::optional<DataTO> _dataTO;
};
