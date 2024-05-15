
add_library(EngineGpuKernels
    ../Definitions.h
    ../TOs.h

    Base/Array.cuh
    Base/Base.cuh
    Base/CudaMemoryManager.cuh
    Base/CudaNumberGenerator.cuh
    Base/CudaShapeGenerator.cuh
    Base/Definitions.cuh
    Base/HashMap.cuh
    Base/HashSet.cuh
    Base/List.cuh
    Base/Macros.cuh
    Base/Math.cuh
    Base/Util.cuh

    Simulation/Object.cuh
    Simulation/Objects.cu
    Simulation/Objects.cuh
    Simulation/ObjectFactory.cuh
    Simulation/Map.cuh

    Editor/DataAccessKernels.cu
    Editor/DataAccessKernels.cuh
    Editor/DataAccessKernelsLauncher.cu
    Editor/DataAccessKernelsLauncher.cuh
    Editor/EditKernels.cu
    Editor/EditKernels.cuh
    Editor/EditKernelsLauncher.cu
    Editor/EditKernelsLauncher.cuh
    Editor/GarbageCollectorKernels.cu
    Editor/GarbageCollectorKernels.cuh
    Editor/GarbageCollectorKernelsLauncher.cu
    Editor/GarbageCollectorKernelsLauncher.cuh
    Editor/Operations.cuh

    ConstantMemory.cu
    ConstantMemory.cuh

    DebugKernels.cu
    DebugKernels.cuh
    DensityMap.cuh

    FlowFieldKernels.cu
    FlowFieldKernels.cuh

    GenomeDecoder.cuh
    MapSectionCollector.cuh
    MaxAgeBalancer.cu
    MaxAgeBalancer.cuh
    Physics.cuh
    PreprocessedCellFunctionData.cuh
    SelectionResult.cuh
    SimulationCudaFacade.cu
    SimulationCudaFacade.cuh
    SimulationData.cu
    SimulationData.cuh
    SimulationKernels.cu
    SimulationKernels.cuh
    SimulationKernelsLauncher.cu
    SimulationKernelsLauncher.cuh
    SpotCalculator.cuh
    TestKernels.cu
    TestKernels.cuh
    TestKernelsLauncher.cu
    TestKernelsLauncher.cuh

    Processors/AttackerProcessor.cuh
    Processors/CellComputationProcessor.cuh
    Processors/CellConnectionProcessor.cuh
    Processors/CellFunctionProcessor.cuh
    Processors/CellProcessor.cuh
    Processors/ClusterProcessor.cuh
    Processors/ConstructorProcessor.cuh
    Processors/DetonatorProcessor.cuh
    Processors/InjectorProcessor.cuh
    Processors/MuscleProcessor.cuh
    Processors/MutationProcessor.cuh
    Processors/NerveProcessor.cuh
    Processors/NeuronProcessor.cuh
    Processors/ParticleProcessor.cuh
    Processors/ReconnectorProcessor.cuh
    Processors/SensorProcessor.cuh
    Processors/TransmitterProcessor.cuh

    Statistics/SimulationStatistics.cuh
    Statistics/StatisticsService.cu
    Statistics/StatisticsService.cuh
    Statistics/StatisticsKernelsLauncher.cu
    Statistics/StatisticsKernelsLauncher.cuh
    Statistics/StatisticsKernels.cu
    Statistics/StatisticsKernels.cuh
    Statistics/SimulationMapStatistics.cuh
    Statistics/StatisticsDataCache.cu
    Statistics/StatisticsDataCache.cuh
    Statistics/CustomStatisticsKernels.cu
    Statistics/CustomStatisticsKernels.cuh

    Rendering/RenderingData.cu
    Rendering/RenderingData.cuh
    Rendering/RenderingKernels.cu
    Rendering/RenderingKernels.cuh
    Rendering/RenderingKernelsLauncher.cu
    Rendering/RenderingKernelsLauncher.cuh

    )

target_link_libraries(EngineGpuKernels Base)
target_link_libraries(EngineGpuKernels EngineInterface)

# See https://gitlab.kitware.com/cmake/cmake/-/issues/17520
set_property(TARGET EngineGpuKernels PROPERTY CUDA_RESOLVE_DEVICE_SYMBOLS ON)