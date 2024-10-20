#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/NumberGenerator.h"
#include "EngineInterface/DescriptionEditService.h"
#include "EngineInterface/Descriptions.h"
#include "EngineInterface/GenomeConstants.h"
#include "EngineInterface/GenomeDescriptionService.h"
#include "EngineInterface/SimulationFacade.h"
#include "IntegrationTestFramework.h"

class LivingStateTransitionTests : public IntegrationTestFramework
{
public:
    static SimulationParameters getParameters()
    {
        SimulationParameters result;
        result.innerFriction = 0;
        result.baseValues.friction = 0;
        for (int i = 0; i < MAX_COLORS; ++i) {
            result.baseValues.radiationCellAgeStrength[i] = 0;
        }
        return result;
    }

    LivingStateTransitionTests()
        : IntegrationTestFramework(getParameters())
    {}

    ~LivingStateTransitionTests() = default;

protected:
};

TEST_F(LivingStateTransitionTests, staysReady)
{
    DataDescription data;
    data.addCells({
        CellDescription().setId(1).setPos({10.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Ready),
        CellDescription().setId(2).setPos({11.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Ready),
    });
    data.addConnection(1, 2);

    _simulationFacade->setSimulationData(data);
    _simulationFacade->calcTimesteps(1);
    auto actualData = _simulationFacade->getSimulationData();
    EXPECT_EQ(LivingState_Ready, getCell(actualData, 1).livingState);
    EXPECT_EQ(LivingState_Ready, getCell(actualData, 2).livingState);
}

TEST_F(LivingStateTransitionTests, dyingIfAdjacentDying)
{
    DataDescription data;
    data.addCells({
        CellDescription().setId(1).setPos({10.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Ready),
        CellDescription().setId(2).setPos({11.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Dying),
    });
    data.addConnection(1, 2);

    _simulationFacade->setSimulationData(data);
    _simulationFacade->calcTimesteps(1);
    auto actualData = _simulationFacade->getSimulationData();
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 1).livingState);
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 2).livingState);
}

TEST_F(LivingStateTransitionTests, activatingUnderConstruction)
{
    DataDescription data;
    data.addCells({
        CellDescription().setId(1).setPos({10.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_UnderConstruction),
        CellDescription().setId(2).setPos({11.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Activating),
    });
    data.addConnection(1, 2);

    _simulationFacade->setSimulationData(data);
    _simulationFacade->calcTimesteps(1);
    auto actualData = _simulationFacade->getSimulationData();
    EXPECT_EQ(LivingState_Activating, getCell(actualData, 1).livingState);
    EXPECT_EQ(LivingState_Ready, getCell(actualData, 2).livingState);
}

TEST_F(LivingStateTransitionTests, staysReadyIfAdjacentDying_differentCreatureId)
{
    DataDescription data;
    data.addCells({
        CellDescription().setId(1).setPos({10.0f, 10.0f}).setMaxConnections(1).setCreatureId(1).setLivingState(LivingState_Ready),
        CellDescription().setId(2).setPos({11.0f, 10.0f}).setMaxConnections(1).setCreatureId(2).setLivingState(LivingState_Dying),
    });
    data.addConnection(1, 2);

    _simulationFacade->setSimulationData(data);
    _simulationFacade->calcTimesteps(1);
    auto actualData = _simulationFacade->getSimulationData();
    EXPECT_EQ(LivingState_Ready, getCell(actualData, 1).livingState);
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 2).livingState);
}

TEST_F(LivingStateTransitionTests, noSelfReplicatingConstructorIsDyingIfAdjacentDying)
{
    DataDescription data;
    data.addCells({
        CellDescription().setId(1).setPos({10.0f, 10.0f}).setMaxConnections(1).setCellFunction(ConstructorDescription()).setLivingState(LivingState_Ready),
        CellDescription().setId(2).setPos({11.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Dying),
    });
    data.addConnection(1, 2);

    _simulationFacade->setSimulationData(data);
    _simulationFacade->calcTimesteps(1);
    auto actualData = _simulationFacade->getSimulationData();
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 1).livingState);
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 2).livingState);
}

TEST_F(LivingStateTransitionTests, separatingSelfReplicatorIsDyingIfAdjacentDying)
{
    auto genome = GenomeDescriptionService::get().convertDescriptionToBytes(
        GenomeDescription().setHeader(GenomeHeaderDescription().setSeparateConstruction(true)).setCells({CellGenomeDescription().setCellFunction(ConstructorGenomeDescription().setMakeSelfCopy())}));

    DataDescription data;
    data.addCells({
        CellDescription()
            .setId(1)
            .setPos({10.0f, 10.0f})
            .setMaxConnections(1)
            .setCellFunction(ConstructorDescription().setGenome(genome))
            .setLivingState(LivingState_Ready),
        CellDescription().setId(2).setPos({11.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Dying),
    });
    data.addConnection(1, 2);

    _simulationFacade->setSimulationData(data);
    _simulationFacade->calcTimesteps(1);
    auto actualData = _simulationFacade->getSimulationData();
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 1).livingState);
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 2).livingState);
}

TEST_F(LivingStateTransitionTests, noSeparatingSelfReplicatorStaysReadyIfAdjacentDying)
{
    auto genome = GenomeDescriptionService::get().convertDescriptionToBytes(
        GenomeDescription()
            .setHeader(GenomeHeaderDescription().setSeparateConstruction(false))
            .setCells({CellGenomeDescription().setCellFunction(ConstructorGenomeDescription().setMakeSelfCopy()), CellGenomeDescription()}));

    DataDescription data;
    data.addCells({
        CellDescription()
            .setId(1)
            .setPos({10.0f, 10.0f})
            .setEnergy(_parameters.cellNormalEnergy[0] * 3)
            .setMaxConnections(2)
            .setCellFunction(ConstructorDescription().setGenome(genome))
            .setLivingState(LivingState_Ready),
        CellDescription().setId(2).setPos({11.0f, 10.0f}).setMaxConnections(2).setLivingState(LivingState_Ready),
        CellDescription().setId(3).setPos({12.0f, 10.0f}).setMaxConnections(1).setLivingState(LivingState_Dying),
    });
    data.addConnection(1, 2);
    data.addConnection(2, 3);

    _simulationFacade->setSimulationData(data);
    _simulationFacade->calcTimesteps(1);
    auto actualData = _simulationFacade->getSimulationData();
    auto actualCell1 = getCell(actualData, 1);
    auto actualConstructor = std::get<ConstructorDescription>(*actualCell1.cellFunction);
    EXPECT_EQ(4, actualData.cells.size());
    EXPECT_EQ(1, actualConstructor.genomeCurrentNodeIndex);
    EXPECT_EQ(LivingState_Ready, actualCell1.livingState);
    EXPECT_EQ(LivingState_Dying, getCell(actualData, 2).livingState);
}
