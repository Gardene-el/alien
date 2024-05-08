#pragma once
#include <chrono>

struct StatisticsParameters
{
    bool enableCustomStatistics = true;
    int timestepPerStatisticsUpdate = 30;  // = 30;
    bool operator==(StatisticsParameters const& other) const;
    bool operator!=(StatisticsParameters const& other) const { return !operator==(other); }
};