#include "SimulationParametersBaseWidgets.h"

#include <imgui.h>

#include "EngineInterface/Descriptions.h"
#include "EngineInterface/SimulationFacade.h"
#include "EngineInterface/SimulationParametersEditService.h"
#include "EngineInterface/SimulationParametersTypes.h"
#include "EngineInterface/SimulationParametersUpdateConfig.h"
#include "EngineInterface/SimulationParametersValidationService.h"

#include "AlienImGui.h"
#include "CellFunctionStrings.h"
#include "HelpStrings.h"
#include "SimulationParametersMainWindow.h"

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

void _SimulationParametersBaseWidgets::init(SimulationFacade const& simulationFacade)
{
    _simulationFacade = simulationFacade;
    for (int i = 0; i < CellFunction_Count; ++i) {
        _cellFunctionStrings.emplace_back(Const::CellFunctionToStringMap.at(i));
    }
}

void _SimulationParametersBaseWidgets::process()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();
    auto lastParameters = parameters;

    /**
     * General
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("常规"))) {
        AlienImGui::InputText(
            AlienImGui::InputTextParameters().name("项目名称").textWidth(RightColumnWidth).defaultValue(origParameters.projectName),
            parameters.projectName,
            sizeof(Char64) / sizeof(char));
    }
    AlienImGui::EndTreeNode();
    /**
     * Rendering
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("可视化"))) {
        AlienImGui::ColorButtonWithPicker(
            AlienImGui::ColorButtonWithPickerParameters().name("背景颜色").textWidth(RightColumnWidth).defaultValue(origParameters.backgroundColor),
            parameters.backgroundColor,
            _backupColor,
            _zoneColorPalette.getReference());
        AlienImGui::Switcher(
            AlienImGui::SwitcherParameters()
                .name("主要细胞着色")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellColoring)
                .values(
                    {"能量",
                     "标准细胞颜色",
                     "突变体",
                     "突变体和细胞功能",
                     "细胞状态",
                     "基因组复杂度",
                     "单个细胞功能",
                     "所有细胞功能"})
                .tooltip(Const::ColoringParameterTooltip),
            parameters.cellColoring);
        if (parameters.cellColoring == CellColoring_CellFunction) {
            AlienImGui::Switcher(
                AlienImGui::SwitcherParameters()
                    .name("高亮细胞功能")
                    .textWidth(RightColumnWidth)
                    .defaultValue(origParameters.highlightedCellFunction)
                    .values(_cellFunctionStrings)
                    .tooltip("可以在这里选择要突出显示的特定细胞功能类型。"),
                parameters.highlightedCellFunction);
        }
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞半径")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(0.5f)
                .defaultValue(&origParameters.cellRadius)
                .tooltip("指定绘制的细胞在单位长度中的半径。"),
            &parameters.cellRadius);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞活动的缩放级别")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(32.0f)
                .infinity(true)
                .defaultValue(&origParameters.zoomLevelNeuronalActivity)
                .tooltip("神经元活动变得可见的缩放级别。"),
            &parameters.zoomLevelNeuronalActivity);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("攻击可视化")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.attackVisualization)
                .tooltip("如果激活，攻击细胞的成功攻击将被可视化。"),
            parameters.attackVisualization);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("肌肉运动可视化")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.muscleMovementVisualization)
                .tooltip("如果激活，肌肉细胞移动的方向将被可视化。"),
            parameters.muscleMovementVisualization);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("无边框渲染")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.borderlessRendering)
                .tooltip("如果激活，模拟将在视口中周期性地渲染。"),
            parameters.borderlessRendering);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("自适应空间网格")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.gridLines)
                .tooltip("根据缩放级别在背景中绘制合适的网格。"),
            parameters.gridLines);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("标记参考域")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.markReferenceDomain)
                .tooltip("在世界重复自身之前沿其边缘绘制边界。"),
            parameters.markReferenceDomain);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("显示辐射源")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.showRadiationSources)
                .tooltip("在辐射源的中心绘制红色十字。"),
            parameters.showRadiationSources);
    }
    AlienImGui::EndTreeNode();

    /**
     * Numerics
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("数值"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("时间步长")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .defaultValue(&origParameters.timestepSize)
                .tooltip(std::string("单个模拟步骤中计算的时间持续时间。较小的值可以提高模拟的准确性，而较大的值可能导致数值不稳定。")),
            &parameters.timestepSize);
    }
    AlienImGui::EndTreeNode();

    /**
     * Physics: Motion
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：运动"))) {
        if (AlienImGui::Switcher(
                AlienImGui::SwitcherParameters()
                    .name("运动类型")
                    .textWidth(RightColumnWidth)
                    .defaultValue(origParameters.motionType)
                    .values({"流体动力学", "基于碰撞"})
                    .tooltip(std::string(
                        "这里定义了粒子运动的算法。如果选择“流体动力学”，则使用 SPH 流体求解器计算力。粒子随后表现得像（可压缩的）液体或气体。"
                        "另一个选项“基于碰撞”基于粒子碰撞计算力，应优先用于固体的机械模拟。")),
                parameters.motionType)) {
            if (parameters.motionType == MotionType_Fluid) {
                parameters.motionData.fluidMotion = FluidMotion();
            } else {
                parameters.motionData.collisionMotion = CollisionMotion();
            }
        }
        if (parameters.motionType == MotionType_Fluid) {
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("平滑长度")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(3.0f)
                    .defaultValue(&origParameters.motionData.fluidMotion.smoothingLength)
                    .tooltip(std::string("平滑长度决定了在计算密度、压力和粘度时相邻粒子的影响区域。过小的值会导致数值不稳定，而过大的值会导致粒子漂散。")),
                &parameters.motionData.fluidMotion.smoothingLength);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("压力")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(0.3f)
                    .defaultValue(&origParameters.motionData.fluidMotion.pressureStrength)
                    .tooltip(std::string("该参数可用于控制压力的强度。")),
                &parameters.motionData.fluidMotion.pressureStrength);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("粘度")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(0.3f)
                    .defaultValue(&origParameters.motionData.fluidMotion.viscosityStrength)
                    .tooltip(std::string("该参数可用于控制粘度的强度。较大的值会导致更平滑的运动。")),
                &parameters.motionData.fluidMotion.viscosityStrength);
        } else {
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("排斥力强度")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(0.3f)
                    .defaultValue(&origParameters.motionData.collisionMotion.cellRepulsionStrength)
                    .tooltip(std::string("两个未连接的细胞之间的排斥力强度。")),
                &parameters.motionData.collisionMotion.cellRepulsionStrength);
            AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("最大碰撞距离")
                    .textWidth(RightColumnWidth)
                    .min(0)
                    .max(3.0f)
                    .defaultValue(&origParameters.motionData.collisionMotion.cellMaxCollisionDistance)
                    .tooltip(std::string("两个细胞可以发生碰撞的最大距离。")),
                &parameters.motionData.collisionMotion.cellMaxCollisionDistance);
        }
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("摩擦")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .logarithmic(true)
                .format("%.4f")
                .defaultValue(&origParameters.baseValues.friction)
                .tooltip(std::string("这指定了每个时间步长减慢的速度比例。")),
            &parameters.baseValues.friction);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("刚性")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .format("%.2f")
                .defaultValue(&origParameters.baseValues.rigidity)
                .tooltip(std::string(
                    "控制连接细胞的刚性。\n较高的值将使连接的细胞更均匀地作为刚体移动。")),
            &parameters.baseValues.rigidity);
    }
    AlienImGui::EndTreeNode();

    /**
     * Physics: Thresholds
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：阈值"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大速度")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(6.0f)
                .defaultValue(&origParameters.cellMaxVelocity)
                .tooltip(std::string("细胞可以达到的最大速度。")),
            &parameters.cellMaxVelocity);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大力")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(3.0f)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellMaxForce)
                .tooltip(std::string("在不导致细胞解体的情况下可以施加到细胞上的最大力。")),
            parameters.baseValues.cellMaxForce);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最小距离")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .defaultValue(&origParameters.cellMinDistance)
                .tooltip(std::string("两个细胞之间的最小距离。")),
            &parameters.cellMinDistance);
    }
    AlienImGui::EndTreeNode();

    /**
     * Physics: Binding
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：连接"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大距离")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(5.0f)
                .colorDependence(true)
                .defaultValue(origParameters.cellMaxBindingDistance)
                .tooltip(std::string("两个细胞可以建立连接的最大距离。")),
            parameters.cellMaxBindingDistance);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("融合速度")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(2.0f)
                .defaultValue(&origParameters.baseValues.cellFusionVelocity)
                .tooltip(std::string("两个碰撞细胞建立连接所需的最小相对速度。")),
            &parameters.baseValues.cellFusionVelocity);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大能量")
                .textWidth(RightColumnWidth)
                .min(50.0f)
                .max(10000000.0f)
                .logarithmic(true)
                .infinity(true)
                .format("%.0f")
                .defaultValue(&origParameters.baseValues.cellMaxBindingEnergy)
                .tooltip(std::string("细胞可以包含与相邻细胞键合时的最大能量。如果细胞的能量超过此值，所有键合将被破坏。")),
            &parameters.baseValues.cellMaxBindingEnergy);
    }
    AlienImGui::EndTreeNode();

    /**
     * Physics: Radiation
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("物理：辐射"))) {
        auto& editService = SimulationParametersEditService::get();
        auto strength = editService.getRadiationStrengths(parameters);
        auto origStrengths = editService.getRadiationStrengths(origParameters);
        auto editedStrength = strength;
        if (AlienImGui::SliderFloat(
                AlienImGui::SliderFloatParameters()
                    .name("相对强度")
                    .textWidth(RightColumnWidth)
                    .min(0.0f)
                    .max(1.0f)
                    .format("%.3f")
                    .defaultValue(&origStrengths.values.front())
                    .tooltip("细胞会随着时间的推移发射能量粒子。其中一部分能量可以直接释放在细胞附近，而其余部分则由可用的辐射源之一利用。"
                             "该参数决定了分配给细胞附近发射的能量粒子的能量比例。允许 0 到 1 之间的值。"),
                &editedStrength.values.front(),
                nullptr,
                &parameters.baseStrengthRatioPinned)) {
            editService.adaptRadiationStrengths(editedStrength, strength, 0);
            editService.applyRadiationStrengths(parameters, editedStrength);
        }

        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("吸收因子")
                .textWidth(RightColumnWidth)
                .logarithmic(true)
                .colorDependence(true)
                .min(0)
                .max(1.0)
                .format("%.4f")
                .defaultValue(origParameters.baseValues.radiationAbsorption)
                .tooltip("可以在这里指定细胞从入射能量粒子中吸收能量的比例。"),
            parameters.baseValues.radiationAbsorption);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("辐射类型 I：强度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(0.01f)
                .logarithmic(true)
                .format("%.6f")
                .defaultValue(origParameters.baseValues.radiationCellAgeStrength)
                .tooltip("表示老化细胞发射的粒子的能量有多大。"),
            parameters.baseValues.radiationCellAgeStrength);
        AlienImGui::SliderInt(
            AlienImGui::SliderIntParameters()
                .name("辐射类型 I：最小年龄")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .infinity(true)
                .min(0)
                .max(10000000)
                .logarithmic(true)
                .defaultValue(origParameters.radiationMinCellAge)
                .tooltip("可以在这里定义细胞发射能量粒子的最小年龄。"),
            parameters.radiationMinCellAge);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("辐射类型 II：强度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(0.01f)
                .logarithmic(true)
                .format("%.6f")
                .defaultValue(origParameters.highRadiationFactor)
                .tooltip("表示高能量细胞发射的粒子的能量有多大。"),
            parameters.highRadiationFactor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("辐射类型 II：能量阈值")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .infinity(true)
                .min(0)
                .max(100000.0f)
                .logarithmic(true)
                .format("%.1f")
                .defaultValue(origParameters.highRadiationMinCellEnergy)
                .tooltip("可以在这里定义细胞发射能量粒子的最小能量。"),
            parameters.highRadiationMinCellEnergy);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最小分裂能量")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .infinity(true)
                .min(1.0f)
                .max(10000.0f)
                .logarithmic(true)
                .format("%.0f")
                .defaultValue(origParameters.particleSplitEnergy)
                .tooltip("能量粒子在分裂成两个粒子（并因此获得一个小动量）之前所需的最小能量。分裂不会立即发生，而是需要经过一定时间。"),
            parameters.particleSplitEnergy);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("能量到细胞的转化")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.particleTransformationAllowed)
                .tooltip("如果激活，当粒子的能量超过正常能量值时，能量粒子将转化为细胞。"),
            parameters.particleTransformationAllowed);
    }
    AlienImGui::EndTreeNode();

    /**
     * Cell life cycle
     */
    ImGui::PushID("Transformation");
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞生命周期"))) {
        AlienImGui::SliderInt(
            AlienImGui::SliderIntParameters()
                .name("最大年龄")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .logarithmic(true)
                .infinity(true)
                .min(1)
                .max(10000000)
                .defaultValue(origParameters.cellMaxAge)
                .tooltip("定义细胞的最大年龄。如果细胞超过此年龄，它将转化为能量粒子。"),
            parameters.cellMaxAge);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最小能量")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(10.0f)
                .max(200.0f)
                .defaultValue(origParameters.baseValues.cellMinEnergy)
                .tooltip("细胞生存所需的最小能量。"),
            parameters.baseValues.cellMinEnergy);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("正常能量")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(10.0f)
                .max(200.0f)
                .defaultValue(origParameters.cellNormalEnergy)
                .tooltip("这里定义了细胞的正常能量值。它在各种上下文中用作参考值："
                         "\n\n" ICON_FA_CHEVRON_RIGHT
                         " 攻击细胞和发送器细胞：当这些细胞的能量高于正常值时，它们的一部分能量会分配给周围的细胞。\n\n" ICON_FA_CHEVRON_RIGHT
                         " 构建细胞：创建新细胞需要能量。只有当构建细胞的剩余能量不低于正常值时，才会执行新细胞的创建。\n\n" ICON_FA_CHEVRON_RIGHT
                         " 如果激活了能量粒子到细胞的转化，当粒子的能量超过正常值时，能量粒子将转化为细胞。"),
            parameters.cellNormalEnergy);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("垂死细胞的衰变率")
                .colorDependence(true)
                .textWidth(RightColumnWidth)
                .min(1e-6f)
                .max(0.1f)
                .format("%.6f")
                .logarithmic(true)
                .defaultValue(origParameters.baseValues.cellDeathProbability)
                .tooltip("细胞处于“垂死”状态时，每个时间步长发生解体（即转化为能量粒子）的概率。这可能在满足以下条件之一时发生：\n\n" ICON_FA_CHEVRON_RIGHT
                         " 细胞能量过低。\n\n" ICON_FA_CHEVRON_RIGHT " 细胞已超过其最大年龄。"),
            parameters.baseValues.cellDeathProbability);
        AlienImGui::Switcher(
            AlienImGui::SwitcherParameters()
                .name("细胞死亡后果")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellDeathConsequences)
                .values({"无", "整个生物体死亡", "分离的生物体部分死亡"})
                .tooltip("这里可以定义当生物体的一个细胞处于“垂死”状态时会发生什么。\n\n" ICON_FA_CHEVRON_RIGHT
                         " 无：只有该细胞死亡。\n\n" ICON_FA_CHEVRON_RIGHT
                         " 整个生物体死亡：生物体的所有细胞也会死亡。\n\n" ICON_FA_CHEVRON_RIGHT
                         " 分离的生物体部分死亡：只有不再与用于自我复制的构建细胞连接的生物体部分死亡。"),
            parameters.cellDeathConsequences);
    }
    AlienImGui::EndTreeNode();
    ImGui::PopID();

    /**
     * Mutations
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("基因组复制变异"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("神经网络")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationNeuronData)
                .tooltip("这种类型的变异可以改变基因组中编码的每个神经元细胞的神经网络的权重、偏置和激活函数。"),
            parameters.baseValues.cellCopyMutationNeuronData);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞属性")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationCellProperties)
                .tooltip("这种类型的变异会改变随机属性（例如（输入）执行顺序编号、所需能量、阻止输出以及特定功能属性，如传感器的最小密度、神经网络权重等）。"
                         "空间结构、颜色、细胞功能类型和自我复制能力不会改变。此变异应用于基因组中编码的每个细胞。"),
            parameters.baseValues.cellCopyMutationCellProperties);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("几何")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationGeometry)
                .tooltip("这种类型的变异会改变几何类型、连接距离、刚度和单个构建标志。发生变化的概率由指定值乘以基因组中编码的细胞数量得出。"),
            parameters.baseValues.cellCopyMutationGeometry);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("自定义几何")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationCustomGeometry)
                .tooltip("这种类型的变异只改变自定义几何的角度和所需连接。发生变化的概率由指定值乘以基因组中编码的细胞数量得出。"),
            parameters.baseValues.cellCopyMutationCustomGeometry);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("细胞功能类型")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationCellFunction)
                .tooltip("这种类型的变异会改变细胞功能的类型。改变后的细胞功能将具有随机属性。发生变化的概率由指定值乘以基因组中编码的细胞数量得出。"
                         "如果“保持自我复制”标志被禁用，它还可以通过将构建细胞改变为其他类型或反之来改变自我复制能力。"),
            parameters.baseValues.cellCopyMutationCellFunction);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("插入")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationInsertion)
                .tooltip("这种类型的变异会在基因组的随机位置插入一个新的细胞描述。发生变化的概率由指定值乘以基因组中编码的细胞数量得出。"),
            parameters.baseValues.cellCopyMutationInsertion);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("删除")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationDeletion)
                .tooltip("这种类型的变异会从基因组的随机位置删除一个细胞描述。发生变化的概率由指定值乘以基因组中编码的细胞数量得出。"),
            parameters.baseValues.cellCopyMutationDeletion);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("平移")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationTranslation)
                .tooltip("这种类型的变异会将基因组中随机位置的一块细胞描述移动到新的随机位置。"),
            parameters.baseValues.cellCopyMutationTranslation);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("复制")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationDuplication)
                .tooltip("这种类型的变异会将基因组中随机位置的一块细胞描述复制到新的随机位置。"),
            parameters.baseValues.cellCopyMutationDuplication);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("单个细胞颜色")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationCellColor)
                .tooltip("这种类型的变异通过使用指定的颜色转换来改变基因组中单个细胞描述的颜色。发生变化的概率由指定值乘以基因组中编码的细胞数量得出。"),
            parameters.baseValues.cellCopyMutationCellColor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("子基因组颜色")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationSubgenomeColor)
                .tooltip("这种类型的变异通过使用指定的颜色转换来改变子基因组中所有细胞描述的颜色。"),
            parameters.baseValues.cellCopyMutationSubgenomeColor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("基因组颜色")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.7f")
                .logarithmic(true)
                .colorDependence(true)
                .defaultValue(origParameters.baseValues.cellCopyMutationGenomeColor)
                .tooltip("这种类型的变异通过使用指定的颜色转换来改变基因组中所有细胞描述的颜色。"),
            parameters.baseValues.cellCopyMutationGenomeColor);
        AlienImGui::CheckboxColorMatrix(
            AlienImGui::CheckboxColorMatrixParameters()
                .name("颜色转换")
                .textWidth(RightColumnWidth)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.cellCopyMutationColorTransitions))
                .tooltip("颜色转换用于颜色变异。行索引表示源颜色，列索引表示目标颜色。"),
            parameters.cellCopyMutationColorTransitions);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("防止基因组深度增加")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellCopyMutationPreventDepthIncrease)
                .tooltip(std::string("基因组具有树状结构，因为它可以包含子基因组。如果激活此标志，变异不会增加基因组结构的深度。")),
            parameters.cellCopyMutationPreventDepthIncrease);
        auto preserveSelfReplication = !parameters.cellCopyMutationSelfReplication;
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("保持自我复制")
                .textWidth(RightColumnWidth)
                .defaultValue(!origParameters.cellCopyMutationSelfReplication)
                .tooltip("如果停用，变异也可以通过将构建细胞改变为其他类型或反之来改变基因组中的自我复制能力。"),
            preserveSelfReplication);
        parameters.cellCopyMutationSelfReplication = !preserveSelfReplication;
    }
    AlienImGui::EndTreeNode();

    /**
     * Attacker
     */
    ImGui::PushID("Attacker");
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：攻击者"))) {
        AlienImGui::InputFloatColorMatrix(
            AlienImGui::InputFloatColorMatrixParameters()
                .name("食物链颜色矩阵")
                .max(1)
                .textWidth(RightColumnWidth)
                .tooltip("该矩阵可用于确定一个细胞攻击另一个细胞的效果。攻击细胞的颜色对应行号，被攻击细胞的颜色对应列号。值为 0 表示被攻击的细胞无法被消化，"
                         "即无法获得能量。值为 1 表示在消化过程中可以获得最大能量。\n\n示例：如果在第 2 行（红色）和第 3 列（绿色）输入零，"
                         "则表示红色细胞不能吃掉绿色细胞。")
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.baseValues.cellFunctionAttackerFoodChainColorMatrix)),
            parameters.baseValues.cellFunctionAttackerFoodChainColorMatrix);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("攻击强度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .logarithmic(true)
                .min(0)
                .max(0.5f)
                .defaultValue(origParameters.cellFunctionAttackerStrength)
                .tooltip("表示成功攻击的细胞被削弱的能量比例。然而，此能量比例会受到攻击者模拟参数中可调整的其他因素的影响。"),
            parameters.cellFunctionAttackerStrength);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("攻击半径")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(3.0f)
                .defaultValue(origParameters.cellFunctionAttackerRadius)
                .tooltip("攻击细胞可以攻击另一个细胞的最大距离。"),
            parameters.cellFunctionAttackerRadius);
        AlienImGui::InputFloatColorMatrix(
            AlienImGui::InputFloatColorMatrixParameters()
                .name("复杂生物保护")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(20.0f)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.baseValues.cellFunctionAttackerGenomeComplexityBonus))
                .tooltip("该参数越大，攻击具有更复杂基因组的生物所能获得的能量就越少。"),
            parameters.baseValues.cellFunctionAttackerGenomeComplexityBonus);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("能量消耗")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .format("%.5f")
                .logarithmic(true)
                .defaultValue(origParameters.baseValues.cellFunctionAttackerEnergyCost)
                .tooltip("细胞尝试攻击时以发射能量粒子形式损失的能量。"),
            parameters.baseValues.cellFunctionAttackerEnergyCost);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("摧毁细胞")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellFunctionAttackerDestroyCells)
                .tooltip("如果激活，攻击细胞能够摧毁其他细胞。如果停用，它只会损坏它们。"),
            parameters.cellFunctionAttackerDestroyCells);
    }
    AlienImGui::EndTreeNode();
    ImGui::PopID();

    /**
     * Constructor
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：构建者"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("连接距离")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.1f)
                .max(3.0f)
                .defaultValue(origParameters.cellFunctionConstructorConnectingCellMaxDistance)
                .tooltip("构建者可以在此距离内自动将构建的细胞连接到附近的其他细胞。"),
            parameters.cellFunctionConstructorConnectingCellMaxDistance);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("完整性检查")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellFunctionConstructorCheckCompletenessForSelfReplication)
                .tooltip("如果激活，只有当细胞网络中所有其他非自我复制的构建者都完成时，自我复制过程才能开始。"),
            parameters.cellFunctionConstructorCheckCompletenessForSelfReplication);
    }
    AlienImGui::EndTreeNode();

    /**
     * Defender
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：防御者"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("抗攻击者强度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(1.0f)
                .max(5.0f)
                .defaultValue(origParameters.cellFunctionDefenderAgainstAttackerStrength)
                .tooltip("如果被攻击的细胞连接到防御细胞，或者它本身是防御细胞，则攻击强度将按此因子降低。"),
            parameters.cellFunctionDefenderAgainstAttackerStrength);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("抗注射者强度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(1.0f)
                .max(5.0f)
                .defaultValue(origParameters.cellFunctionDefenderAgainstInjectorStrength)
                .tooltip("如果构建细胞被注射者攻击并连接到防御细胞，则注射持续时间将按此因子增加。"),
            parameters.cellFunctionDefenderAgainstInjectorStrength);
    }
    AlienImGui::EndTreeNode();

    /**
     * Injector
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：注射者"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("注射半径")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.1f)
                .max(4.0f)
                .defaultValue(origParameters.cellFunctionInjectorRadius)
                .tooltip("注射细胞可以感染另一个细胞的最大距离。"),
            parameters.cellFunctionInjectorRadius);
        AlienImGui::InputIntColorMatrix(
            AlienImGui::InputIntColorMatrixParameters()
                .name("注射时间")
                .logarithmic(true)
                .max(100000)
                .textWidth(RightColumnWidth)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.cellFunctionInjectorDurationColorMatrix))
                .tooltip("注射细胞感染另一个细胞所需的激活次数。一次激活通常需要 6 个时间步。行号决定注射细胞的颜色，而列号对应被感染细胞的颜色。"),
            parameters.cellFunctionInjectorDurationColorMatrix);
    }
    AlienImGui::EndTreeNode();

    /**
     * Muscle
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：肌肉"))) {
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("向目标移动")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellFunctionMuscleMovementTowardTargetedObject)
                .tooltip("如果激活，处于移动模式的肌肉细胞只有在触发信号来自已瞄准目标的传感器细胞时才会移动。输入中指定的角度将相对于目标进行解释。"),
            parameters.cellFunctionMuscleMovementTowardTargetedObject);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("能量消耗")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(5.0f)
                .format("%.5f")
                .logarithmic(true)
                .defaultValue(origParameters.cellFunctionMuscleEnergyCost)
                .tooltip("细胞肌肉动作以发射能量粒子形式损失的能量。"),
            parameters.cellFunctionMuscleEnergyCost);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("移动加速度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(0.4f)
                .logarithmic(true)
                .defaultValue(origParameters.cellFunctionMuscleMovementAcceleration)
                .tooltip("肌肉细胞在激活期间可以修改其速度的最大值。此参数仅适用于处于移动模式的肌肉细胞。"),
            parameters.cellFunctionMuscleMovementAcceleration);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("收缩和伸展变化量")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(0.1f)
                .defaultValue(origParameters.cellFunctionMuscleContractionExpansionDelta)
                .tooltip("肌肉细胞可以缩短或延长细胞连接的最大长度。此参数仅适用于处于收缩/伸展模式的肌肉细胞。"),
            parameters.cellFunctionMuscleContractionExpansionDelta);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("弯曲角度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(10.0f)
                .defaultValue(origParameters.cellFunctionMuscleBendingAngle)
                .tooltip("肌肉细胞可以增大/减小两个细胞连接之间角度的最大值。此参数仅适用于处于弯曲模式的肌肉细胞。"),
            parameters.cellFunctionMuscleBendingAngle);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("弯曲加速度")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(0.5f)
                .defaultValue(origParameters.cellFunctionMuscleBendingAcceleration)
                .tooltip("肌肉细胞在弯曲动作期间可以修改其速度的最大值。此参数仅适用于处于弯曲模式的肌肉细胞。"),
            parameters.cellFunctionMuscleBendingAcceleration);
    }
    AlienImGui::EndTreeNode();

    /**
     * Sensor
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：传感器"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("半径")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(10.0f)
                .max(800.0f)
                .defaultValue(origParameters.cellFunctionSensorRange)
                .tooltip("传感器细胞可以检测质量集中的最大半径。"),
            parameters.cellFunctionSensorRange);
    }
    AlienImGui::EndTreeNode();

    /**
     * Transmitter
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：发送器"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("能量分配半径")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(5.0f)
                .defaultValue(origParameters.cellFunctionTransmitterEnergyDistributionRadius)
                .tooltip("发送器细胞将其额外能量传递给附近发送器或构建细胞的最大距离。"),
            parameters.cellFunctionTransmitterEnergyDistributionRadius);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("能量分配值")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(20.0f)
                .defaultValue(origParameters.cellFunctionTransmitterEnergyDistributionValue)
                .tooltip("发送器细胞可以传递给附近发送器、构建细胞或连接细胞的能量量。"),
            parameters.cellFunctionTransmitterEnergyDistributionValue);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("同生物能量分配")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellFunctionTransmitterEnergyDistributionSameCreature)
                .tooltip("如果激活，发送器细胞只能将能量传递给属于同一生物的附近细胞。"),
            parameters.cellFunctionTransmitterEnergyDistributionSameCreature);
    }
    AlienImGui::EndTreeNode();

    /**
     * Reconnector
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：重连接者"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("半径")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(3.0f)
                .defaultValue(origParameters.cellFunctionReconnectorRadius)
                .tooltip("重连接细胞可以建立或破坏与其他细胞的连接的最大半径。"),
            parameters.cellFunctionReconnectorRadius);
    }
    AlienImGui::EndTreeNode();

    /**
     * Detonator
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().name("细胞功能：引爆者"))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("爆炸半径")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(10.0f)
                .defaultValue(origParameters.cellFunctionDetonatorRadius)
                .tooltip("爆炸的半径。"),
            parameters.cellFunctionDetonatorRadius);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("连锁爆炸概率")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(1.0f)
                .defaultValue(origParameters.cellFunctionDetonatorChainExplosionProbability)
                .tooltip("一个引爆者的爆炸触发爆炸半径内其他引爆者爆炸的概率。"),
            parameters.cellFunctionDetonatorChainExplosionProbability);
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
                .name("低基因组复杂度惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .format("%.2f")
                .defaultValue(origParameters.baseValues.radiationAbsorptionLowGenomeComplexityPenalty)
                .tooltip(Const::ParameterRadiationAbsorptionLowGenomeComplexityPenaltyTooltip),
            parameters.baseValues.radiationAbsorptionLowGenomeComplexityPenalty);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("低连接惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(5.0f)
                .format("%.1f")
                .defaultValue(origParameters.radiationAbsorptionLowConnectionPenalty)
                .tooltip("当此参数增大时，细胞连接较少的细胞将从入射能量粒子中吸收更少的能量。"),
            parameters.radiationAbsorptionLowConnectionPenalty);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("高速度惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(30.0f)
                .logarithmic(true)
                .format("%.2f")
                .defaultValue(origParameters.radiationAbsorptionHighVelocityPenalty)
                .tooltip("当此参数增大时，快速移动的细胞将从入射能量粒子中吸收更少的能量。"),
            parameters.radiationAbsorptionHighVelocityPenalty);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("低速度惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .format("%.2f")
                .defaultValue(origParameters.baseValues.radiationAbsorptionLowVelocityPenalty)
                .tooltip("当此参数增大时，缓慢移动的细胞将从入射能量粒子中吸收更少的能量。"),
            parameters.baseValues.radiationAbsorptionLowVelocityPenalty);
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
                .name("同突变体保护")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.cellFunctionAttackerSameMutantPenalty))
                .tooltip("此参数越大，攻击具有相同突变 id 的生物所能获得的能量就越少。"),
            parameters.cellFunctionAttackerSameMutantPenalty);
        AlienImGui::InputFloatColorMatrix(
            AlienImGui::InputFloatColorMatrixParameters()
                .name("新复杂突变体保护")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.baseValues.cellFunctionAttackerNewComplexMutantPenalty))
                .tooltip("高值可以保护具有相同或更高基因组复杂度的新突变体免受攻击。"),
            parameters.baseValues.cellFunctionAttackerNewComplexMutantPenalty);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("传感器检测因子")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .defaultValue(origParameters.cellFunctionAttackerSensorDetectionFactor)
                .tooltip("此参数控制目标是否必须先被传感器检测到才能被攻击。此值越大，如果目标尚未被检测到，则攻击期间能获得的能量就越少。为此，"
                         "攻击细胞会搜索连接的（或连接-连接的）传感器细胞，查看它们上次检测到哪些细胞网络，并与被攻击的目标进行比较。"),
            parameters.cellFunctionAttackerSensorDetectionFactor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("几何惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(5.0f)
                .defaultValue(origParameters.baseValues.cellFunctionAttackerGeometryDeviationExponent)
                .tooltip("此值越大，如果被攻击细胞的局部几何形状与攻击细胞不匹配，细胞从攻击中获得的能量就越少。"),
            parameters.baseValues.cellFunctionAttackerGeometryDeviationExponent);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("连接不匹配惩罚")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(1.0f)
                .defaultValue(origParameters.baseValues.cellFunctionAttackerConnectionsMismatchPenalty)
                .tooltip("此参数越大，攻击包含更多连接的细胞就越困难。"),
            parameters.baseValues.cellFunctionAttackerConnectionsMismatchPenalty);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("颜色不均匀因子")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(2.0f)
                .defaultValue(origParameters.cellFunctionAttackerColorInhomogeneityFactor)
                .tooltip("如果被攻击的细胞连接到不同颜色的细胞，此因子会影响捕获能量的能量。"),
            parameters.cellFunctionAttackerColorInhomogeneityFactor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("能量分配半径")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(5.0f)
                .defaultValue(origParameters.cellFunctionAttackerEnergyDistributionRadius)
                .tooltip("攻击细胞在攻击期间将其捕获的能量传递给附近发送器或构建细胞的最大距离。"),
            parameters.cellFunctionAttackerEnergyDistributionRadius);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("能量分配值")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0)
                .max(20.0f)
                .defaultValue(origParameters.cellFunctionAttackerEnergyDistributionValue)
                .tooltip("攻击细胞可以传递给附近发送器、构建细胞或连接细胞的能量量。"),
            parameters.cellFunctionAttackerEnergyDistributionValue);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Cell color transition rules
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：细胞颜色转换规则")
                                      .visible(parameters.features.cellColorTransitionRules)
                                      .blinkWhenActivated(true))) {
        for (int color = 0; color < MAX_COLORS; ++color) {
            ImGui::PushID(color);
            auto widgetParameters = AlienImGui::InputColorTransitionParameters()
                                        .textWidth(RightColumnWidth)
                                        .color(color)
                                        .defaultTargetColor(origParameters.baseValues.cellColorTransitionTargetColor[color])
                                        .defaultTransitionAge(origParameters.baseValues.cellColorTransitionDuration[color])
                                        .logarithmic(true)
                                        .infinity(true);
            if (0 == color) {
                widgetParameters.name("目标颜色和持续时间")
                    .tooltip("可以定义描述细胞颜色如何随时间变化的规则。为此，可以为每种细胞颜色定义一种后续颜色。此外，还必须指定持续时间，"
                             "定义相应的颜色保持多少个时间步。");
            }
            AlienImGui::InputColorTransition(
                widgetParameters, color, parameters.baseValues.cellColorTransitionTargetColor[color], parameters.baseValues.cellColorTransitionDuration[color]);
            ImGui::PopID();
        }
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Cell age limiter
     */
    if (AlienImGui::BeginTreeNode(
            AlienImGui::TreeNodeParameters().name("专家设置：细胞年龄限制器").visible(parameters.features.cellAgeLimiter).blinkWhenActivated(true))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("最大非活动细胞年龄")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(1.0f)
                .max(10000000.0f)
                .format("%.0f")
                .logarithmic(true)
                .infinity(true)
                .disabledValue(parameters.baseValues.cellInactiveMaxAge)
                .defaultEnabledValue(&origParameters.cellInactiveMaxAgeActivated)
                .defaultValue(origParameters.baseValues.cellInactiveMaxAge)
                .tooltip("在这里，可以为功能或其邻居功能未被触发的细胞设置最大年龄。处于“构建中”状态的细胞不受此选项影响。"),
            parameters.baseValues.cellInactiveMaxAge,
            &parameters.cellInactiveMaxAgeActivated);
        AlienImGui::SliderInt(
            AlienImGui::SliderIntParameters()
                .name("最大新生细胞年龄")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(1)
                .max(10000000)
                .logarithmic(true)
                .infinity(true)
                .disabledValue(parameters.cellEmergentMaxAge)
                .defaultEnabledValue(&origParameters.cellEmergentMaxAgeActivated)
                .defaultValue(origParameters.cellEmergentMaxAge)
                .tooltip("可以在这里设置从能量粒子中产生的细胞的最大年龄。"),
            parameters.cellEmergentMaxAge,
            &parameters.cellEmergentMaxAgeActivated);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("构建完成后重置年龄")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellResetAgeAfterActivation)
                .tooltip("如果激活此选项，则在其细胞网络构建完成后，即细胞状态从“构建中”变为“就绪”时，细胞的年龄将重置为 0。"
                         "此选项在设置了较低的“最大非活动细胞年龄”时特别有用，因为处于构建中的细胞网络是不活动的，"
                         "如果构建耗时较长，它们可能在完成后立即死亡。"),
            parameters.cellResetAgeAfterActivation);
        AlienImGui::SliderInt(
            AlienImGui::SliderIntParameters()
                .name("最大年龄平衡")
                .textWidth(RightColumnWidth)
                .logarithmic(true)
                .min(1000)
                .max(1000000)
                .disabledValue(&parameters.cellMaxAgeBalancerInterval)
                .defaultEnabledValue(&origParameters.cellMaxAgeBalancer)
                .defaultValue(&origParameters.cellMaxAgeBalancerInterval)
                .tooltip("定期调整最大年龄。它增加复制体最少的细胞颜色的最大年龄。"
                         "反之，复制体最多的细胞颜色的最大年龄会降低。"),
            &parameters.cellMaxAgeBalancerInterval,
            &parameters.cellMaxAgeBalancer);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Cell glow
     */
    if (AlienImGui::BeginTreeNode(
            AlienImGui::TreeNodeParameters().name("专家设置：细胞发光").visible(parameters.features.cellGlow).blinkWhenActivated(true))) {
        AlienImGui::Switcher(
            AlienImGui::SwitcherParameters()
                .name("着色")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.cellGlowColoring)
                .values(
                    {"能量",
                     "标准细胞颜色",
                     "突变体",
                     "突变体和细胞功能",
                     "细胞状态",
                     "基因组复杂度",
                     "单个细胞功能",
                     "所有细胞功能"})
                .tooltip(Const::ColoringParameterTooltip),
            parameters.cellGlowColoring);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("半径")
                .textWidth(RightColumnWidth)
                .min(1.0f)
                .max(8.0f)
                .defaultValue(&origParameters.cellGlowRadius)
                .tooltip("发光的半径。请注意，较大的半径会影响性能。"),
            &parameters.cellGlowRadius);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("强度")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1.0f)
                .defaultValue(&origParameters.cellGlowStrength)
                .tooltip("发光的强度。"),
            &parameters.cellGlowStrength);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Customize deletion mutations
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：自定义删除变异")
                                      .visible(parameters.features.customizeDeletionMutations)
                                      .blinkWhenActivated(true))) {
        AlienImGui::SliderInt(
            AlienImGui::SliderIntParameters()
                .name("最小尺寸")
                .textWidth(RightColumnWidth)
                .min(0)
                .max(1000)
                .logarithmic(true)
                .defaultValue(&origParameters.cellCopyMutationDeletionMinSize)
                .tooltip("这里确定了删除变异可以产生的基因组（基于编码细胞）的最小尺寸。默认值为 0。"),
            &parameters.cellCopyMutationDeletionMinSize);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Customize neuron mutations
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：自定义神经元变异")
                                      .visible(parameters.features.customizeNeuronMutations)
                                      .blinkWhenActivated(true))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("受影响的权重")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.3f")
                .defaultValue(&origParameters.cellCopyMutationNeuronDataWeight)
                .tooltip("在一次神经元变异中，细胞的神经网络中发生变化的权重比例。默认值为 0.2。"),
            &parameters.cellCopyMutationNeuronDataWeight);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("受影响的偏置")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.3f")
                .defaultValue(&origParameters.cellCopyMutationNeuronDataBias)
                .tooltip("在一次神经元变异中，细胞的神经网络中发生变化的偏置比例。默认值为 0.2。"),
            &parameters.cellCopyMutationNeuronDataBias);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("受影响的激活函数")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(1.0f)
                .format("%.3f")
                .defaultValue(&origParameters.cellCopyMutationNeuronDataActivationFunction)
                .tooltip("在一次神经元变异中，细胞的神经网络中发生变化的激活函数比例。默认值为 0.05。"),
            &parameters.cellCopyMutationNeuronDataActivationFunction);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("增强因子")
                .textWidth(RightColumnWidth)
                .min(1.0f)
                .max(1.2f)
                .format("%.3f")
                .defaultValue(&origParameters.cellCopyMutationNeuronDataReinforcement)
                .tooltip("如果变异调整了神经网络的权重或偏置，它可以被增强、减弱或通过偏移量移动。"
                         "这里定义了用于增强的因子。默认值为 1.05。"),
            &parameters.cellCopyMutationNeuronDataReinforcement);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("阻尼因子")
                .textWidth(RightColumnWidth)
                .min(1.0f)
                .max(1.2f)
                .format("%.3f")
                .defaultValue(&origParameters.cellCopyMutationNeuronDataDamping)
                .tooltip("如果变异调整了神经网络的权重或偏置，它可以被增强、减弱或通过偏移量移动。"
                         "这里定义了用于减弱的因子。默认值为 1.05。"),
            &parameters.cellCopyMutationNeuronDataDamping);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("偏移量")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(0.2f)
                .format("%.3f")
                .defaultValue(&origParameters.cellCopyMutationNeuronDataOffset)
                .tooltip("如果变异调整了神经网络的权重或偏置，它可以被增强、减弱或通过偏移量移动。"
                         "这里定义了用于偏移量的值。默认值为 0.05。"),
            &parameters.cellCopyMutationNeuronDataOffset);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: External energy control
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：外部能量控制")
                                      .visible(parameters.features.externalEnergyControl)
                                      .blinkWhenActivated(true))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("外部能量总量")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(100000000.0f)
                .format("%.0f")
                .logarithmic(true)
                .infinity(true)
                .defaultValue(&origParameters.externalEnergy)
                .tooltip("该参数可用于设置外部能量池的能量总量。然后可以以一定的速率（参见流入设置）将这种能量传递给所有构建细胞。"
                         "\n\n提示：按住 CTRL 点击滑块即可显式输入数值。\n\n警告：过多的外部能量可能导致大量细胞产生，"
                         "并使模拟变慢甚至崩溃。"),
            &parameters.externalEnergy);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("流入")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(1.0f)
                .format("%.5f")
                .logarithmic(true)
                .defaultValue(origParameters.externalEnergyInflowFactor)
                .tooltip("这里可以指定传递给构建细胞的能量比例。\n\n例如，值为 0.05 意味着每次构建细胞尝试构建新细胞时，"
                         "所需能量的 5% 会从外部能量源免费转移。"),
            parameters.externalEnergyInflowFactor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("条件流入")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.00f)
                .max(1.0f)
                .format("%.5f")
                .defaultValue(origParameters.externalEnergyConditionalInflowFactor)
                .tooltip("这里可以指定如果构建细胞能够提供构建过程的剩余能量，则转移给它们的能量比例。"
                         "\n\n例如，值为 0.6 意味着构建细胞从外部能量源免费获得构建新细胞所需能量的 60%。"
                         "但它必须自行提供所需能量的 40%。否则不会转移任何能量。"),
            parameters.externalEnergyConditionalInflowFactor);
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("仅对非复制体流入")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.externalEnergyInflowOnlyForNonSelfReplicators)
                .tooltip("如果激活，外部能量只能传递给不是自我复制体的构建细胞。"
                         "此选项可用于促进额外身体部位的进化。"),
            parameters.externalEnergyInflowOnlyForNonSelfReplicators);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("回流")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(1.0f)
                .defaultValue(origParameters.externalEnergyBackflowFactor)
                .tooltip("从模拟回流到外部能量池的能量比例。每次细胞失去能量或死亡时，其一部分能量将被提取。"
                         "剩余的能量比例留在模拟中，用于创建新的能量粒子。"),
            parameters.externalEnergyBackflowFactor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("回流限制")
                .textWidth(RightColumnWidth)
                .min(0.0f)
                .max(100000000.0f)
                .format("%.0f")
                .logarithmic(true)
                .infinity(true)
                .defaultValue(&origParameters.externalEnergyBackflowLimit)
                .tooltip("只要外部能量总量低于此值，模拟中的能量才能回流到外部能量池。"),
            &parameters.externalEnergyBackflowLimit);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Genome complexity measurement
     */
    if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                      .name("专家设置：基因组复杂度测量")
                                      .visible(parameters.features.genomeComplexityMeasurement)
                                      .blinkWhenActivated(true))) {
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("尺寸因子")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(1.0f)
                .format("%.2f")
                .defaultValue(origParameters.genomeComplexitySizeFactor)
                .tooltip("该参数控制基因组中编码的细胞数量如何影响其复杂度的计算。"),
            parameters.genomeComplexitySizeFactor);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("分支因子")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(20.0f)
                .format("%.2f")
                .defaultValue(origParameters.genomeComplexityRamificationFactor)
                .tooltip("通过此参数，细胞结构到基因组的分支数量被计入基因组复杂度的计算中。例如，包含许多子基因组或许多构建分支的基因组"
                         "将具有较高的复杂度值。"),
            parameters.genomeComplexityRamificationFactor);
        AlienImGui::SliderInt(
            AlienImGui::SliderIntParameters()
                .name("深度级别")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(1)
                .max(20)
                .infinity(true)
                .defaultValue(origParameters.genomeComplexityDepthLevel)
                .tooltip("这允许指定复杂度计算应在子基因组的哪个级别进行。例如，值为 2 意味着除了主基因组之外，"
                         "还考虑子基因组和子-子基因组。"),
            parameters.genomeComplexityDepthLevel);
        AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("神经元因子")
                .textWidth(RightColumnWidth)
                .colorDependence(true)
                .min(0.0f)
                .max(4.0f)
                .format("%.2f")
                .defaultValue(origParameters.genomeComplexityNeuronFactor)
                .tooltip("该参数将基因组中编码的神经元数量计入复杂度值。"),
            parameters.genomeComplexityNeuronFactor);
    }
    AlienImGui::EndTreeNode();

    /**
     * Expert settings: Legacy behavior
     */
    if (AlienImGui::BeginTreeNode(
            AlienImGui::TreeNodeParameters().name("专家设置：旧版行为").visible(parameters.features.legacyModes).blinkWhenActivated(true))) {
        AlienImGui::Checkbox(
            AlienImGui::CheckboxParameters()
                .name("从相邻传感器获取角度")
                .textWidth(RightColumnWidth)
                .defaultValue(origParameters.legacyCellFunctionMuscleMovementAngleFromSensor)
                .tooltip("此参数改变“向目标移动”参数的行为。如果激活，肌肉细胞直接从先前检测到目标的连接（或连接-连接）传感器细胞获取移动角度"
                         "（旧版行为）。如果停用，输入信号必须仅来自传感器细胞且不得相邻（新行为）。"),
            parameters.legacyCellFunctionMuscleMovementAngleFromSensor);
    }
    AlienImGui::EndTreeNode();

    SimulationParametersValidationService::get().validateAndCorrect(parameters);

    if (parameters != lastParameters) {
        _simulationFacade->setSimulationParameters(parameters, SimulationParametersUpdateConfig::AllExceptChangingPositions);
    }
}

std::string _SimulationParametersBaseWidgets::getLocationName()
{
    return "“基础”的模拟器参数";
}

int _SimulationParametersBaseWidgets::getLocationIndex() const
{
    return 0;
}

void _SimulationParametersBaseWidgets::setLocationIndex(int locationIndex)
{
    // do nothing
}
