
add_library(EngineInterface

    Statistics/DataPointCollection.cpp
    Statistics/DataPointCollection.h
    Statistics/RawStatisticsData.h
    Statistics/StatisticsConverterService.cpp
    Statistics/StatisticsConverterService.h
    Statistics/StatisticsHistory.cpp
    Statistics/StatisticsHistory.h

    ArraySizes.h
    AuxiliaryData.h
    AuxiliaryDataParserService.cpp
    AuxiliaryDataParserService.h
    CellFunctionConstants.h
    Colors.h


    Definitions.h
    DescriptionEditService.cpp
    DescriptionEditService.h
    Descriptions.cpp
    Descriptions.h
    EngineConstants.h
    Features.cpp
    Features.h
    GenomeConstants.h
    GenomeDescriptionService.cpp
    GenomeDescriptionService.h
    GenomeDescriptions.h
    GeneralSettings.h
    GpuSettings.h
    InspectedEntityIds.h
    Motion.h
    MutationType.h
    OverlayDescriptions.h
    PreviewDescriptionService.cpp
    PreviewDescriptionService.h
    PreviewDescriptions.h
    RadiationSource.h
    SelectionShallowData.h
    SerializerService.cpp
    SerializerService.h
    Settings.h
    ShallowUpdateSelectionData.h
    ShapeGenerator.cpp
    ShapeGenerator.h
    SimulationController.h
    SimulationParameters.cpp
    SimulationParameters.h
    SimulationParametersService.cpp
    SimulationParametersService.h
    SimulationParametersSpot.h
    SimulationParametersSpotActivatedValues.h
    SimulationParametersSpotValues.h
    SpaceCalculator.cpp
    SpaceCalculator.h

    ZoomLevels.h)

target_link_libraries(EngineInterface Boost::boost)
target_link_libraries(EngineInterface cereal)
target_link_libraries(alien ZLIB::ZLIB)

find_path(ZSTR_INCLUDE_DIRS "zstr.hpp")
target_include_directories(EngineInterface PRIVATE ${ZSTR_INCLUDE_DIRS})

if (MSVC)
    target_compile_options(EngineInterface PRIVATE "/MP")
endif()
