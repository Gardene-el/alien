#include "SimulationParametersZoneWidgets.h"

#include "EngineInterface/SimulationFacade.h"
#include "EngineInterface/SimulationParametersValidationService.h"
#include "EngineInterface/SimulationParametersZone.h"

#include "AlienImGui.h"
#include "LocationHelper.h"
#include "LoginDialog.h"
#include "SimulationInteractionController.h"

namespace
{
    auto constexpr RightColumnWidth = 285.0f;

    template <int numRows, int numCols, typename T>
    std::vector<std::vector<T>> toVector(T const v[numRows][numCols])
    {
        std::vector<std::vector<T>> result;
        for (int row = 0; row < numRows; ++row) {
            std::vector<T> rowVector;
            for (int col = 0; col < numCols; ++col) {
                rowVector.emplace_back(v[row][col]);
            }
            result.emplace_back(rowVector);
        }
        return result;
    }
}

void _SimulationParametersZoneWidgets::init(SimulationFacade const& simulationFacade, int locationIndex)
{
    _simulationFacade = simulationFacade;
    _locationIndex = locationIndex;
}

void _SimulationParametersZoneWidgets::process()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();

    auto zoneIndex = LocationHelper::findLocationArrayIndex(parameters, _locationIndex);
    SimulationParametersZone& zone = parameters.zone[zoneIndex];
    SimulationParametersZone const& origZone = origParameters.zone[zoneIndex];
    auto lastZone = zone;
    _zoneName = std::string(zone.name);

    auto worldSize = _simulationFacade->getWorldSize();

    /**
     * General
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("常规"))) {
        AlienImGui::InputText(
            AlienImGui::InputTextParameters().name("名称").textWidth(RightColumnWidth).defaultValue(origZone.name), zone.name, sizeof(Char64) / sizeof(char));

    }
    AlienImGui::EndTreeNode();

    /**
     * Visualization and location
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("可视化"))) {
        AlienImGui::ColorButtonWithPicker(
            AlienImGui::ColorButtonWithPickerParameters().name("背景颜色").textWidth(RightColumnWidth).defaultValue(origZone.color),
            zone.color,
            _backupColor,
            _zoneColorPalette.getReference());
    }
    AlienImGui::EndTreeNode();

    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("位置"))) {
        if (AlienImGui::Switcher(
                AlienImGui::SwitcherParameters().name("形状").values({"圆形", "矩形"}).textWidth(RightColumnWidth).defaultValue(origZone.shapeType),
                zone.shapeType)) {
            setDefaultSpotData(zone);
        }

        auto getMousePickerEnabledFunc = [&]() { return SimulationInteractionController::get().isPositionSelectionMode(); };
        auto setMousePickerEnabledFunc = [&](bool value) { SimulationInteractionController::get().setPositionSelectionMode(value); };
        auto getMousePickerPositionFunc = [&]() { return SimulationInteractionController::get().getPositionSelectionData(); };

        AlienImGui::SliderFloat2(
            AlienImGui::SliderFloat2Parameters()
                .name("位置 (x,y)")
                .textWidth(RightColumnWidth)
                .min({0, 0})
                .max(toRealVector2D(worldSize))
                .defaultValue(RealVector2D{origZone.posX, origZone.posY})
                .format("%.2f")
                .getMousePickerEnabledFunc(getMousePickerEnabledFunc)
                .setMousePickerEnabledFunc(setMousePickerEnabledFunc)
                .getMousePickerPositionFunc(getMousePickerPositionFunc),
            zone.posX,
            zone.posY);
        AlienImGui::SliderFloat2(
            AlienImGui::SliderFloat2Parameters()
                .name("速度 (x,y)")
                .textWidth(RightColumnWidth)
                .min({-4.0f, -4.0f})
                .max({4.0f, 4.0f})
                .defaultValue(RealVector2D{origZone.velX, origZone.velY})
                .format("%.2f"),
            zone.velX,
            zone.velY);
        auto maxRadius = toFloat(std::max(worldSize.x, worldSize.y));
        if (zone.shapeType == SpotShapeType_Circular) {
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("核心半径")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(maxRadius)
                    .defaultValue(&origZone.shapeData.circularSpot.coreRadius)
                    .format("%.1f"),
                &zone.shapeData.circularSpot.coreRadius);
        }
        if (zone.shapeType == SpotShapeType_Rectangular) {
            AlienImGui::SliderFloat2(
                AlienImGui::SliderFloat2Parameters()
                    .name("尺寸 (x,y)")
                    .textWidth(RightColumnWidth)
                    .min({0, 0})
                    .max({toFloat(worldSize.x), toFloat(worldSize.y)})
                    .defaultValue(RealVector2D{origZone.shapeData.rectangularSpot.width, origZone.shapeData.rectangularSpot.height})
                    .format("%.1f"),
                zone.shapeData.rectangularSpot.width,
                zone.shapeData.rectangularSpot.height);
        }

        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("淡出半径")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(maxRadius)
                .defaultValue(&origZone.fadeoutRadius)
                .format("%.1f"),
            &zone.fadeoutRadius);
    }
    AlienImGui::EndTreeNode();

    /**
     * Force field
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("力场"))) {
        {
            auto forceFieldTypeIntern = std::max(0, zone.flowType - 1);  //FlowType_None should not be selectable in ComboBox
            auto origForceFieldTypeIntern = std::max(0, origZone.flowType - 1);

            auto isEnabled = zone.flowType != 0;
            auto origIsEnabled = origZone.flowType != 0;
            if (AlienImGui::Combo(
                    AlienImGui::ComboParameters()
                        .name("场类型")
                        .values({"径向", "中心", "线性"})
                        .textWidth(RightColumnWidth)
                        .defaultValue(origForceFieldTypeIntern)
                        .defaultEnabledValue(&origIsEnabled),
                    forceFieldTypeIntern,
                    &isEnabled)) {
                zone.flowType = isEnabled ? forceFieldTypeIntern + 1 : FlowType_None;

                if (zone.flowType == FlowType_Radial) {
                    zone.flowData.radialFlow = RadialFlow();
                }
                if (zone.flowType == FlowType_Central) {
                    zone.flowData.centralFlow = CentralFlow();
                }
                if (zone.flowType == FlowType_Linear) {
                    zone.flowData.linearFlow = LinearFlow();
                }
            }
        }

        auto isForceFieldActive = zone.flowType != FlowType_None;

        ImGui::BeginDisabled(!isForceFieldActive);
        auto posX = ImGui::GetCursorPos().x;
        if (zone.flowType == FlowType_Radial) {
            ImGui::SetCursorPosX(posX);
            AlienImGui::Combo(
                AlienImGui::ComboParameters()
                    .name("方向")
                    .textWidth(RightColumnWidth)
                    .defaultValue(origZone.flowData.radialFlow.orientation)
                    .values({"顺时针", "逆时针"}),
                zone.flowData.radialFlow.orientation);
            ImGui::SetCursorPosX(posX);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("强度")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(0.5f)
                    .logarithmic(true)
                    .format("%.5f")
                    .defaultValue(&origZone.flowData.radialFlow.strength),
                &zone.flowData.radialFlow.strength);
            ImGui::SetCursorPosX(posX);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("漂移角度")
                    .textWidth(RightColumnWidth)
                    .min(-180.0f)
                    .max(180.0f)
                    .format("%.1f")
                    .defaultValue(&origZone.flowData.radialFlow.driftAngle),
                &zone.flowData.radialFlow.driftAngle);
        }
        if (zone.flowType == FlowType_Central) {
            ImGui::SetCursorPosX(posX);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("强度")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(0.5f)
                    .logarithmic(true)
                    .format("%.5f")
                    .defaultValue(&origZone.flowData.centralFlow.strength),
                &zone.flowData.centralFlow.strength);
        }
        if (zone.flowType == FlowType_Linear) {
            ImGui::SetCursorPosX(posX);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("角度")
                    .textWidth(RightColumnWidth)
                    .min(-180.0f)
                    .max(180.0f)
                    .format("%.1f")
                    .defaultValue(&origZone.flowData.linearFlow.angle),
                &zone.flowData.linearFlow.angle);
            ImGui::SetCursorPosX(posX);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("强度")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(0.5f)
                    .logarithmic(true)
                    .format("%.5f")
                    .defaultValue(&origZone.flowData.linearFlow.strength),
                &zone.flowData.linearFlow.strength);
        }
        ImGui::EndDisabled();
    }
    AlienImGui::EndTreeNode();

    /**
     * Physics: Motion
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：运动"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("摩擦")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1)
                .logarithmic(true)
                .defaultValue(&origZone.values.friction)
                .disabledValue(&parameters.baseValues.friction)
                .format("%.4f"),
            &zone.values.friction,
            &zone.activatedValues.friction);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("刚性")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1)
                .defaultValue(&origZone.values.rigidity)
                .disabledValue(&parameters.baseValues.rigidity)
                .format("%.2f"),
            &zone.values.rigidity,
            &zone.activatedValues.rigidity);
    }
    AlienImGui::EndTreeNode();

    /**
     * Physics: Thresholds
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：阈值"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大力")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(3.0f)
                .colorDependence(true)
                .defaultValue(origZone.values.cellMaxForce)
                .disabledValue(parameters.baseValues.cellMaxForce),
            zone.values.cellMaxForce,
            &zone.activatedValues.cellMaxForce);
    }
    AlienImGui::EndTreeNode();

    /**
     * Physics: Binding
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：连接"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("连接建立速度")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(2.0f)
                .defaultValue(&origZone.values.cellFusionVelocity)
                .disabledValue(&parameters.baseValues.cellFusionVelocity),
            &zone.values.cellFusionVelocity,
            &zone.activatedValues.cellFusionVelocity);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大能量")
                .textWidth(RightColumnWidth)
                .min(50.0f)
                .max(10000000.0f)
                .logarithmic(true)
                .infinity(true)
                .format("%.0f")
                .defaultValue(&origZone.values.cellMaxBindingEnergy)
                .disabledValue(&parameters.baseValues.cellMaxBindingEnergy),
            &zone.values.cellMaxBindingEnergy,
            &zone.activatedValues.cellMaxBindingEnergy);
    }
    AlienImGui::EndTreeNode();


    /**
     * Physics: Radiation
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：辐射"))) {
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("停用辐射源")
                .textWidth(RightColumnWidth)
                .defaultValue(origZone.values.radiationDisableSources)
                .tooltip("如果激活，此区域内的所有辐射源都将被停用。"),
            zone.values.radiationDisableSources);
        zone.activatedValues.radiationDisableSources = zone.values.radiationDisableSources;

        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("吸收因子")
                .textWidth(RightColumnWidth)
                .logarithmic(true)
                .colorDependence(true)
                .min(0)
                .max(1.0)
                .format("%.4f")
                .defaultValue(origZone.values.radiationAbsorption)
                .disabledValue(parameters.baseValues.radiationAbsorption),
            zone.values.radiationAbsorption,
            &zone.activatedValues.radiationAbsorption);

        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("辐射类型 1：强度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(0.01f)
                .logarithmic(true)
                .defaultValue(origZone.values.radiationCellAgeStrength)
                .disabledValue(parameters.baseValues.radiationCellAgeStrength)
                .format("%.6f"),
            zone.values.radiationCellAgeStrength,
            &zone.activatedValues.radiationCellAgeStrength);
    }
    AlienImGui::EndTreeNode();

    /**
     * Cell life cycle
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞生命周期"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最小能量")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(10.0f)
                .max(200.0f)
                .defaultValue(origZone.values.cellMinEnergy)
                .disabledValue(parameters.baseValues.cellMinEnergy),
            zone.values.cellMinEnergy,
            &zone.activatedValues.cellMinEnergy);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("垂死细胞的衰变率")
                .colorDependence(true)
                .textWidth(RightColumnWidth)
                .min(1e-6f)
                .max(0.1f)
                .format("%.6f")
                .logarithmic(true)
                .defaultValue(origZone.values.cellDeathProbability)
                .disabledValue(parameters.baseValues.cellDeathProbability),
            zone.values.cellDeathProbability,
            &zone.activatedValues.cellDeathProbability);
    }
    AlienImGui::EndTreeNode();

    /**
     * Mutation 
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("基因组复制变异"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("神经元权重和偏置")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .colorDependence(true)
                .logarithmic(true)
                .defaultValue(origZone.values.cellCopyMutationNeuronData)
                .disabledValue(parameters.baseValues.cellCopyMutationNeuronData),
            zone.values.cellCopyMutationNeuronData,
            &zone.activatedValues.cellCopyMutationNeuronData);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞属性")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationCellProperties)
                .disabledValue(parameters.baseValues.cellCopyMutationCellProperties),
            zone.values.cellCopyMutationCellProperties,
            &zone.activatedValues.cellCopyMutationCellProperties);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("几何")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationGeometry)
                .disabledValue(parameters.baseValues.cellCopyMutationGeometry),
            zone.values.cellCopyMutationGeometry,
            &zone.activatedValues.cellCopyMutationGeometry);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("自定义几何")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationCustomGeometry)
                .disabledValue(parameters.baseValues.cellCopyMutationCustomGeometry),
            zone.values.cellCopyMutationCustomGeometry,
            &zone.activatedValues.cellCopyMutationCustomGeometry);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞功能类型")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationCellFunction)
                .disabledValue(parameters.baseValues.cellCopyMutationCellFunction),
            zone.values.cellCopyMutationCellFunction,
            &zone.activatedValues.cellCopyMutationCellFunction);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞插入")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationInsertion)
                .disabledValue(parameters.baseValues.cellCopyMutationInsertion),
            zone.values.cellCopyMutationInsertion,
            &zone.activatedValues.cellCopyMutationInsertion);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞删除")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationDeletion)
                .disabledValue(parameters.baseValues.cellCopyMutationDeletion),
            zone.values.cellCopyMutationDeletion,
            &zone.activatedValues.cellCopyMutationDeletion);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("平移")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationTranslation)
                .disabledValue(parameters.baseValues.cellCopyMutationTranslation),
            zone.values.cellCopyMutationTranslation,
            &zone.activatedValues.cellCopyMutationTranslation);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("复制")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationDuplication)
                .disabledValue(parameters.baseValues.cellCopyMutationDuplication),
            zone.values.cellCopyMutationDuplication,
            &zone.activatedValues.cellCopyMutationDuplication);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("单个细胞颜色")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationCellColor)
                .disabledValue(parameters.baseValues.cellCopyMutationCellColor),
            zone.values.cellCopyMutationCellColor,
            &zone.activatedValues.cellCopyMutationCellColor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("子基因组颜色")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationSubgenomeColor)
                .disabledValue(parameters.baseValues.cellCopyMutationSubgenomeColor),
            zone.values.cellCopyMutationSubgenomeColor,
            &zone.activatedValues.cellCopyMutationSubgenomeColor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("基因组颜色")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origZone.values.cellCopyMutationGenomeColor)
                .disabledValue(parameters.baseValues.cellCopyMutationGenomeColor),
            zone.values.cellCopyMutationGenomeColor,
            &zone.activatedValues.cellCopyMutationGenomeColor);
    }
    AlienImGui::EndTreeNode();

    /**
     * Attacker
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：攻击者"))) {
        AlienImGui::InputFloatColorMatrix(
            AlienImGui::InputFloatColorMatrixParameters()
                .name("食物链颜色矩阵")
                .max(1)
                .textWidth(RightColumnWidth)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origZone.values.cellFunctionAttackerFoodChainColorMatrix))
                .disabledValue(toVector<MAX_COLORS, MAX_COLORS>(parameters.baseValues.cellFunctionAttackerFoodChainColorMatrix)),
            zone.values.cellFunctionAttackerFoodChainColorMatrix,
            &zone.activatedValues.cellFunctionAttackerFoodChainColorMatrix);
        AlienImGui::InputFloatColorMatrix(
            AlienImGui::InputFloatColorMatrixParameters()
                .name("复杂生物保护")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(20.0f)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origZone.values.cellFunctionAttackerGenomeComplexityBonus))
                .disabledValue(toVector<MAX_COLORS, MAX_COLORS>(parameters.baseValues.cellFunctionAttackerGenomeComplexityBonus)),
            zone.values.cellFunctionAttackerGenomeComplexityBonus,
            &zone.activatedValues.cellFunctionAttackerGenomeComplexityBonus);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("能量消耗")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .format("%.5f")
                .logarithmic(true)
                .defaultValue(origZone.values.cellFunctionAttackerEnergyCost)
                .disabledValue(parameters.baseValues.cellFunctionAttackerEnergyCost),
            zone.values.cellFunctionAttackerEnergyCost,
            &zone.activatedValues.cellFunctionAttackerEnergyCost);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Advanced absorption control
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：高级能量吸收控制")
                                      .visible(parameters.features.advancedAbsorptionControl)
                                      .blinkWhenActivated(true))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("低速度惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .format("%.2f")
                .defaultValue(origZone.values.radiationAbsorptionLowVelocityPenalty)
                .disabledValue(parameters.baseValues.radiationAbsorptionLowVelocityPenalty),
            zone.values.radiationAbsorptionLowVelocityPenalty,
            &zone.activatedValues.radiationAbsorptionLowVelocityPenalty);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("低基因组复杂度惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .format("%.2f")
                .defaultValue(origZone.values.radiationAbsorptionLowGenomeComplexityPenalty)
                .disabledValue(parameters.baseValues.radiationAbsorptionLowGenomeComplexityPenalty),
            zone.values.radiationAbsorptionLowGenomeComplexityPenalty,
            &zone.activatedValues.radiationAbsorptionLowGenomeComplexityPenalty);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Advanced attacker control
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：高级攻击者控制")
                                      .visible(parameters.features.advancedAttackerControl)
                                      .blinkWhenActivated(true))) {
        AlienImGui::InputFloatColorMatrix(
            AlienImGui::InputFloatColorMatrixParameters()
                .name("新复杂突变体保护")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origZone.values.cellFunctionAttackerNewComplexMutantPenalty))
                .disabledValue(toVector<MAX_COLORS, MAX_COLORS>(parameters.baseValues.cellFunctionAttackerNewComplexMutantPenalty)),
            zone.values.cellFunctionAttackerNewComplexMutantPenalty,
            &zone.activatedValues.cellFunctionAttackerNewComplexMutantPenalty);

        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("几何惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(5.0f)
                .defaultValue(origZone.values.cellFunctionAttackerGeometryDeviationExponent)
                .disabledValue(parameters.baseValues.cellFunctionAttackerGeometryDeviationExponent),
            zone.values.cellFunctionAttackerGeometryDeviationExponent,
            &zone.activatedValues.cellFunctionAttackerGeometryDeviationExponent);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("连接不匹配惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .defaultValue(origZone.values.cellFunctionAttackerConnectionsMismatchPenalty)
                .disabledValue(parameters.baseValues.cellFunctionAttackerConnectionsMismatchPenalty),
            zone.values.cellFunctionAttackerConnectionsMismatchPenalty,
            &zone.activatedValues.cellFunctionAttackerConnectionsMismatchPenalty);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Cell age limiter
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：细胞年龄限制器")
                                      .visible(parameters.features.cellAgeLimiter)
                                      .blinkWhenActivated(true))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大非活动细胞年龄")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(1.0f)
                .max(10000000.0f)
                .logarithmic(true)
                .infinity(true)
                .format("%.0f")
                .disabledValue(parameters.baseValues.cellInactiveMaxAge)
                .defaultValue(origZone.values.cellInactiveMaxAge),
            zone.values.cellInactiveMaxAge,
            &zone.activatedValues.cellInactiveMaxAge);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Cell color transition rules
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：细胞颜色转换规则")
                                      .visible(parameters.features.cellColorTransitionRules)
                                      .blinkWhenActivated(true))) {
        ImGui::Checkbox("##cellColorTransition", &zone.activatedValues.cellColorTransition);
        ImGui::SameLine();
        ImGui::BeginDisabled(!zone.activatedValues.cellColorTransition);
        auto posX = ImGui::GetCursorPos().x;
        for (int color = 0; color < MAX_COLORS; ++color) {
            ImGui::SetCursorPosX(posX);
            ImGui::PushID(color);
            auto parameters = AlienImGui::InputColorTransitionParameters()
                                  .textWidth(RightColumnWidth)
                                  .color(color)
                                  .defaultTargetColor(origZone.values.cellColorTransitionTargetColor[color])
                                  .defaultTransitionAge(origZone.values.cellColorTransitionDuration[color])
                                  .logarithmic(true)
                                  .infinity(true);
            if (0 == color) {
                parameters.name("目标颜色和持续时间");
            }
            AlienImGui::InputColorTransition(
                parameters, color, zone.values.cellColorTransitionTargetColor[color], zone.values.cellColorTransitionDuration[color]);
            ImGui::PopID();
        }
        ImGui::EndDisabled();
        if (!zone.activatedValues.cellColorTransition) {
            for (int color = 0; color < MAX_COLORS; ++color) {
                zone.values.cellColorTransitionTargetColor[color] = parameters.baseValues.cellColorTransitionTargetColor[color];
                zone.values.cellColorTransitionDuration[color] = parameters.baseValues.cellColorTransitionDuration[color];
            }
        }
    }
    AlienImGui::EndTreeNode();

    SimulationParametersValidationService::get().validateAndCorrect(zone, parameters);

    if (zone != lastZone) {
        auto isRunning = _simulationFacade->isSimulationRunning();
        _simulationFacade->setSimulationParameters(
            parameters, isRunning ? SimulationParametersUpdateConfig::AllExceptChangingPositions : SimulationParametersUpdateConfig::All);
    }
}

std::string _SimulationParametersZoneWidgets::getLocationName()
{
    return "“" + _zoneName + "”的模拟器参数";
}

int _SimulationParametersZoneWidgets::getLocationIndex() const
{
    return _locationIndex;
}

void _SimulationParametersZoneWidgets::setLocationIndex(int locationIndex)
{
    _locationIndex = locationIndex;
}

void _SimulationParametersZoneWidgets::setDefaultSpotData(SimulationParametersZone& spot) const
{
    auto worldSize = _simulationFacade->getWorldSize();

    auto maxRadius = toFloat(std::min(worldSize.x, worldSize.y)) / 2;
    if (spot.shapeType == SpotShapeType_Circular) {
        spot.shapeData.circularSpot.coreRadius = maxRadius / 3;
    } else {
        spot.shapeData.rectangularSpot.height = maxRadius / 3;
        spot.shapeData.rectangularSpot.width = maxRadius / 3;
    }
}
