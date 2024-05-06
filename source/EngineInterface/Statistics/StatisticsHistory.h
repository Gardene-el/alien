#pragma once

#include <mutex>
#include <vector>

#include "EngineInterface/Definitions.h"
#include "EngineInterface/Statistics/DataPointCollection.h"

using StatisticsHistoryData = std::vector<DataPointCollection>;

class StatisticsHistory
{
public:
    StatisticsHistoryData getCopiedData() const;

    std::mutex& getMutex() const;
    StatisticsHistoryData& getDataRef();
    StatisticsHistoryData const& getDataRef() const;

private:
    mutable std::mutex _mutex;
    StatisticsHistoryData _data;
};
