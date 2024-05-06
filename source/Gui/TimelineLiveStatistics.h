#pragma once

#include <optional>
#include <vector>

#include "EngineInterface/Colors.h"
#include "EngineInterface/Definitions.h"
#include "EngineInterface/Statistics/DataPointCollection.h"
#include "EngineInterface/Statistics/RawStatisticsData.h"

struct TimelineLiveStatistics
{
    static double constexpr MaxLiveHistory = 240.0;  //in seconds

    double timepoint = 0;   //in seconds
    float history = 10.0f;  //in seconds

    std::vector<DataPointCollection> dataPointCollectionHistory;
    std::optional<TimelineStatistics> lastData;
    std::optional<uint64_t> lastTimestep;

    void truncate();
    void add(TimelineStatistics const& statistics, uint64_t timestep);
};
