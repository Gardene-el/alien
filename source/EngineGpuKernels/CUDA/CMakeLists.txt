
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
    Base/Map.cuh
    Base/Math.cuh
    Base/Object.cuh
    Base/Objects.cu
    Base/Objects.cuh
    Base/ObjectFactory.cuh
    Base/Util.cuh
    Base/GarbageCollectorKernels.cu
    Base/GarbageCollectorKernels.cuh
    Base/GarbageCollectorKernelsLauncher.cu
    Base/GarbageCollectorKernelsLauncher.cuh

    ConstantMemory.cu
    ConstantMemory.cuh
    DataAccessKernels.cu
    DataAccessKernels.cuh
    DataAccessKernelsLauncher.cu
    DataAccessKernelsLauncher.cuh
    DebugKernels.cu
    DebugKernels.cuh
    DensityMap.cuh
    EditKernels.cu
    EditKernels.cuh
    EditKernelsLauncher.cu
    EditKernelsLauncher.cuh
    FlowFieldKernels.cu
    FlowFieldKernels.cuh

    GenomeDecoder.cuh
    MapSectionCollector.cuh
    MaxAgeBalancer.cu
    MaxAgeBalancer.cuh
    Operations.cuh
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