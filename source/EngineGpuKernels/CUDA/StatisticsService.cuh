#include <optional>

#include "EngineInterface/StatisticsHistory.h"
#include "Definitions.cuh"

class _StatisticsService
{
public:
    void addDataPoint(StatisticsHistory& history, TimelineStatistics const& newRawStatistics, uint64_t timestep);
    void resetTime(StatisticsHistory& history, uint64_t timestep);
    void rewriteHistory(StatisticsHistory& history, StatisticsHistoryData const& newHistoryData, uint64_t timestep);

private:
    static auto constexpr DefaultTimeStepDelta = 10.0;

    double _longtermTimestepDelta = DefaultTimeStepDelta;

    std::optional<TimelineStatistics> _lastRawStatistics;
    std::optional<uint64_t> _lastTimestep;
};
