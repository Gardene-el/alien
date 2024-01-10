#pragma once
#include "SimulationParameters.h"

class SimulationParametersService
{
public:
    static void activateFeaturesBasedOnParameters(SimulationParameters& parameters);
    static void resetParametersBasedOnFeatures(SimulationParameters& parameters);
};