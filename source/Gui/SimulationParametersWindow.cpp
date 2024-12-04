#include "SimulationParametersWindow.h"

#include <ImFileDialog.h>
#include <imgui.h>
#include <Fonts/IconsFontAwesome5.h>

#include "Base/GlobalSettings.h"
#include "EngineInterface/SimulationFacade.h"
#include "EngineInterface/SimulationParametersValidationService.h"
#include "PersisterInterface/SerializerService.h"

#include "AlienImGui.h"
#include "CellFunctionStrings.h"
#include "GenericFileDialog.h"
#include "HelpStrings.h"
#include "GenericMessageDialog.h"
#include "SimulationInteractionController.h"
#include "RadiationSourcesWindow.h"
#include "OverlayController.h"
#include "StyleRepository.h"

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

    template<int numElements>
    std::vector<float> toVector(float const v[numElements])
    {
        std::vector<float> result;
        for (int i = 0; i < numElements; ++i) {
            result.emplace_back(v[i]);
        }
        return result;
    }
}

void SimulationParametersWindow::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;

    for (int n = 0; n < IM_ARRAYSIZE(_savedPalette); n++) {
        ImVec4 color;
        ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.2f, color.x, color.y, color.z);
        color.w = 1.0f; //alpha
        _savedPalette[n] = static_cast<ImU32>(ImColor(color));
    }

    auto path = std::filesystem::current_path();
    if (path.has_parent_path()) {
        path = path.parent_path();
    }
    _startingPath = GlobalSettings::get().getValue("windows.simulation parameters.starting path", path.string());
    _featureListOpen = GlobalSettings::get().getValue("windows.simulation parameters.feature list.open", _featureListOpen);
    _featureListHeight = GlobalSettings::get().getValue("windows.simulation parameters.feature list.height", _featureListHeight);

    for (int i = 0; i < CellFunction_Count; ++i) {
        _cellFunctionStrings.emplace_back(Const::CellFunctionToStringMap.at(i));
    }

}

SimulationParametersWindow::SimulationParametersWindow()
    : AlienWindow("模拟器参数集", "windows.simulation parameters", false)
{}

void SimulationParametersWindow::shutdownIntern()
{
    GlobalSettings::get().setValue("windows.simulation parameters.starting path", _startingPath);
    GlobalSettings::get().setValue("windows.simulation parameters.feature list.open", _featureListOpen);
    GlobalSettings::get().setValue("windows.simulation parameters.feature list.height", _featureListHeight);
}

void SimulationParametersWindow::processIntern()
{
    processToolbar();

    if (ImGui::BeginChild("##parameterchild1", {0, -scale(44.0f)})) {
        if (ImGui::BeginChild("##parameterchild2", {0, _featureListOpen ? -scale(_featureListHeight) : -scale(50.0f)})) {
            processTabWidget();
        }
        ImGui::EndChild();
        processAddonList();
    }
    ImGui::EndChild();

    processStatusBar();
}

SimulationParametersSpot SimulationParametersWindow::createSpot(SimulationParameters const& simParameters, int index)
{
    auto worldSize = _simulationFacade->getWorldSize();
    SimulationParametersSpot spot;
    spot.posX = toFloat(worldSize.x / 2);
    spot.posY = toFloat(worldSize.y / 2);

    auto maxRadius = toFloat(std::min(worldSize.x, worldSize.y)) / 2;
    spot.shapeType = SpotShapeType_Circular;
    createDefaultSpotData(spot);
    spot.fadeoutRadius = maxRadius / 3;
    spot.color = _savedPalette[((2 + index) * 8) % IM_ARRAYSIZE(_savedPalette)];

    spot.values = simParameters.baseValues;
    return spot;
}

void SimulationParametersWindow::createDefaultSpotData(SimulationParametersSpot& spot)
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

void SimulationParametersWindow::processToolbar()
{
    if (AlienImGui::ToolbarButton(ICON_FA_FOLDER_OPEN)) {
        onOpenParameters();
    }
    AlienImGui::Tooltip("从文件中获取模拟器参数集");

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(ICON_FA_SAVE)) {
        onSaveParameters();
    }
    AlienImGui::Tooltip("保存模拟器参数集至文件");

    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(ICON_FA_COPY)) {
        _copiedParameters = _simulationFacade->getSimulationParameters();
        printOverlayMessage("模拟器参数集已复制");
    }
    AlienImGui::Tooltip("复制模拟器参数集");

    ImGui::SameLine();
    ImGui::BeginDisabled(!_copiedParameters);
    if (AlienImGui::ToolbarButton(ICON_FA_PASTE)) {
        _simulationFacade->setSimulationParameters(*_copiedParameters);
        _simulationFacade->setOriginalSimulationParameters(*_copiedParameters);
        printOverlayMessage("模拟器参数集已粘贴");
    }
    ImGui::EndDisabled();
    AlienImGui::Tooltip("粘贴模拟器参数集");

    AlienImGui::Separator();
}

void SimulationParametersWindow::processTabWidget()
{
    auto currentSessionId = _simulationFacade->getSessionId();

    std::optional<bool> scheduleAppendTab;
    std::optional<int> scheduleDeleteTabAtIndex;
    
    if (ImGui::BeginChild("##", ImVec2(0, 0), false)) {

        if (ImGui::BeginTabBar("##Parameters", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyResizeDown)) {
            auto parameters = _simulationFacade->getSimulationParameters();

            //add spot
            if (parameters.numSpots < MAX_SPOTS) {
                if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
                    scheduleAppendTab = true;
                }
                AlienImGui::Tooltip("增加特别参数区");
            }

            processBase();

            for (int tab = 0; tab < parameters.numSpots; ++tab) {
                if (!processSpot(tab)) {
                    scheduleDeleteTabAtIndex = tab;
                }
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();

    _focusBaseTab = !_sessionId.has_value() || currentSessionId != *_sessionId;
    _sessionId= currentSessionId;

    if (scheduleAppendTab.has_value()) {
        onAppendTab();
    }
    if (scheduleDeleteTabAtIndex.has_value()) {
        onDeleteTab(scheduleDeleteTabAtIndex.value());
    }
}

void SimulationParametersWindow::processBase()
{
    if (ImGui::BeginTabItem("基础", nullptr, _focusBaseTab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)) {
        auto parameters = _simulationFacade->getSimulationParameters();
        auto origParameters = _simulationFacade->getOriginalSimulationParameters();
        auto lastParameters = parameters;

        if (ImGui::BeginChild("##", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {

            /**
             * General
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("通用"))) {
                AlienImGui::InputText(
                    AlienImGui::InputTextParameters()
                        .name("模拟器名称")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.projectName),
                    parameters.projectName,
                    sizeof(Char64) / sizeof(char));

                AlienImGui::EndTreeNode();
            }
            /**
             * Rendering
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("可视化"))) {
                AlienImGui::ColorButtonWithPicker(
                    AlienImGui::ColorButtonWithPickerParameters().name("背景颜色").textWidth(RightColumnWidth).defaultValue(origParameters.backgroundColor),
                    parameters.backgroundColor,
                    _backupColor,
                    _savedPalette);
                AlienImGui::Switcher(
                    AlienImGui::SwitcherParameters()
                        .name("初步细胞染色设置")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.cellColoring)
                        .values(
                            {"能量程度",
                             "标准细胞颜色",
                             "基因代际",
                             "基因代际和细胞功能",
                             "细胞状态",
                             "基因组复杂度",
                             "单一细胞功能",
                             "所有细胞功能"})
                        .tooltip(Const::ColoringParameterTooltip),
                    parameters.cellColoring);
                if (parameters.cellColoring == CellColoring_CellFunction) {
                    AlienImGui::Switcher(
                        AlienImGui::SwitcherParameters()
                            .name("高光细胞功能")
                            .textWidth(RightColumnWidth)
                            .defaultValue(origParameters.highlightedCellFunction)
                            .values(_cellFunctionStrings)
                            .tooltip("在此所指定的细胞功能将被高光染色"),
                        parameters.highlightedCellFunction);
                }
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("细胞染色范围")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(0.5f)
                        .defaultValue(&origParameters.cellRadius)
                        .tooltip("指定一个单位的细胞将被染色的半径"),
                    &parameters.cellRadius);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("可视细胞活动的缩放水平")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(32.0f)
                        .infinity(true)
                        .defaultValue(&origParameters.zoomLevelNeuronalActivity)
                        .tooltip("可观察到生物神经活动的缩放水平"),
                    &parameters.zoomLevelNeuronalActivity);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("攻击可视化")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.attackVisualization)
                        .tooltip("如果启用，攻击器细胞发出的成功攻击将可视化。"),
                    parameters.attackVisualization);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("肌肉运动可视化")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.muscleMovementVisualization)
                        .tooltip("如果启用，肌肉细胞运动的方向将可视化."),
                    parameters.muscleMovementVisualization);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("无边际渲染")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.borderlessRendering)
                        .tooltip("如果启用，模拟器将在视口中无限重复渲染。"),
                    parameters.borderlessRendering);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("自动空间网格划分")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.gridLines)
                        .tooltip("根据缩放水平在背景中绘制合适大小的网格。"),
                    parameters.gridLines);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("标记实际模拟器区域")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.markReferenceDomain)
                        .tooltip("绘制包围世界的边界，在启用了无边际渲染的情况下好用。"),
                    parameters.markReferenceDomain);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("显示放射性粒子源")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.showRadiationSources)
                        .tooltip("在放射性粒子源的中心绘制红十字以标记。"),
                    parameters.showRadiationSources);
                AlienImGui::EndTreeNode();
            }

            /**
             * Numerics
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("数值"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("时序大小")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(1.0f)
                        .defaultValue(&origParameters.timestepSize)
                        .tooltip(std::string("在单次模拟步骤中计算的时间持续长度。较小的值可以提高模拟的精确度，而较大的值可能导致数值不稳定。")),
                    &parameters.timestepSize);
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Motion
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：运动"))) {
                if (AlienImGui::Switcher(
                        AlienImGui::SwitcherParameters()
                            .name("运动类型")
                            .textWidth(RightColumnWidth)
                            .defaultValue(origParameters.motionType)
                            .values({"流体动力", "基于碰撞"})
                            .tooltip(std::string(
                                "粒子运动的算法在这里定义。如果选择‘流体动力学’，则使用SPH流体求解器来计算力。此时，粒子会表现得像（可压缩的）液体或气体。另一个选项‘基于碰撞’则基于粒子碰撞计算力，适用于固体的机械模拟。")),
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
                            .name("平滑尺度")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(3.0f)
                            .defaultValue(&origParameters.motionData.fluidMotion.smoothingLength)
                            .tooltip(std::string("平滑长度决定了在计算密度、压力和粘度时邻近粒子的影响区域。过小的值会导致数值不稳定，而过大的值会导致粒子相互分离。")),
                        &parameters.motionData.fluidMotion.smoothingLength);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("压力")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(0.3f)
                            .defaultValue(&origParameters.motionData.fluidMotion.pressureStrength)
                            .tooltip(std::string("该参数可以控制压力的大小。")),
                        &parameters.motionData.fluidMotion.pressureStrength);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("粘度")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(0.3f)
                            .defaultValue(&origParameters.motionData.fluidMotion.viscosityStrength)
                            .tooltip(std::string("该参数可以用来控制粘度的大小。 更大的值导致更平滑的运动。")),
                        &parameters.motionData.fluidMotion.viscosityStrength);
                } else {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("排斥强度")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(0.3f)
                            .defaultValue(&origParameters.motionData.collisionMotion.cellRepulsionStrength)
                            .tooltip(std::string("该参数可以用来控制未连接的两个细胞之间排斥力的强度")),
                        &parameters.motionData.collisionMotion.cellRepulsionStrength);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("最大碰撞距离")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(3.0f)
                            .defaultValue(&origParameters.motionData.collisionMotion.cellMaxCollisionDistance)
                            .tooltip(std::string("该参数可以用来控制两个细胞可能发生碰撞的最大距离。")),
                        &parameters.motionData.collisionMotion.cellMaxCollisionDistance);
                }
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("摩擦力")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(1.0f)
                        .logarithmic(true)
                        .format("%.4f")
                        .defaultValue(&origParameters.baseValues.friction)
                        .tooltip(std::string("该参数可以用来控制每个步数中物体速度被减慢的比例。T")),
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
                            "该参数可以用来控制单个生物连接的细胞的整体刚性。\n较高的值将使连接的单元更像一个刚体那样更均匀地移动。")),
                    &parameters.baseValues.rigidity);
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Thresholds
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：阈值"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最大速度")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(6.0f)
                        .defaultValue(&origParameters.cellMaxVelocity)
                        .tooltip(std::string("一个细胞能达到的最大速度")),
                    &parameters.cellMaxVelocity);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最大力度")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(3.0f)
                        .colorDependence(true)
                        .defaultValue(origParameters.baseValues.cellMaxForce)
                        .tooltip(std::string("在不导致细胞解体的情况下可以施加于细胞的最大力。")),
                    parameters.baseValues.cellMaxForce);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最小距离")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(1.0f)
                        .defaultValue(&origParameters.cellMinDistance)
                        .tooltip(std::string("两个细胞之间的最小距离")),
                    &parameters.cellMinDistance);
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Binding
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：连接"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最大距离")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(5.0f)
                        .colorDependence(true)
                        .defaultValue(origParameters.cellMaxBindingDistance)
                        .tooltip(std::string("两个细胞连接仍然有效的最大距离。")),
                    parameters.cellMaxBindingDistance);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("连接速度")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(2.0f)
                        .defaultValue(&origParameters.baseValues.cellFusionVelocity)
                        .tooltip(std::string("两个碰撞细胞之间能够建立连接的最小相对速度。")),
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
                        .tooltip(std::string("细胞能够维持与相邻细胞结合的最大能量。如果细胞的能量超过这个值，所有连接将会被破坏。")),
                    &parameters.baseValues.cellMaxBindingEnergy);
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Radiation
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：放射性粒子"))) {
                if (AlienImGui::Button(AlienImGui::ButtonParameters()
                                           .buttonText("打开编辑器")
                                           .name("放射性粒子源")
                                           .textWidth(RightColumnWidth)
                                           .showDisabledRevertButton(true)
                            .tooltip("如果没有指定辐射源，细胞会在各自的位置释放能量粒子。另一方面，如果定义了一个或多个辐射源，细胞释放的能量粒子将在这些辐射源处产生。"))) {
                    RadiationSourcesWindow::get().setOn(true);
                }

                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("吸收率")
                        .textWidth(RightColumnWidth)
                        .logarithmic(true)
                        .colorDependence(true)
                        .min(0)
                        .max(1.0)
                        .format("%.4f")
                        .defaultValue(origParameters.baseValues.radiationAbsorption)
                        .tooltip("可以在此指定细胞从入射能量粒子中吸收的能量比例。"),
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
                        .tooltip("控制老化细胞释放的粒子的能量大小。"),
                    parameters.baseValues.radiationCellAgeStrength);
                AlienImGui::SliderInt(
                    AlienImGui::SliderIntParameters()
                        .name("辐射类型 I： 最小年龄")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .infinity(true)
                        .min(0)
                        .max(10000000)
                        .logarithmic(true)
                        .defaultValue(origParameters.radiationMinCellAge)
                        .tooltip("可以在此定义细胞开始发射能量粒子的最小年龄。"),
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
                        .tooltip("控制高能量细胞释放的粒子的能量大小。"),
                    parameters.highRadiationFactor);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("辐射类型 II：最小阈值")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .infinity(true)
                        .min(0)
                        .max(100000.0f)
                        .logarithmic(true)
                        .format("%.1f")
                        .defaultValue(origParameters.highRadiationMinCellEnergy)
                        .tooltip("可以在此定义细胞开始释放能量粒子的最小能量。"),
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
                        .tooltip("能量粒子达到某一最低能量后可以分裂成两个粒子，并获得一个小的动量。分裂不会立即发生，而是在经过一定时间后才会发生。"),
                    parameters.particleSplitEnergy);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("能量是否可以向细胞转化")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.particleTransformationAllowed)
                        .tooltip("如果启用，一个能量粒子将在其能量值超过一般水平时转化为细胞。"),
                    parameters.particleTransformationAllowed);

                AlienImGui::EndTreeNode();
            }

            /**
             * Cell life cycle
             */
            ImGui::PushID("Transformation");
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞生命周期"))) {
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
                        .tooltip("定义细胞的最大年龄。如果一个细胞超过了最大年龄，它将被转化为能量粒子。"),
                    parameters.cellMaxAge);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最小能量")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(10.0f)
                        .max(200.0f)
                        .defaultValue(origParameters.baseValues.cellMinEnergy)
                        .tooltip("一个细胞存在所需的最小能量。"),
                    parameters.baseValues.cellMinEnergy);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("一般能量")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(10.0f)
                        .max(200.0f)
                        .defaultValue(origParameters.cellNormalEnergy)
                        .tooltip("这里定义了细胞的一般能量值。在多种情况下，这个值被用作参考值：\n\n" ICON_FA_CHEVRON_RIGHT
            " 攻击者和传输者细胞：当这些细胞的能量高于一般值时，它们的一部分能量会分配给周围的细胞。\n\n" ICON_FA_CHEVRON_RIGHT
            " 建造者细胞：创建新细胞需要消耗能量。只有当建造者细胞的剩余能量不低于一般值时，才会执行新细胞的创建。\n\n" ICON_FA_CHEVRON_RIGHT
            " 如果激活了能量粒子向细胞的转化，当粒子的能量超过一般值时，能量粒子将转化为细胞。"),
    parameters.cellNormalEnergy);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("正在死亡的细胞的腐败率")
                        .colorDependence(true)
                        .textWidth(RightColumnWidth)
                        .min(1e-6f)
                        .max(0.1f)
                        .format("%.6f")
                        .logarithmic(true)
                        .defaultValue(origParameters.baseValues.cellDeathProbability)
                        .tooltip("在一个细胞处于“濒死”状态时，每个时间步细胞解体（即转化为能量粒子）的概率。这可能在满足以下条件之一时发生：\n\n"
                                ICON_FA_CHEVRON_RIGHT " 细胞的能量过低。\n\n"
                                ICON_FA_CHEVRON_RIGHT " 细胞超过了其最大年龄。"),parameters.baseValues.cellDeathProbability);
    AlienImGui::Switcher(
        AlienImGui::SwitcherParameters()
        .name("细胞死亡判定")
        .textWidth(RightColumnWidth)
        .defaultValue(origParameters.cellDeathConsequences)
        .values({"无", "整个生物体死亡", "分离的生物体部分死亡"})
        .tooltip("在这里可以定义当某个细胞处于“濒死”状态时，生物体会发生什么。\n\n" ICON_FA_CHEVRON_RIGHT
            " 无：只有该细胞死亡。\n\n" ICON_FA_CHEVRON_RIGHT " 整个生物体死亡：生物体的所有细胞也将死亡。\n\n" ICON_FA_CHEVRON_RIGHT
            " 分离的生物体部分死亡：只有不再连接到自我复制的建造者细胞的生物体部分会死亡。"),parameters.cellDeathConsequences);
                AlienImGui::EndTreeNode();
            }
            ImGui::PopID();

            /**
             * Mutation 
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("基因组复制时的突变"))) {
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
                        .tooltip("这种类型的突变会改变基因组中编码的单个神经元细胞的神经网络的权重或偏置。变化的概率由指定值乘以基因组中编码细胞的数量决定。"),
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
                        .tooltip("这种类型的突变会改变一个随机属性（例如，（输入）执行顺序号、所需能量、块输出以及特定功能的属性，如传感器的最小密度、神经网络权重等）。空间结构、颜色、细胞功能类型和自我复制能力不会改变。变化的概率由指定值乘以基因组中编码细胞的数量决定。"),
                    parameters.baseValues.cellCopyMutationCellProperties);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("几何形状")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origParameters.baseValues.cellCopyMutationGeometry)
                        .tooltip("这种类型的突变仅改变自定义几何体的角度和所需连接。变化的概率由指定值乘以基因组中编码细胞的数量决定。"),
                    parameters.baseValues.cellCopyMutationGeometry);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("自定义几何形状")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origParameters.baseValues.cellCopyMutationCustomGeometry)
                        .tooltip("这种类型的突变仅改变自定义几何体的角度和所需连接。变化的概率由指定值乘以基因组中编码细胞的数量决定。"),
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
                        .tooltip("这种类型的突变改变了细胞功能的类型。改变后的细胞功能将具有随机属性。变化的概率由指定值乘以基因组中编码细胞的数量决定。如果“保留自我复制”标志被禁用，它还可以通过将构造函数更改为其他内容或反之来改变自我复制能力。"),
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
                        .tooltip("这种类型的突变在基因组的随机位置插入一个新的细胞描述。变化的概率由指定值乘以基因组中编码细胞的数量决定。"),
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
                        .tooltip("这种类型的突变会从基因组的随机位置删除一个细胞描述。变化的概率由指定值乘以基因组中编码细胞的数量决定。"),
                    parameters.baseValues.cellCopyMutationDeletion);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("变换")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origParameters.baseValues.cellCopyMutationTranslation)
                        .tooltip("这种类型的突变会将一块细胞描述从基因组的一个随机位置移动到另一个新的随机位置。"),
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
                        .tooltip("这种类型的突变会将一块细胞描述从基因组的一个随机位置复制到另一个新的随机位置。"),
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
                        .tooltip("这种类型的突变通过使用指定的颜色转换来改变基因组中单个细胞描述的颜色。变化的概率由指定值乘以基因组中编码细胞的数量决定。"),
                    parameters.baseValues.cellCopyMutationCellColor);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("次级基因组颜色")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origParameters.baseValues.cellCopyMutationSubgenomeColor)
                        .tooltip("这种类型的突变通过使用指定的颜色转换来改变次级基因组中所有细胞描述的颜色。"),
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
                        .tooltip(
                            "这种类型的突变通过使用指定的颜色转换来改变基因组中所有细胞描述的颜色。"),
                    parameters.baseValues.cellCopyMutationGenomeColor);
                AlienImGui::CheckboxColorMatrix(
                    AlienImGui::CheckboxColorMatrixParameters()
                        .name("颜色转换")
                        .textWidth(RightColumnWidth)
                        .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.cellFunctionConstructorMutationColorTransitions))
                        .tooltip(
                            "颜色转换用于颜色突变。行索引表示源颜色，列索引表示目标颜色。"),
                    parameters.cellFunctionConstructorMutationColorTransitions);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("防止基因组深度增加")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.cellFunctionConstructorMutationPreventDepthIncrease)
                        .tooltip(std::string("基因组具有树状结构，因为它可以包含子基因组。如果激活此标志，突变将不会增加基因组结构的深度。")),
                    parameters.cellFunctionConstructorMutationPreventDepthIncrease);
                auto preserveSelfReplication = !parameters.cellFunctionConstructorMutationSelfReplication;
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("保留自我复制")
                        .textWidth(RightColumnWidth)
                        .defaultValue(!origParameters.cellFunctionConstructorMutationSelfReplication)
                        .tooltip("如果未激活，突变也可以通过将构造细胞更改为其他细胞或反之来改变基因组中的自我复制能力。"),
                    preserveSelfReplication);
                parameters.cellFunctionConstructorMutationSelfReplication = !preserveSelfReplication;
                AlienImGui::EndTreeNode();
            }

            /**
             * Attacker
             */
            ImGui::PushID("Attacker");
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：攻击器"))) {
                AlienImGui::InputFloatColorMatrix(
                    AlienImGui::InputFloatColorMatrixParameters()
                        .name("食物链颜色矩阵")
                        .max(1)
                        .textWidth(RightColumnWidth)
                        .tooltip(
                            "该矩阵可用于确定一个细胞攻击另一个细胞的效果。攻击细胞的颜色对应行号，被攻击细胞的颜色对应列号。值为0表示被攻击细胞不能被消化，即不能获得能量。值为1表示在消化过程中可以获得最大能量。\n例如：如果在第2行（红色）和第3列（绿色）中输入0，则表示红色细胞不能吃绿色细胞。")
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
                        .tooltip("表示成功攻击细胞后削弱的能量部分。然而，这部分能量可以受到攻击者模拟参数中可调节的其他因素的影响。"),
                    parameters.cellFunctionAttackerStrength);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("攻击范围")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(3.0f)
                        .defaultValue(origParameters.cellFunctionAttackerRadius)
                        .tooltip("一个攻击器细胞攻击其他细胞的最大范围。"),
                    parameters.cellFunctionAttackerRadius);
                AlienImGui::InputFloatColorMatrix(
                    AlienImGui::InputFloatColorMatrixParameters()
                        .name("复杂生物保护")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(20.0f)
                        .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.baseValues.cellFunctionAttackerGenomeComplexityBonus))
                        .tooltip("该参数越大，通过攻击具有更复杂基因组的生物所获得的能量就越少。"),
                    parameters.baseValues.cellFunctionAttackerGenomeComplexityBonus);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("能量损耗")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(1.0f)
                        .format("%.5f")
                        .logarithmic(true)
                        .defaultValue(origParameters.baseValues.cellFunctionAttackerEnergyCost)
                        .tooltip("尝试攻击细胞时以能量粒子形式损失的能量大小。"),
                    parameters.baseValues.cellFunctionAttackerEnergyCost);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("毁灭细胞")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.cellFunctionAttackerDestroyCells)
                        .tooltip(
                            "如果启用，攻击细胞可以毁灭细胞。如果关闭，他只能伤害他们。"),
                    parameters.cellFunctionAttackerDestroyCells);
                AlienImGui::EndTreeNode();
            }
            ImGui::PopID();

            /**
             * Constructor
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：构筑器"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("连接距离")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0.1f)
                        .max(3.0f)
                        .defaultValue(origParameters.cellFunctionConstructorConnectingCellMaxDistance)
                        .tooltip("构造器可以在此距离内自动将构造的细胞连接到附近的其他细胞。"),
                    parameters.cellFunctionConstructorConnectingCellMaxDistance);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("完成度检查")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.cellFunctionConstructorCheckCompletenessForSelfReplication)
                        .tooltip("如果激活，自我复制过程只能在细胞网络中所有其他非自我复制构造器完成后开始。"),
                    parameters.cellFunctionConstructorCheckCompletenessForSelfReplication);
                AlienImGui::EndTreeNode();
            }

            /**
             * Defender
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：防御器"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("反攻击器强度")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(1.0f)
                        .max(5.0f)
                        .defaultValue(origParameters.cellFunctionDefenderAgainstAttackerStrength)
                        .tooltip("如果被攻击的细胞连接到防御细胞或自身是防御细胞，则攻击强度会按此因子减少。"),
                    parameters.cellFunctionDefenderAgainstAttackerStrength);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("反注射器强度")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(1.0f)
                        .max(5.0f)
                        .defaultValue(origParameters.cellFunctionDefenderAgainstInjectorStrength)
                        .tooltip("如果构造细胞被注射器攻击并且连接到防御细胞，则注射持续时间会按此因子增加。"),
                    parameters.cellFunctionDefenderAgainstInjectorStrength);
                AlienImGui::EndTreeNode();
            }

            /**
             * Injector
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：注射器"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("注射范围")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0.1f)
                        .max(4.0f)
                        .defaultValue(origParameters.cellFunctionInjectorRadius)
                        .tooltip("注射器细胞可以感染其他细胞的最大距离。"),
                    parameters.cellFunctionInjectorRadius);
                AlienImGui::InputIntColorMatrix(
                    AlienImGui::InputIntColorMatrixParameters()
                        .name("注射时间")
                        .logarithmic(true)
                        .max(100000)
                        .textWidth(RightColumnWidth)
                        .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.cellFunctionInjectorDurationColorMatrix))
                        .tooltip("注射器细胞需要激活的次数才能感染其他细胞。一次激活通常需要6个时间步长。行号决定注射器细胞的颜色，而列号对应被感染细胞的颜色。"),
                    parameters.cellFunctionInjectorDurationColorMatrix);
                AlienImGui::EndTreeNode();
            }

            /**
             * Muscle
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：运动器（肌肉）"))) {
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("是否朝向目标移动")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.cellFunctionMuscleMovementTowardTargetedObject)
                        .tooltip("如果激活，处于运动模式的肌肉细胞只有在触发信号来自已锁定目标的传感器细胞时才会移动。输入中指定的角度相对于目标进行解释。"),
                    parameters.cellFunctionMuscleMovementTowardTargetedObject);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("运动加速度")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(0.4f)
                        .logarithmic(true)
                        .defaultValue(origParameters.cellFunctionMuscleMovementAcceleration)
                        .tooltip("肌肉细胞在激活期间可以修改其速度的最大值。此参数仅适用于处于运动模式的肌肉细胞。"),
                    parameters.cellFunctionMuscleMovementAcceleration);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("收缩和扩张增量")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(0.1f)
                        .defaultValue(origParameters.cellFunctionMuscleContractionExpansionDelta)
                        .tooltip("肌肉细胞可以缩短或延长细胞连接的最大长度。此参数仅适用于处于收缩/扩张模式的肌肉细胞。"),
                    parameters.cellFunctionMuscleContractionExpansionDelta);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("弯曲角度")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(10.0f)
                        .defaultValue(origParameters.cellFunctionMuscleBendingAngle)
                        .tooltip("肌肉细胞可以增加/减少两个细胞连接之间角度的最大值。此参数仅适用于处于弯曲模式的肌肉细胞。"),
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
                AlienImGui::EndTreeNode();
            }

            /**
             * Sensor
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：感知器"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("范围")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(10.0f)
                        .max(800.0f)
                        .defaultValue(origParameters.cellFunctionSensorRange)
                        .tooltip("传感器细胞可以检测质量集中区域的最大半径。"),
                    parameters.cellFunctionSensorRange);
                AlienImGui::EndTreeNode();
            }

            /**
             * Transmitter
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：传输器"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("能量传输范围")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(5.0f)
                        .defaultValue(origParameters.cellFunctionTransmitterEnergyDistributionRadius)
                        .tooltip("传输器细胞将其额外能量传递给附近的传输器或构造器细胞的最大距离。"),
                    parameters.cellFunctionTransmitterEnergyDistributionRadius);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("能量传输值")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(20.0f)
                        .defaultValue(origParameters.cellFunctionTransmitterEnergyDistributionValue)
                        .tooltip("传输器细胞可以传递给附近的传输器或构造细胞或连接细胞的能量值。"),
                    parameters.cellFunctionTransmitterEnergyDistributionValue);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("同一生物的能量分配")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origParameters.cellFunctionTransmitterEnergyDistributionSameCreature)
                        .tooltip("如果激活，传输器细胞只能将能量传递给属于同一生物的附近细胞。"),
                    parameters.cellFunctionTransmitterEnergyDistributionSameCreature);
                AlienImGui::EndTreeNode();
            }

            /**
             * Reconnector
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：重连器"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("范围")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0.0f)
                        .max(3.0f)
                        .defaultValue(origParameters.cellFunctionReconnectorRadius)
                        .tooltip("重连器细胞可以建立或破坏与其他细胞连接的最大半径。"),
                    parameters.cellFunctionReconnectorRadius);
                AlienImGui::EndTreeNode();
            }

            /**
             * Detonator
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：爆炸器"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("爆炸半径")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0.0f)
                        .max(10.0f)
                        .defaultValue(origParameters.cellFunctionDetonatorRadius)
                        .tooltip("爆炸器爆炸的范围。"),
                    parameters.cellFunctionDetonatorRadius);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("连锁爆炸可能性")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0.0f)
                        .max(1.0f)
                        .defaultValue(origParameters.cellFunctionDetonatorChainExplosionProbability)
                        .tooltip("一个雷管爆炸会引发爆炸半径内其他雷管爆炸的概率。"),
                    parameters.cellFunctionDetonatorChainExplosionProbability);
                AlienImGui::EndTreeNode();
            }

            /**
             * Addon: Advanced absorption control
             */
            if (parameters.features.advancedAbsorptionControl) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：高级能量吸收控制"))) {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("基因组低复杂度惩罚")
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
                            .name("低细胞连接程度惩罚")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(5.0f)
                            .format("%.1f")
                            .defaultValue(origParameters.radiationAbsorptionLowConnectionPenalty)
                            .tooltip(
                                "当此参数增加时，具有较少细胞连接的细胞将从进入的能量粒子中吸收更少的能量。"),
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
                            .tooltip("当此参数增加时，快速移动的细胞将从进入的能量粒子中吸收更少的能量。"),
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
                            .tooltip("当此参数增加时，慢速移动的细胞将从进入的能量粒子中吸收更少的能量。"),
                        parameters.baseValues.radiationAbsorptionLowVelocityPenalty);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Advanced attacker control
             */
            if (parameters.features.advancedAttackerControl) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：高级攻击器控制"))) {
                    AlienImGui::InputFloatColorMatrix(
                        AlienImGui::InputFloatColorMatrixParameters()
                            .name("同种族保护")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(1.0f)
                            .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.cellFunctionAttackerSameMutantPenalty))
                            .tooltip("此参数越大，通过攻击具有相同突变ID的生物所获得的能量就越少。"),
                        parameters.cellFunctionAttackerSameMutantPenalty);
                    AlienImGui::InputFloatColorMatrix(
                        AlienImGui::InputFloatColorMatrixParameters()
                            .name("新种族保护")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(1.0f)
                            .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origParameters.baseValues.cellFunctionAttackerNewComplexMutantPenalty))
                            .tooltip("较高的数值可以保护具有相同或更高基因组复杂性的新突变体不被攻击。"),
                        parameters.baseValues.cellFunctionAttackerNewComplexMutantPenalty);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("感知器检测因素")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(1.0f)
                            .defaultValue(origParameters.cellFunctionAttackerSensorDetectionFactor)
                            .tooltip(
                                "此参数控制目标是否必须先通过传感器检测才能被攻击。此值越大，如果目标尚未被检测到，则在攻击期间获得的能量就越少。\n为此，攻击细胞会搜索连接的（或间接连接的）传感器细胞，以查看它们上次检测到的细胞网络，并将其与被攻击目标进行比较。"),
                        parameters.cellFunctionAttackerSensorDetectionFactor);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("几何形状惩罚")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(5.0f)
                            .defaultValue(origParameters.baseValues.cellFunctionAttackerGeometryDeviationExponent)
                            .tooltip("此值越大，如果被攻击细胞的局部几何形状与攻击细胞不匹配，则细胞从攻击中获得的能量就越少。"),
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
                            .name("颜色均衡因素")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(2.0f)
                            .defaultValue(origParameters.cellFunctionAttackerColorInhomogeneityFactor)
                            .tooltip("如果被攻击的细胞连接到颜色不同的细胞，此因子会影响捕获能量的能量值。"),
                        parameters.cellFunctionAttackerColorInhomogeneityFactor);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("能量传输范围")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(5.0f)
                            .defaultValue(origParameters.cellFunctionAttackerEnergyDistributionRadius)
                            .tooltip("攻击细胞在攻击期间捕获的能量传输到附近的传输细胞或构造细胞的最大距离。"),
                        parameters.cellFunctionAttackerEnergyDistributionRadius);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("能量传输值")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(20.0f)
                            .defaultValue(origParameters.cellFunctionAttackerEnergyDistributionValue)
                            .tooltip("The amount of energy which a attacker cell can transfer to nearby transmitter or constructor cells or to connected cells."),
                        parameters.cellFunctionAttackerEnergyDistributionValue);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Cell color transition rules
             */
            if (parameters.features.cellColorTransitionRules) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：细胞颜色过渡规则").highlighted(false))) {
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
                                .tooltip(
                                    "可以定义规则来描述细胞颜色如何随时间变化。为此，可以为每种细胞颜色定义后续颜色。此外，还必须指定持续时间，以定义相应颜色保持的时间步数。");
                        }
                        AlienImGui::InputColorTransition(
                            widgetParameters,
                            color,
                            parameters.baseValues.cellColorTransitionTargetColor[color],
                            parameters.baseValues.cellColorTransitionDuration[color]);
                        ImGui::PopID();
                    }
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Cell age limiter
             */
            if (parameters.features.cellAgeLimiter) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：细胞年龄限制器").highlighted(false))) {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("最大非活跃细胞年龄")
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
                            .tooltip("在这里，您可以设置功能或其邻居的功能未被触发的细胞的最大年龄。处于“建设中”状态的细胞不受此选项影响。"),
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
                            .tooltip("在这里可以设置由能量粒子产生的细胞的最大年龄。"),
                        parameters.cellEmergentMaxAge,
                        &parameters.cellEmergentMaxAgeActivated);
                    AlienImGui::Checkbox(
                        AlienImGui::CheckboxParameters()
                            .name("构造后重置年龄")
                            .textWidth(RightColumnWidth)
                            .defaultValue(origParameters.cellResetAgeAfterActivation)
                            .tooltip("如果激活此选项，细胞网络构建完成后，细胞的年龄将重置为0，即细胞状态从“建设中”变为“准备就绪”。如果设置了较低的“最大非活跃细胞年龄”，此选项特别有用，因为处于建设中的细胞网络是非活跃的，如果构建时间过长，它们可能在完成后立即死亡。"),
                        parameters.cellResetAgeAfterActivation);
                    AlienImGui::SliderInt(
                        AlienImGui::SliderIntParameters()
                            .name("年龄最大值动态平衡")
                            .textWidth(RightColumnWidth)
                            .logarithmic(true)
                            .min(1000)
                            .max(1000000)
                            .disabledValue(&parameters.cellMaxAgeBalancerInterval)
                            .defaultEnabledValue(&origParameters.cellMaxAgeBalancer)
                            .defaultValue(&origParameters.cellMaxAgeBalancerInterval)
                            .tooltip(
                                "定期调整最大年龄。它会增加具有最少复制器的细胞颜色的最大年龄。相反，对于具有最多复制器的细胞颜色，最大年龄会减少。"),
                        &parameters.cellMaxAgeBalancerInterval,
                        &parameters.cellMaxAgeBalancer);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Cell glow
             */
            if (parameters.features.cellGlow) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：细胞光辉").highlighted(false))) {
                    AlienImGui::Switcher(
                        AlienImGui::SwitcherParameters()
                            .name("染色")
                            .textWidth(RightColumnWidth)
                            .defaultValue(origParameters.cellGlowColoring)
                            .values(
                                {"能量",
                                 "标准细胞颜色",
                                 "基因代际",
                                 "基因代际和细胞功能",
                                 "细胞状态",
                                 "基因组复杂度",
                                 "单一细胞功能",
                                 "全部细胞功能"})
                            .tooltip(Const::ColoringParameterTooltip),
                        parameters.cellGlowColoring);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("范围")
                            .textWidth(RightColumnWidth)
                            .min(1.0f)
                            .max(8.0f)
                            .defaultValue(&origParameters.cellGlowRadius)
                            .tooltip("光辉的半径。请注意范围较大时会对性能产生影响。"),
                        &parameters.cellGlowRadius);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("强度")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(1.0f)
                            .defaultValue(&origParameters.cellGlowStrength)
                            .tooltip("光辉的强度。"),
                        &parameters.cellGlowStrength);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: External energy control
             */
            if (parameters.features.externalEnergyControl) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：额外能量控制").highlighted(false))) {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("额外能量总数")
                            .textWidth(RightColumnWidth)
                            .min(0.0f)
                            .max(100000000.0f)
                            .format("%.0f")
                            .logarithmic(true)
                            .infinity(true)
                            .defaultValue(&origParameters.externalEnergy)
                            .tooltip("此参数可用于设置外部能量池的能量量。然后，这种类型的能量可以以一定的速率传输到所有构造细胞（参见流入设置）。\n\n提示：按住 CTRL 点击滑块可以直接输入数值。\n\n警告：过多的外部能量可能导致大量细胞生成，从而减慢甚至崩溃模拟。"),
                        &parameters.externalEnergy);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("流入设置")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.5f")
                            .logarithmic(true)
                            .defaultValue(origParameters.externalEnergyInflowFactor)
                            .tooltip("在这里可以指定传输到构造细胞的能量比例。\n\n例如，值为 0.05 表示每次构造细胞尝试构建新细胞时，5% 的所需能量将免费从外部能量源传输。"),
                        parameters.externalEnergyInflowFactor);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("条件流入设置")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0.00f)
                            .max(1.0f)
                            .format("%.5f")
                            .defaultValue(origParameters.externalEnergyConditionalInflowFactor)
                            .tooltip(
                                "在这里可以指定传输到构造细胞的能量比例，如果它们能够提供剩余的能量用于构建过程。\n\n例如，值为 0.6 表示构造细胞将免费从外部能量源获得构建新细胞所需能量的 60%。然而，它必须自行提供 40% 的所需能量。否则，将不会传输任何能量。"),
                        parameters.externalEnergyConditionalInflowFactor);
                    AlienImGui::Checkbox(
                        AlienImGui::CheckboxParameters()
                            .name("非复制器的流入设置")
                            .textWidth(RightColumnWidth)
                            .defaultValue(origParameters.externalEnergyInflowOnlyForNonSelfReplicators)
                            .tooltip("如果激活此选项，外部能量只能传输到非自我复制的构造细胞。此选项可用于促进额外身体部位的进化。"),
                        parameters.externalEnergyInflowOnlyForNonSelfReplicators);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("回流设置")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0.0f)
                            .max(1.0f)
                            .defaultValue(origParameters.externalEnergyBackflowFactor)
                            .tooltip("从模拟中流回到外部能量池的能量比例。每次细胞失去能量或死亡时，其能量的一部分将被取走。剩余的能量将留在模拟中，并用于创建新的能量粒子。"),
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
                            .tooltip("只有当外部能量量低于此值时，模拟中的能量才能流回到外部能量池。"),
                        &parameters.externalEnergyBackflowLimit);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Genome complexity measurement
             */
            if (parameters.features.genomeComplexityMeasurement) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：基因组复杂度测量办法").highlighted(false))) {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("细胞数量因素")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .defaultValue(origParameters.genomeComplexitySizeFactor)
                            .tooltip("此参数控制基因组中细胞的数量如何影响其复杂度的计算。"),
                        parameters.genomeComplexitySizeFactor);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("分叉因素")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0.0f)
                            .max(20.0f)
                            .format("%.2f")
                            .defaultValue(origParameters.genomeComplexityRamificationFactor)
                            .tooltip("通过此参数，细胞结构的分支数量将被纳入基因组复杂性的计算中。例如，包含许多子基因组或许多构造分支的基因组将具有较高的复杂性值。"),
                        parameters.genomeComplexityRamificationFactor);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("神经元因素")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0.0f)
                            .max(4.0f)
                            .format("%.2f")
                            .defaultValue(origParameters.genomeComplexityNeuronFactor)
                            .tooltip("此参数将基因组中编码的神经元数量纳入复杂性值的计算中。"),
                        parameters.genomeComplexityNeuronFactor);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Legacy behavior
             */
            if (parameters.features.legacyModes) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：经典行为"))) {
                    AlienImGui::Checkbox(
                        AlienImGui::CheckboxParameters()
                            .name("从相邻传感器获取角度")
                            .textWidth(RightColumnWidth)
                            .defaultValue(origParameters.legacyCellFunctionMuscleMovementAngleFromSensor)
                            .tooltip("此参数改变了“朝向目标移动”参数的行为。如果激活，肌肉细胞将直接从连接的（或间接连接的）传感器细胞获取先前检测到目标的移动角度（传统行为）。如果未激活，输入信号只能来自传感器细胞，且不能是相邻的（新行为）。"),
                        parameters.legacyCellFunctionMuscleMovementAngleFromSensor);
                    AlienImGui::EndTreeNode();
                }
            }
        }
        ImGui::EndChild();

        SimulationParametersValidationService::get().validateAndCorrect(parameters);
        validateAndCorrectLayout();

        if (parameters != lastParameters) {
            _simulationFacade->setSimulationParameters(parameters, SimulationParametersUpdateConfig::AllExceptChangingPositions);
        }

        ImGui::EndTabItem();
    }
}

bool SimulationParametersWindow::processSpot(int index)
{
    std::string name = "区域 " + std::to_string(index + 1);
    bool isOpen = true;
    if (ImGui::BeginTabItem(name.c_str(), &isOpen, ImGuiTabItemFlags_None)) {
        auto parameters = _simulationFacade->getSimulationParameters();
        auto origParameters = _simulationFacade->getOriginalSimulationParameters();
        auto lastParameters = parameters;

        SimulationParametersSpot& spot = parameters.spots[index];
        SimulationParametersSpot const& origSpot = origParameters.spots[index];
        SimulationParametersSpot const& lastSpot = lastParameters.spots[index];

        if (ImGui::BeginChild("##", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            auto worldSize = _simulationFacade->getWorldSize();

            /**
             * Colors and location
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("可视化"))) {
                AlienImGui::ColorButtonWithPicker(
                    AlienImGui::ColorButtonWithPickerParameters().name("背景颜色").textWidth(RightColumnWidth).defaultValue(origSpot.color),
                    spot.color,
                    _backupColor,
                    _savedPalette);
                AlienImGui::EndTreeNode();
            }

            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("位置"))) {
                if (AlienImGui::Switcher(
                        AlienImGui::SwitcherParameters()
                            .name("形状")
                            .values({"圆形", "长方形"})
                            .textWidth(RightColumnWidth)
                            .defaultValue(origSpot.shapeType),
                        spot.shapeType)) {
                    createDefaultSpotData(spot);
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
                        .defaultValue(RealVector2D{origSpot.posX, origSpot.posY})
                        .format("%.2f")
                        .getMousePickerEnabledFunc(getMousePickerEnabledFunc)
                        .setMousePickerEnabledFunc(setMousePickerEnabledFunc)
                        .getMousePickerPositionFunc(getMousePickerPositionFunc),
                    spot.posX,
                    spot.posY);
                AlienImGui::SliderFloat2(
                    AlienImGui::SliderFloat2Parameters()
                        .name("速度 (x,y)")
                        .textWidth(RightColumnWidth)
                        .min({-4.0f, -4.0f})
                        .max({4.0f, 4.0f})
                        .defaultValue(RealVector2D{origSpot.velX, origSpot.velY})
                        .format("%.2f"),
                    spot.velX,
                    spot.velY);
                auto maxRadius = toFloat(std::max(worldSize.x, worldSize.y));
                if (spot.shapeType == SpotShapeType_Circular) {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("核心范围")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(maxRadius)
                            .defaultValue(&origSpot.shapeData.circularSpot.coreRadius)
                            .format("%.1f"),
                        &spot.shapeData.circularSpot.coreRadius);
                }
                if (spot.shapeType == SpotShapeType_Rectangular) {
                    AlienImGui::SliderFloat2(
                        AlienImGui::SliderFloat2Parameters()
                            .name("大小 (x,y)")
                            .textWidth(RightColumnWidth)
                            .min({0, 0})
                            .max({toFloat(worldSize.x), toFloat(worldSize.y)})
                            .defaultValue(RealVector2D{origSpot.shapeData.rectangularSpot.width, origSpot.shapeData.rectangularSpot.height})
                            .format("%.1f"),
                        spot.shapeData.rectangularSpot.width,
                        spot.shapeData.rectangularSpot.height);
                }

                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("淡出范围")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(maxRadius)
                        .defaultValue(&origSpot.fadeoutRadius)
                        .format("%.1f"),
                    &spot.fadeoutRadius);
                AlienImGui::EndTreeNode();
            }

            /**
             * Flow
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("力场"))) {
                auto isForceFieldActive = spot.flowType != FlowType_None;

                auto forceFieldTypeIntern = std::max(0, spot.flowType - 1);  //FlowType_None should not be selectable in ComboBox
                auto origForceFieldTypeIntern = std::max(0, origSpot.flowType - 1);
                if (ImGui::Checkbox("##forceField", &isForceFieldActive)) {
                    spot.flowType = isForceFieldActive ? FlowType_Radial : FlowType_None;
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(!isForceFieldActive);
                auto posX = ImGui::GetCursorPos().x;
                if (AlienImGui::Combo(
                        AlienImGui::ComboParameters()
                            .name("类型")
                            .values({"放射性", "中心性", "线性"})
                            .textWidth(RightColumnWidth)
                            .defaultValue(origForceFieldTypeIntern),
                        forceFieldTypeIntern)) {
                    spot.flowType = forceFieldTypeIntern + 1;
                    if (spot.flowType == FlowType_Radial) {
                        spot.flowData.radialFlow = RadialFlow();
                    }
                    if (spot.flowType == FlowType_Central) {
                        spot.flowData.centralFlow = CentralFlow();
                    }
                    if (spot.flowType == FlowType_Linear) {
                        spot.flowData.linearFlow = LinearFlow();
                    }
                }
                if (spot.flowType == FlowType_Radial) {
                    ImGui::SetCursorPosX(posX);
                    AlienImGui::Combo(
                        AlienImGui::ComboParameters()
                            .name("朝向")
                            .textWidth(RightColumnWidth)
                            .defaultValue(origSpot.flowData.radialFlow.orientation)
                            .values({"顺时针", "逆时针"}),
                        spot.flowData.radialFlow.orientation);
                    ImGui::SetCursorPosX(posX);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("强度")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(0.5f)
                            .logarithmic(true)
                            .format("%.5f")
                            .defaultValue(&origSpot.flowData.radialFlow.strength),
                        &spot.flowData.radialFlow.strength);
                    ImGui::SetCursorPosX(posX);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("漂移角度")
                            .textWidth(RightColumnWidth)
                            .min(-180.0f)
                            .max(180.0f)
                            .format("%.1f")
                            .defaultValue(&origSpot.flowData.radialFlow.driftAngle),
                        &spot.flowData.radialFlow.driftAngle);
                }
                if (spot.flowType == FlowType_Central) {
                    ImGui::SetCursorPosX(posX);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("强度")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(0.5f)
                            .logarithmic(true)
                            .format("%.5f")
                            .defaultValue(&origSpot.flowData.centralFlow.strength),
                        &spot.flowData.centralFlow.strength);
                }
                if (spot.flowType == FlowType_Linear) {
                    ImGui::SetCursorPosX(posX);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("角度")
                            .textWidth(RightColumnWidth)
                            .min(-180.0f)
                            .max(180.0f)
                            .format("%.1f")
                            .defaultValue(&origSpot.flowData.linearFlow.angle),
                        &spot.flowData.linearFlow.angle);
                    ImGui::SetCursorPosX(posX);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("强度")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(0.5f)
                            .logarithmic(true)
                            .format("%.5f")
                            .defaultValue(&origSpot.flowData.linearFlow.strength),
                        &spot.flowData.linearFlow.strength);
                }
                ImGui::EndDisabled();
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Motion
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：运动"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("摩擦力")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(1)
                        .logarithmic(true)
                        .defaultValue(&origSpot.values.friction)
                        .disabledValue(&parameters.baseValues.friction)
                        .format("%.4f"),
                    &spot.values.friction,
                    &spot.activatedValues.friction);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("刚性")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(1)
                        .defaultValue(&origSpot.values.rigidity)
                        .disabledValue(&parameters.baseValues.rigidity)
                        .format("%.2f"),
                    &spot.values.rigidity,
                    &spot.activatedValues.rigidity);
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Thresholds
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：阈值"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最大力度")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(3.0f)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellMaxForce)
                        .disabledValue(parameters.baseValues.cellMaxForce),
                    spot.values.cellMaxForce,
                    &spot.activatedValues.cellMaxForce);
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Binding
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：连接"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("连接速度")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(2.0f)
                        .defaultValue(&origSpot.values.cellFusionVelocity)
                        .disabledValue(&parameters.baseValues.cellFusionVelocity),
                    &spot.values.cellFusionVelocity,
                    &spot.activatedValues.cellFusionVelocity);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最大能量")
                        .textWidth(RightColumnWidth)
                        .min(50.0f)
                        .max(10000000.0f)
                        .logarithmic(true)
                        .infinity(true)
                        .format("%.0f")
                        .defaultValue(&origSpot.values.cellMaxBindingEnergy)
                        .disabledValue(&parameters.baseValues.cellMaxBindingEnergy),
                    &spot.values.cellMaxBindingEnergy,
                    &spot.activatedValues.cellMaxBindingEnergy);
                AlienImGui::EndTreeNode();
            }

            /**
             * Physics: Radiation
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("物理：放射性粒子"))) {
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters()
                        .name("禁用放射性粒子源")
                        .textWidth(RightColumnWidth)
                        .defaultValue(origSpot.values.radiationDisableSources)
                        .tooltip("如果启用，所有在该区域的放射性粒子源都将被临时禁用。"),
                    spot.values.radiationDisableSources);
                spot.activatedValues.radiationDisableSources = spot.values.radiationDisableSources;

                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("吸收率")
                        .textWidth(RightColumnWidth)
                        .logarithmic(true)
                        .colorDependence(true)
                        .min(0)
                        .max(1.0)
                        .format("%.4f")
                        .defaultValue(origSpot.values.radiationAbsorption)
                        .disabledValue(parameters.baseValues.radiationAbsorption),
                    spot.values.radiationAbsorption,
                    &spot.activatedValues.radiationAbsorption);

                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("辐射类型 I：强度")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(0.01f)
                        .logarithmic(true)
                        .defaultValue(origSpot.values.radiationCellAgeStrength)
                        .disabledValue(parameters.baseValues.radiationCellAgeStrength)
                        .format("%.6f"),
                    spot.values.radiationCellAgeStrength,
                    &spot.activatedValues.radiationCellAgeStrength);
                AlienImGui::EndTreeNode();
            }

            /**
             * Cell life cycle
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞生命周期"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("最小能量")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(10.0f)
                        .max(200.0f)
                        .defaultValue(origSpot.values.cellMinEnergy)
                        .disabledValue(parameters.baseValues.cellMinEnergy),
                    spot.values.cellMinEnergy,
                    &spot.activatedValues.cellMinEnergy);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("正在死亡的细胞的腐败率")
                        .colorDependence(true)
                        .textWidth(RightColumnWidth)
                        .min(1e-6f)
                        .max(0.1f)
                        .format("%.6f")
                        .logarithmic(true)
                    .defaultValue(origSpot.values.cellDeathProbability).disabledValue(parameters.baseValues.cellDeathProbability),
                    spot.values.cellDeathProbability,
                    &spot.activatedValues.cellDeathProbability);
                AlienImGui::EndTreeNode();
            }

            /**
             * Mutation 
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("基因组复制时的突变"))) {
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("神经元比重和偏差")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .colorDependence(true)
                        .logarithmic(true)
                        .defaultValue(origSpot.values.cellCopyMutationNeuronData)
                        .disabledValue(parameters.baseValues.cellCopyMutationNeuronData),
                    spot.values.cellCopyMutationNeuronData,
                    &spot.activatedValues.cellCopyMutationNeuronData);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("细胞属性")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationCellProperties)
                        .disabledValue(parameters.baseValues.cellCopyMutationCellProperties),
                    spot.values.cellCopyMutationCellProperties,
                    &spot.activatedValues.cellCopyMutationCellProperties);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("几何形状")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationGeometry)
                        .disabledValue(parameters.baseValues.cellCopyMutationGeometry),
                    spot.values.cellCopyMutationGeometry,
                    &spot.activatedValues.cellCopyMutationGeometry);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("自定义几何形状")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationCustomGeometry)
                        .disabledValue(parameters.baseValues.cellCopyMutationCustomGeometry),
                    spot.values.cellCopyMutationCustomGeometry,
                    &spot.activatedValues.cellCopyMutationCustomGeometry);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("细胞功能类型")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationCellFunction)
                        .disabledValue(parameters.baseValues.cellCopyMutationCellFunction),
                    spot.values.cellCopyMutationCellFunction,
                    &spot.activatedValues.cellCopyMutationCellFunction);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("插入")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationInsertion)
                        .disabledValue(parameters.baseValues.cellCopyMutationInsertion),
                    spot.values.cellCopyMutationInsertion,
                    &spot.activatedValues.cellCopyMutationInsertion);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("删除")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationDeletion)
                        .disabledValue(parameters.baseValues.cellCopyMutationDeletion),
                    spot.values.cellCopyMutationDeletion,
                    &spot.activatedValues.cellCopyMutationDeletion);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("变换")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationTranslation)
                        .disabledValue(parameters.baseValues.cellCopyMutationTranslation),
                    spot.values.cellCopyMutationTranslation,
                    &spot.activatedValues.cellCopyMutationTranslation);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("复制")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationDuplication)
                        .disabledValue(parameters.baseValues.cellCopyMutationDuplication),
                    spot.values.cellCopyMutationDuplication,
                    &spot.activatedValues.cellCopyMutationDuplication);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("单个细胞颜色")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationCellColor)
                        .disabledValue(parameters.baseValues.cellCopyMutationCellColor),
                    spot.values.cellCopyMutationCellColor,
                    &spot.activatedValues.cellCopyMutationCellColor);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("次级基因组颜色")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationSubgenomeColor)
                        .disabledValue(parameters.baseValues.cellCopyMutationSubgenomeColor),
                    spot.values.cellCopyMutationSubgenomeColor,
                    &spot.activatedValues.cellCopyMutationSubgenomeColor);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("基因组颜色")
                        .textWidth(RightColumnWidth)
                        .min(0.0f)
                        .max(1.0f)
                        .format("%.7f")
                        .logarithmic(true)
                        .colorDependence(true)
                        .defaultValue(origSpot.values.cellCopyMutationGenomeColor)
                        .disabledValue(parameters.baseValues.cellCopyMutationGenomeColor),
                    spot.values.cellCopyMutationGenomeColor,
                    &spot.activatedValues.cellCopyMutationGenomeColor);
                AlienImGui::EndTreeNode();
            }

            /**
             * Attacker
             */
            if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("细胞功能：攻击器"))) {
                AlienImGui::InputFloatColorMatrix(
                    AlienImGui::InputFloatColorMatrixParameters()
                        .name("食物链颜色矩阵")
                        .max(1)
                        .textWidth(RightColumnWidth)
                        .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origSpot.values.cellFunctionAttackerFoodChainColorMatrix))
                        .disabledValue(toVector<MAX_COLORS, MAX_COLORS>(parameters.baseValues.cellFunctionAttackerFoodChainColorMatrix)),
                    spot.values.cellFunctionAttackerFoodChainColorMatrix,
                    &spot.activatedValues.cellFunctionAttackerFoodChainColorMatrix);
                AlienImGui::InputFloatColorMatrix(
                    AlienImGui::InputFloatColorMatrixParameters()
                        .name("复杂生物保护")
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(20.0f)
                        .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origSpot.values.cellFunctionAttackerGenomeComplexityBonus))
                        .disabledValue(toVector<MAX_COLORS, MAX_COLORS>(parameters.baseValues.cellFunctionAttackerGenomeComplexityBonus)),
                    spot.values.cellFunctionAttackerGenomeComplexityBonus,
                    &spot.activatedValues.cellFunctionAttackerGenomeComplexityBonus);
                AlienImGui::SliderFloat(
                    AlienImGui::SliderFloatParameters()
                        .name("能量损耗")
                        .textWidth(RightColumnWidth)
                        .colorDependence(true)
                        .min(0)
                        .max(1.0f)
                        .format("%.5f")
                        .logarithmic(true)
                        .defaultValue(origSpot.values.cellFunctionAttackerEnergyCost)
                        .disabledValue(parameters.baseValues.cellFunctionAttackerEnergyCost),
                    spot.values.cellFunctionAttackerEnergyCost,
                    &spot.activatedValues.cellFunctionAttackerEnergyCost);
                AlienImGui::EndTreeNode();
            }

            /**
             * Addon: Advanced absorption control
             */
            if (parameters.features.advancedAbsorptionControl) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：高级能量吸收控制"))) {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("低速度惩罚")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(1.0f)
                            .format("%.2f")
                            .defaultValue(origSpot.values.radiationAbsorptionLowVelocityPenalty)
                            .disabledValue(parameters.baseValues.radiationAbsorptionLowVelocityPenalty),
                        spot.values.radiationAbsorptionLowVelocityPenalty,
                        &spot.activatedValues.radiationAbsorptionLowVelocityPenalty);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("基因组低复杂度惩罚")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(1.0f)
                            .format("%.2f")
                            .defaultValue(origSpot.values.radiationAbsorptionLowGenomeComplexityPenalty)
                            .disabledValue(parameters.baseValues.radiationAbsorptionLowGenomeComplexityPenalty),
                        spot.values.radiationAbsorptionLowGenomeComplexityPenalty,
                        &spot.activatedValues.radiationAbsorptionLowGenomeComplexityPenalty);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Advanced attacker control
             */
            if (parameters.features.advancedAttackerControl) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：高级攻击器控制"))) {
                    AlienImGui::InputFloatColorMatrix(
                        AlienImGui::InputFloatColorMatrixParameters()
                            .name("新种族保护")
                            .textWidth(RightColumnWidth)
                            .min(0)
                            .max(1.0f)
                            .defaultValue(toVector<MAX_COLORS, MAX_COLORS>(origSpot.values.cellFunctionAttackerNewComplexMutantPenalty))
                            .disabledValue(toVector<MAX_COLORS, MAX_COLORS>(parameters.baseValues.cellFunctionAttackerNewComplexMutantPenalty)),
                        spot.values.cellFunctionAttackerNewComplexMutantPenalty,
                        &spot.activatedValues.cellFunctionAttackerNewComplexMutantPenalty);

                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("几何形状惩罚")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(5.0f)
                            .defaultValue(origSpot.values.cellFunctionAttackerGeometryDeviationExponent)
                            .disabledValue(parameters.baseValues.cellFunctionAttackerGeometryDeviationExponent),
                        spot.values.cellFunctionAttackerGeometryDeviationExponent,
                        &spot.activatedValues.cellFunctionAttackerGeometryDeviationExponent);
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("连接不匹配惩罚")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(0)
                            .max(1.0f)
                            .defaultValue(origSpot.values.cellFunctionAttackerConnectionsMismatchPenalty)
                            .disabledValue(parameters.baseValues.cellFunctionAttackerConnectionsMismatchPenalty),
                        spot.values.cellFunctionAttackerConnectionsMismatchPenalty,
                        &spot.activatedValues.cellFunctionAttackerConnectionsMismatchPenalty);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Cell age limiter
             */
            if (parameters.features.cellAgeLimiter) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：细胞年龄限制器").highlighted(false))) {
                    AlienImGui::SliderFloat(
                        AlienImGui::SliderFloatParameters()
                            .name("最大非活跃细胞年龄")
                            .textWidth(RightColumnWidth)
                            .colorDependence(true)
                            .min(1.0f)
                            .max(10000000.0f)
                            .logarithmic(true)
                            .infinity(true)
                            .format("%.0f")
                            .disabledValue(parameters.baseValues.cellInactiveMaxAge)
                            .defaultValue(origSpot.values.cellInactiveMaxAge),
                        spot.values.cellInactiveMaxAge,
                        &spot.activatedValues.cellInactiveMaxAge);
                    AlienImGui::EndTreeNode();
                }
            }

            /**
             * Addon: Cell color transition rules
             */
            if (parameters.features.cellColorTransitionRules) {
                if (AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件：细胞颜色过渡规则").highlighted(false))) {
                    ImGui::Checkbox("##cellColorTransition", &spot.activatedValues.cellColorTransition);
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!spot.activatedValues.cellColorTransition);
                    auto posX = ImGui::GetCursorPos().x;
                    for (int color = 0; color < MAX_COLORS; ++color) {
                        ImGui::SetCursorPosX(posX);
                        ImGui::PushID(color);
                        auto parameters = AlienImGui::InputColorTransitionParameters()
                                              .textWidth(RightColumnWidth)
                                              .color(color)
                                              .defaultTargetColor(origSpot.values.cellColorTransitionTargetColor[color])
                                              .defaultTransitionAge(origSpot.values.cellColorTransitionDuration[color])
                                              .logarithmic(true)
                                              .infinity(true);
                        if (0 == color) {
                            parameters.name("目标颜色和持续时间");
                        }
                        AlienImGui::InputColorTransition(
                            parameters, color, spot.values.cellColorTransitionTargetColor[color], spot.values.cellColorTransitionDuration[color]);
                        ImGui::PopID();
                    }
                    ImGui::EndDisabled();
                    AlienImGui::EndTreeNode();
                    if (!spot.activatedValues.cellColorTransition) {
                        for (int color = 0; color < MAX_COLORS; ++color) {
                            spot.values.cellColorTransitionTargetColor[color] = parameters.baseValues.cellColorTransitionTargetColor[color];
                            spot.values.cellColorTransitionDuration[color] = parameters.baseValues.cellColorTransitionDuration[color];
                        }
                    }
                }
            }
        }
        ImGui::EndChild();
        SimulationParametersValidationService::get().validateAndCorrect(parameters);

        if (spot != lastSpot) {
            auto isRunning = _simulationFacade->isSimulationRunning();
            _simulationFacade->setSimulationParameters(
                parameters, isRunning ? SimulationParametersUpdateConfig::AllExceptChangingPositions : SimulationParametersUpdateConfig::All);
        }

        ImGui::EndTabItem();
    }

    return isOpen;
}

void SimulationParametersWindow::processAddonList()
{
    if (_featureListOpen) {
        ImGui::Spacing();
        ImGui::Spacing();
        AlienImGui::MovableSeparator(_featureListHeight);
    } else {
        AlienImGui::Separator();
    }

    _featureListOpen = AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("插件").highlighted(true).defaultOpen(_featureListOpen));
    if (_featureListOpen) {
        if (ImGui::BeginChild("##addons", {scale(0), 0})) {
            auto parameters = _simulationFacade->getSimulationParameters();
            auto origFeatures = _simulationFacade->getOriginalSimulationParameters().features;
            auto lastFeatures= parameters.features;

            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("高级能量吸收控制")
                    .textWidth(0)
                    .defaultValue(origFeatures.advancedAbsorptionControl)
                    .tooltip("此插件提供了更多控制细胞吸收能量粒子的能力。"),
                parameters.features.advancedAbsorptionControl);
            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("高级攻击器控制")
                    .textWidth(0)
                    .defaultValue(origFeatures.advancedAttackerControl)
                    .tooltip("它包含影响攻击细胞从攻击中获得的能量数量的进一步的设置。"),
                parameters.features.advancedAttackerControl);
            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("细胞年龄限制器")
                    .textWidth(0)
                    .defaultValue(origFeatures.cellAgeLimiter)
                    .tooltip("它提供了更多控制细胞最大年龄的可能性。"),
                parameters.features.cellAgeLimiter);
            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("细胞颜色过渡规则")
                    .textWidth(0)
                    .defaultValue(origFeatures.cellColorTransitionRules)
                    .tooltip("这可以用来根据细胞的年龄定义颜色过渡。"),
                parameters.features.cellColorTransitionRules);
            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("细胞光辉")
                    .textWidth(0)
                    .defaultValue(origFeatures.cellGlow)
                    .tooltip("它启用一个额外的渲染步骤，使细胞发光。"),
                parameters.features.cellGlow);
            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("额外能量控制")
                    .textWidth(0)
                    .defaultValue(origFeatures.externalEnergyControl)
                    .tooltip("此插件用于添加外部能量源。其能量可以逐渐传输到模拟中的构造细胞。反之，辐射和死亡细胞的能量也可以传输回外部能量源。"),
                parameters.features.externalEnergyControl);
            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("基因组复杂度测量方法")
                    .textWidth(0)
                    .defaultValue(origFeatures.genomeComplexityMeasurement)
                    .tooltip("这里激活是基因组复杂度计算的参数。此基因组复杂度可以用于“高级能量吸收控制”和“高级攻击器控制”，以在自然选择中偏爱更复杂的基因组。如果未激活，则使用默认值，仅考虑基因组大小。"),
                parameters.features.genomeComplexityMeasurement);
            AlienImGui::Checkbox(
                AlienImGui::CheckboxParameters()
                    .name("经典行为")
                    .textWidth(0)
                    .defaultValue(origFeatures.legacyModes)
                    .tooltip("它包含与旧版本兼容的功能。"),
                parameters.features.legacyModes);

            if (parameters.features != lastFeatures) {
                _simulationFacade->setSimulationParameters(parameters);
            }
        }
        ImGui::EndChild();
        AlienImGui::EndTreeNode();
    }
}

void SimulationParametersWindow::processStatusBar()
{
    std::vector<std::string> statusItems;
    statusItems.emplace_back("在滑槽上 CTRL + 点击 可以输入精确值");

    AlienImGui::StatusBar(statusItems);
}

void SimulationParametersWindow::onAppendTab()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();

    int index = parameters.numSpots;
    parameters.spots[index] = createSpot(parameters, index);
    origParameters.spots[index] = createSpot(parameters, index);
    ++parameters.numSpots;
    ++origParameters.numSpots;
    _simulationFacade->setSimulationParameters(parameters);
    _simulationFacade->setOriginalSimulationParameters(origParameters);
}

void SimulationParametersWindow::onDeleteTab(int index)
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();

    for (int i = index; i < parameters.numSpots - 1; ++i) {
        parameters.spots[i] = parameters.spots[i + 1];
        origParameters.spots[i] = origParameters.spots[i + 1];
    }
    --parameters.numSpots;
    --origParameters.numSpots;
    _simulationFacade->setSimulationParameters(parameters);
    _simulationFacade->setOriginalSimulationParameters(origParameters);
}

void SimulationParametersWindow::onOpenParameters()
{
    GenericFileDialog::get().showOpenFileDialog(
        "打开模拟器参数集", "模拟器参数集 (*.parameters){.parameters},.*", _startingPath, [&](std::filesystem::path const& path) {
            auto firstFilename = ifd::FileDialog::Instance().GetResult();
            auto firstFilenameCopy = firstFilename;
            _startingPath = firstFilenameCopy.remove_filename().string();

            SimulationParameters parameters;
            if (!SerializerService::get().deserializeSimulationParametersFromFile(parameters, firstFilename.string())) {
                GenericMessageDialog::get().information("打开模拟器参数集", "选中的文件无法被打开。");
            } else {
                _simulationFacade->setSimulationParameters(parameters);
                _simulationFacade->setOriginalSimulationParameters(parameters);
            }
        });
}

void SimulationParametersWindow::onSaveParameters()
{
    GenericFileDialog::get().showSaveFileDialog(
        "保存模拟器参数集", "模拟器参数集 (*.parameters){.parameters},.*", _startingPath, [&](std::filesystem::path const& path) {
        auto firstFilename = ifd::FileDialog::Instance().GetResult();
        auto firstFilenameCopy = firstFilename;
        _startingPath = firstFilenameCopy.remove_filename().string();

        auto parameters = _simulationFacade->getSimulationParameters();
        if (!SerializerService::get().serializeSimulationParametersToFile(firstFilename.string(), parameters)) {
            GenericMessageDialog::get().information("保存模拟器参数集", "选中的文件无法被保存。");
        }
    });
}

void SimulationParametersWindow::validateAndCorrectLayout()
{
    _featureListHeight = std::max(0.0f, _featureListHeight);
}
