#include "StatisticsParameters.h"
bool StatisticsParameters::operator==(StatisticsParameters const& other) const
{

    return timestepPerStatisticsUpdate == other.timestepPerStatisticsUpdate && enableCustomStatistics == other.enableCustomStatistics;
}