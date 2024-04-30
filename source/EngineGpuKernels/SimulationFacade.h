#pragma once


#ifdef __HIPCC__
/**Null for tempoal*/
#else
#include "CUDA/SimulationCudaFacade.cuh"
typedef _SimulationCudaFacade _SimulationFacade;
#endif