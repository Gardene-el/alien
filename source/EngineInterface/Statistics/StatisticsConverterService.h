#include <optional>

#include "EngineInterface/Statistics/DataPointCollection.h"

class StatisticsConverterService
{
public:
    static DataPointCollection convert(
        TimelineStatistics const& newData,
        uint64_t timestep,
        double time,
        std::optional<TimelineStatistics> const& lastData,
        std::optional<uint64_t> lastTimestep);
};
