#include "InspectorWindow.h"

#include <sstream>
#include <imgui.h>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include "EngineInterface/DescriptionEditService.h"
#include "EngineInterface/SimulationFacade.h"
#include "EngineInterface/GenomeDescriptionService.h"
#include "EngineInterface/PreviewDescriptionService.h"

#include "StyleRepository.h"
#include "Viewport.h"
#include "EditorModel.h"
#include "AlienImGui.h"
#include "CellFunctionStrings.h"
#include "GenomeEditorWindow.h"
#include "HelpStrings.h"
#include "OverlayController.h"

using namespace std::string_literals;

namespace
{
    auto const CellWindowWidth = 350.0f;
    auto const ParticleWindowWidth = 280.0f;
    auto const BaseTabTextWidth = 162.0f;
    auto const CellFunctionTextWidth = 195.0f;
    auto const CellFunctionDefenderWidth = 100.0f;
    auto const CellFunctionBaseTabTextWidth = 150.0f;
    auto const SignalTextWidth = 130.0f;
    auto const GenomeTabTextWidth = 195.0f;
    auto const ParticleContentTextWidth = 80.0f;

    auto const TreeNodeFlags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen;
}

_InspectorWindow::_InspectorWindow(SimulationFacade const& simulationFacade, uint64_t entityId, RealVector2D const& initialPos, bool selectGenomeTab)
    : _entityId(entityId)
    , _initialPos(initialPos)
    , _simulationFacade(simulationFacade)
    , _selectGenomeTab(selectGenomeTab)
{
}

_InspectorWindow::~_InspectorWindow() {}

void _InspectorWindow::process()
{
    if (!_on) {
        return;
    }
    auto width = calcWindowWidth();
    auto height = isCell() ? StyleRepository::get().scale(370.0f)
                           : StyleRepository::get().scale(70.0f);
    auto borderlessRendering = _simulationFacade->getSimulationParameters().borderlessRendering;
    ImGui::SetNextWindowBgAlpha(Const::WindowAlpha * ImGui::GetStyle().Alpha);
    ImGui::SetNextWindowSize({width, height}, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos({_initialPos.x, _initialPos.y}, ImGuiCond_Appearing);
    auto entity = EditorModel::get().getInspectedEntity(_entityId);
    if (ImGui::Begin(generateTitle().c_str(), &_on, ImGuiWindowFlags_HorizontalScrollbar)) {
        auto windowPos = ImGui::GetWindowPos();
        if (isCell()) {
            processCell(std::get<CellDescription>(entity));
        } else {
            processParticle(std::get<ParticleDescription>(entity));
        }
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        auto entityPos = Viewport::get().mapWorldToViewPosition(DescriptionEditService::get().getPos(entity), borderlessRendering);
        auto factor = StyleRepository::get().scale(1);

        drawList->AddLine(
            {windowPos.x + 15.0f * factor, windowPos.y - 5.0f * factor},
            {entityPos.x, entityPos.y},
            Const::InspectorLineColor,
            1.5f);
        drawList->AddRectFilled(
            {windowPos.x + 5.0f * factor, windowPos.y - 10.0f * factor},
            {windowPos.x + 25.0f * factor, windowPos.y},
            Const::InspectorRectColor,
            1.0,
            0);
        drawList->AddRect(
            {windowPos.x + 5.0f * factor, windowPos.y - 10.0f * factor},
            {windowPos.x + 25.0f * factor, windowPos.y},
            Const::InspectorLineColor,
            1.0,
            0,
            2.0f);
    }
    ImGui::End();
}

bool _InspectorWindow::isClosed() const
{
    return !_on;
}

uint64_t _InspectorWindow::getId() const
{
    return _entityId;
}

bool _InspectorWindow::isCell() const
{
    auto entity = EditorModel::get().getInspectedEntity(_entityId);
    return std::holds_alternative<CellDescription>(entity);
}

std::string _InspectorWindow::generateTitle() const
{
    auto entity = EditorModel::get().getInspectedEntity(_entityId);
    std::stringstream ss;
    if (isCell()) {
        ss << "细胞 #" << std::hex << std::uppercase << _entityId;
    } else {
        ss << "能量粒子 #" << std::hex << std::uppercase << _entityId;
    }
    return ss.str();
}

void _InspectorWindow::processCell(CellDescription cell)
{
    if (ImGui::BeginTabBar(
            "##细胞查看", /*ImGuiTabBarFlags_AutoSelectNewTabs | */ImGuiTabBarFlags_FittingPolicyResizeDown)) {
        auto origCell = cell;
        processCellBaseTab(cell);
        processCellFunctionTab(cell);
        processCellFunctionPropertiesTab(cell);
        if (cell.getCellFunctionType() == CellFunction_Constructor) {
            processCellGenomeTab(std::get<ConstructorDescription>(*cell.cellFunction));
        }
        if (cell.getCellFunctionType() == CellFunction_Injector) {
            processCellGenomeTab(std::get<InjectorDescription>(*cell.cellFunction));
        }
        processCellMetadataTab(cell);
        validateAndCorrect(cell);

        ImGui::EndTabBar();

        if (cell != origCell) {
            _simulationFacade->changeCell(cell);
        }
    }
}

void _InspectorWindow::processCellBaseTab(CellDescription& cell)
{
    if (ImGui::BeginTabItem("基础", nullptr, ImGuiTabItemFlags_None)) {
        if (ImGui::BeginChild("##", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            if (ImGui::TreeNodeEx("属性##基础", TreeNodeFlags)) {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << cell.id;
                auto cellId = ss.str();

                AlienImGui::ComboColor(
                    AlienImGui::ComboColorParameters().name("颜色").textWidth(BaseTabTextWidth).tooltip(Const::GenomeColorTooltip), cell.color);
                AlienImGui::InputFloat(
                    AlienImGui::InputFloatParameters().name("能量").format("%.2f").textWidth(BaseTabTextWidth).tooltip(Const::CellEnergyTooltip),
                    cell.energy);
                AlienImGui::InputInt(AlienImGui::InputIntParameters().name("年龄").textWidth(BaseTabTextWidth).tooltip(Const::CellAgeTooltip), cell.age);
                AlienImGui::InputFloat(AlienImGui::InputFloatParameters().name("位置 X").format("%.2f").textWidth(BaseTabTextWidth), cell.pos.x);
                AlienImGui::InputFloat(AlienImGui::InputFloatParameters().name("位置 Y").format("%.2f").textWidth(BaseTabTextWidth), cell.pos.y);
                AlienImGui::InputFloat(AlienImGui::InputFloatParameters().name("速度 X").format("%.2f").textWidth(BaseTabTextWidth), cell.vel.x);
                AlienImGui::InputFloat(AlienImGui::InputFloatParameters().name("速度 Y").format("%.2f").textWidth(BaseTabTextWidth), cell.vel.y);
                AlienImGui::InputFloat(
                    AlienImGui::InputFloatParameters().name("刚性").format("%.2f").step(0.05f).textWidth(BaseTabTextWidth).tooltip(Const::CellStiffnessTooltip),
                    cell.stiffness);
                AlienImGui::InputInt(AlienImGui::InputIntParameters().name("最大连接数").textWidth(BaseTabTextWidth).tooltip(Const::CellMaxConnectionTooltip), cell.maxConnections);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters().name("不可摧毁的墙").textWidth(BaseTabTextWidth).tooltip(Const::CellIndestructibleTooltip), cell.barrier);
                AlienImGui::InputText(
                    AlienImGui::InputTextParameters().name("细胞 id").textWidth(BaseTabTextWidth).tooltip(Const::CellIdTooltip).readOnly(true), cellId);
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("关联生物##基础", TreeNodeFlags)) {
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters().name("生物id").textWidth(BaseTabTextWidth).tooltip(Const::CellCreatureIdTooltip), cell.creatureId);
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters().name("种族id").textWidth(BaseTabTextWidth).tooltip(Const::CellMutationIdTooltip), cell.mutationId);
                AlienImGui::InputFloat(
                    AlienImGui::InputFloatParameters().name("基因组复杂度").textWidth(BaseTabTextWidth).tooltip(Const::GenomeComplexityTooltip),
                    cell.genomeComplexity);

                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("与其他细胞的连接数", TreeNodeFlags)) {
                for (auto const& [index, connection] : cell.connections | boost::adaptors::indexed(0)) {
                    if (ImGui::TreeNodeEx(("连接 [" + std::to_string(index) + "]").c_str(), ImGuiTreeNodeFlags_None)) {
                        AlienImGui::InputFloat(
                            AlienImGui::InputFloatParameters()
                                .name("相关距离")
                                .format("%.2f")
                                .textWidth(BaseTabTextWidth)
                                .readOnly(true)
                                .tooltip(Const::CellReferenceDistanceTooltip),
                            connection.distance);
                        AlienImGui::InputFloat(
                            AlienImGui::InputFloatParameters()
                                .name("相关角度")
                                .format("%.2f")
                                .textWidth(BaseTabTextWidth)
                                .readOnly(true)
                                .tooltip(Const::CellReferenceAngleTooltip),
                            connection.angleFromPrevious);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

void _InspectorWindow::processCellFunctionTab(CellDescription& cell)
{
    if (ImGui::BeginTabItem("功能", nullptr, ImGuiTabItemFlags_None)) {
        int type = cell.getCellFunctionType();
        if (ImGui::BeginChild("##", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            if (ImGui::TreeNodeEx("属性##功能", TreeNodeFlags)) {
                if (AlienImGui::CellFunctionCombo(
                        AlienImGui::CellFunctionComboParameters()
                            .name("功能")
                            .textWidth(CellFunctionBaseTabTextWidth)
                            .tooltip(Const::getCellFunctionTooltip(type)),
                        type)) {
                    switch (type) {
                    case CellFunction_Neuron: {
                        cell.cellFunction = NeuronDescription();
                    } break;
                    case CellFunction_Transmitter: {
                        cell.cellFunction = TransmitterDescription();
                    } break;
                    case CellFunction_Constructor: {
                        cell.cellFunction = ConstructorDescription();
                    } break;
                    case CellFunction_Sensor: {
                        cell.cellFunction = SensorDescription();
                    } break;
                    case CellFunction_Nerve: {
                        cell.cellFunction = NerveDescription();
                    } break;
                    case CellFunction_Attacker: {
                        cell.cellFunction = AttackerDescription();
                    } break;
                    case CellFunction_Injector: {
                        cell.cellFunction = InjectorDescription();
                    } break;
                    case CellFunction_Muscle: {
                        cell.cellFunction = MuscleDescription();
                    } break;
                    case CellFunction_Defender: {
                        cell.cellFunction = DefenderDescription();
                    } break;
                    case CellFunction_Reconnector: {
                        cell.cellFunction = ReconnectorDescription();
                    } break;
                    case CellFunction_Detonator: {
                        cell.cellFunction = DetonatorDescription();
                    } break;
                    case CellFunction_None: {
                        cell.cellFunction.reset();
                    } break;
                    }
                }

                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters()
                        .name("激活次数")
                        .textWidth(CellFunctionBaseTabTextWidth)
                        .tooltip(Const::GenomeConstructorOffspringActivationTime),
                    cell.activationTime);
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters().name("执行编号").textWidth(CellFunctionBaseTabTextWidth).tooltip(Const::GenomeExecutionNumberTooltip), cell.executionOrderNumber);
                AlienImGui::InputOptionalInt(
                    AlienImGui::InputIntParameters().name("输入编号").textWidth(CellFunctionBaseTabTextWidth).tooltip(Const::GenomeInputExecutionNumberTooltip), cell.inputExecutionOrderNumber);
                AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters().name("锁定输出").textWidth(CellFunctionBaseTabTextWidth).tooltip(Const::GenomeBlockOutputTooltip), cell.outputBlocked);
                AlienImGui::Combo(
                    AlienImGui::ComboParameters()
                        .name("生存状态")
                        .textWidth(CellFunctionBaseTabTextWidth)
                        .values({"准备中", "构筑中", "活动中", "分离中", "复活中", "死去中"})
                        .tooltip(Const::CellLivingStateTooltip),
                    cell.livingState);
                ImGui::TreePop();
            }
        }
        if (ImGui::TreeNodeEx("信号", TreeNodeFlags)) {
            int index = 0;
            for (auto& channel : cell.signal.channels) {
                AlienImGui::InputFloat(
                    AlienImGui::InputFloatParameters().name("频道 #" + std::to_string(index)).format("%.3f").step(0.1f).textWidth(SignalTextWidth),
                    channel);
                ++index;
            }
            ImGui::TreePop();
        }

        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

void _InspectorWindow::processCellFunctionPropertiesTab(CellDescription& cell)
{
    if (cell.getCellFunctionType() == CellFunction_None) {
        return;
    }

    std::string title = Const::CellFunctionToStringMap.at(cell.getCellFunctionType());
    if (ImGui::BeginTabItem(title.c_str(), nullptr, ImGuiTabItemFlags_None)) {
        if (ImGui::BeginChild("##", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            switch (cell.getCellFunctionType()) {
            case CellFunction_Neuron: {
                processNeuronContent(std::get<NeuronDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Transmitter: {
                processTransmitterContent(std::get<TransmitterDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Constructor: {
                processConstructorContent(std::get<ConstructorDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Sensor: {
                processSensorContent(std::get<SensorDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Nerve: {
                processNerveContent(std::get<NerveDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Attacker: {
                processAttackerContent(std::get<AttackerDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Injector: {
                processInjectorContent(std::get<InjectorDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Muscle: {
                processMuscleContent(std::get<MuscleDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Defender: {
                processDefenderContent(std::get<DefenderDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Reconnector: {
                processReconnectorContent(std::get<ReconnectorDescription>(*cell.cellFunction));
            } break;
            case CellFunction_Detonator: {
                processDetonatorContent(std::get<DetonatorDescription>(*cell.cellFunction));
            } break;
            }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

template <typename Description>
void _InspectorWindow::processCellGenomeTab(Description& desc)
{
    auto const& parameters = _simulationFacade->getSimulationParameters();

    int flags = ImGuiTabItemFlags_None;
    if (_selectGenomeTab) {
        flags = flags | ImGuiTabItemFlags_SetSelected;
        _selectGenomeTab = false;
    }
    if (ImGui::BeginTabItem("基因组", nullptr, flags)) {
        if (ImGui::BeginChild("##", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {

            auto previewNodeResult = ImGui::TreeNodeEx("预览（参考配置）", TreeNodeFlags);
            AlienImGui::HelpMarker(Const::GenomePreviewTooltip);
            if (previewNodeResult) {
                if (ImGui::BeginChild("##child", ImVec2(0, scale(200)), true, ImGuiWindowFlags_HorizontalScrollbar)) {
                    auto genomDesc = GenomeDescriptionService::get().convertBytesToDescription(desc.genome);
                    auto previewDesc = PreviewDescriptionService::get().convert(genomDesc, std::nullopt, parameters);
                    std::optional<int> selectedNodeDummy;
                    AlienImGui::ShowPreviewDescription(previewDesc, _genomeZoom, selectedNodeDummy);
                }
                ImGui::EndChild();
                if (AlienImGui::Button("编辑")) {
                    GenomeEditorWindow::get().openTab(GenomeDescriptionService::get().convertBytesToDescription(desc.genome));
                }

                ImGui::SameLine();
                if (AlienImGui::Button(AlienImGui::ButtonParameters().buttonText("注射编辑器中选中的基因组").textWidth(ImGui::GetContentRegionAvail().x))) {
                    printOverlayMessage("基因组已注射");
                    desc.genome = GenomeDescriptionService::get().convertDescriptionToBytes(GenomeEditorWindow::get().getCurrentGenome());
                    if constexpr (std::is_same<Description, ConstructorDescription>()) {
                        desc.genomeCurrentNodeIndex = 0;
                        desc.setNumInheritedGenomeNodes(0);
                    }
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("属性（整个基因组）", TreeNodeFlags)) {
                auto numNodes = toInt(GenomeDescriptionService::get().getNumNodesRecursively(desc.genome, true));
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters()
                        .name("细胞数量")
                        .textWidth(GenomeTabTextWidth)
                        .readOnly(true)
                        .tooltip(Const::GenomeNumCellsRecursivelyTooltip),
                    numNodes);

                auto numBytes = toInt(desc.genome.size());
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters()
                        .name("比特")
                        .textWidth(GenomeTabTextWidth)
                        .readOnly(true)
                        .tooltip(Const::GenomeBytesTooltip),
                    numBytes);

                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters().name("代际").textWidth(GenomeTabTextWidth).tooltip(Const::GenomeGenerationTooltip),
                    desc.genomeGeneration);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("属性（主基因组）", TreeNodeFlags)) {

                auto genomeDesc = GenomeDescriptionService::get().convertBytesToDescription(desc.genome);
                auto numBranches= genomeDesc.header.numBranches;
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters()
                        .name("分支的数量")
                        .textWidth(GenomeTabTextWidth)
                        .readOnly(true)
                        .tooltip(Const::GenomeNumBranchesTooltip),
                    numBranches);

                auto numRepetitions = genomeDesc.header.numRepetitions;
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters()
                        .name("每个分支的重复次数")
                        .textWidth(GenomeTabTextWidth)
                        .infinity(true)
                        .readOnly(true)
                        .tooltip(Const::GenomeRepetitionsPerBranchTooltip),
                    numRepetitions);

                auto numNodes = toInt(genomeDesc.cells.size());
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters()
                        .name("每个重复的细胞数量")
                        .textWidth(GenomeTabTextWidth)
                        .readOnly(true)
                        .tooltip(Const::GenomeNumCellsTooltip),
                    numNodes);

                if constexpr (std::is_same<Description, ConstructorDescription>()) {
                    AlienImGui::InputInt(
                        AlienImGui::InputIntParameters()
                            .name("当前分支的索引")
                            .textWidth(GenomeTabTextWidth).tooltip(Const::GenomeCurrentBranchTooltip),
                        desc.currentBranch);
                    AlienImGui::InputInt(
                        AlienImGui::InputIntParameters()
                            .name("当前重复次数的索引")
                            .textWidth(GenomeTabTextWidth)
                            .tooltip(Const::GenomeCurrentRepetitionTooltip),
                        desc.genomeCurrentRepetition);
                    AlienImGui::InputInt(
                        AlienImGui::InputIntParameters().name("当前细胞索引").textWidth(GenomeTabTextWidth).tooltip(Const::GenomeCurrentCellTooltip),
                        desc.genomeCurrentNodeIndex);
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

void _InspectorWindow::processCellMetadataTab(CellDescription& cell)
{
    if (ImGui::BeginTabItem("注释", nullptr, ImGuiTabItemFlags_None)) {
        if (ImGui::BeginChild("##", ImVec2(0, 0), false, 0)) {
            AlienImGui::InputText(AlienImGui::InputTextParameters().hint("学名").textWidth(0), cell.metadata.name);

            AlienImGui::InputTextMultiline(AlienImGui::InputTextMultilineParameters().hint("笔记").textWidth(0).height(100), cell.metadata.description);
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

void _InspectorWindow::processNerveContent(NerveDescription& nerve)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {

        bool pulseGeneration = nerve.pulseMode > 0;
        if (AlienImGui::Checkbox(AlienImGui::CheckboxParameters().name("生成神经脉冲").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeNerveGeneratePulsesTooltip), pulseGeneration)) {
            nerve.pulseMode = pulseGeneration ? 1 : 0;
        }
        if (pulseGeneration) {
            AlienImGui::InputInt(AlienImGui::InputIntParameters().name("神经脉冲间隔").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeNervePulseIntervalTooltip), nerve.pulseMode);
            bool alternation = nerve.alternationMode > 0;
            if (AlienImGui::Checkbox(
                    AlienImGui::CheckboxParameters().name("交替脉冲").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeNerveAlternatingPulsesTooltip),
                    alternation)) {
                nerve.alternationMode = alternation ? 1 : 0;
            }
            if (alternation) {
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters().name("每个阶段的脉冲数").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeNervePulsesPerPhaseTooltip),
                    nerve.alternationMode);
            }
        }
        ImGui::TreePop();
    }
}

void _InspectorWindow::processNeuronContent(NeuronDescription& neuron)
{
    if (ImGui::TreeNodeEx("神经网络", TreeNodeFlags)) {
        AlienImGui::NeuronSelection(
            AlienImGui::NeuronSelectionParameters().rightMargin(0), neuron.weights, neuron.biases, neuron.activationFunctions);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processConstructorContent(ConstructorDescription& constructor)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        int constructorMode = constructor.activationMode == 0 ? 0 : 1;
        if (AlienImGui::Combo(
                AlienImGui::ComboParameters()
                    .name("激活模型")
                    .textWidth(CellFunctionTextWidth)
                    .values({"手动", "自动"})
                    .tooltip(Const::GenomeConstructorActivationModeTooltip),
                constructorMode)) {
            constructor.activationMode = constructorMode;
        }
        if (constructorMode == 1) {
            AlienImGui::InputInt(
                AlienImGui::InputIntParameters().name("间隔").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeConstructorIntervalTooltip),
                constructor.activationMode);
        }
        AlienImGui::InputInt(
            AlienImGui::InputIntParameters()
                .name("后代激活时间")
                .textWidth(CellFunctionTextWidth)
                .tooltip(Const::GenomeConstructorOffspringActivationTime),
            constructor.constructionActivationTime);
        AlienImGui::InputFloat(
            AlienImGui::InputFloatParameters()
                .name("构筑角度 #1")
                .textWidth(CellFunctionTextWidth)
                .format("%.1f")
                .tooltip(Const::GenomeConstructorAngle1Tooltip),
            constructor.constructionAngle1);
        AlienImGui::InputFloat(
            AlienImGui::InputFloatParameters()
                .name("构筑角度 #2")
                .textWidth(CellFunctionTextWidth)
                .format("%.1f")
                .tooltip(Const::GenomeConstructorAngle2Tooltip),
            constructor.constructionAngle2);
        ImGui::TreePop();

    }
}

void _InspectorWindow::processInjectorContent(InjectorDescription& injector)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("模式")
                .textWidth(CellFunctionTextWidth)
                .values({"仅空细胞", "所有细胞"})
                .tooltip(Const::GenomeInjectorModeTooltip),
            injector.mode);
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("执行数据", TreeNodeFlags)) {
        AlienImGui::InputInt(
            AlienImGui::InputIntParameters().name("计时器").textWidth(CellFunctionTextWidth).tooltip(Const::CellInjectorCounterTooltip), injector.counter);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processAttackerContent(AttackerDescription& attacker)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("能量传输")
                .values({"连接的细胞", "传输器和构筑器"}).tooltip(Const::GenomeAttackerEnergyDistributionTooltip)
                .textWidth(CellFunctionTextWidth),
            attacker.mode);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processDefenderContent(DefenderDescription& defender)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("模式")
                .values({"反攻击器", "反注射器"})
                .textWidth(CellFunctionDefenderWidth)
                .tooltip(Const::GenomeDefenderModeTooltip),
            defender.mode);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processTransmitterContent(TransmitterDescription& transmitter)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("能量传输")
                .values({"连接的细胞", "传输器和构筑器"})
                .tooltip(Const::GenomeTransmitterEnergyDistributionTooltip)
                .textWidth(CellFunctionTextWidth),
            transmitter.mode);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processMuscleContent(MuscleDescription& muscle)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("模式")
                .values({"朝感知到的细胞移动", "扩张和伸缩", "弯曲"})
                .textWidth(CellFunctionTextWidth)
                .tooltip(Const::GenomeMuscleModeTooltip),
            muscle.mode);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processSensorContent(SensorDescription& sensor)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::ComboOptionalColor(
            AlienImGui::ComboColorParameters().name("扫描颜色").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeSensorScanColorTooltip), sensor.restrictToColor);

        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("扫描种群")
                .values({"无", "同一种群", "其他种群", "离散细胞", "人工制作的细胞", "低级种群", "高级种群"})
                .textWidth(CellFunctionTextWidth)
                .tooltip(Const::SensorRestrictToMutantsTooltip),
            sensor.restrictToMutants);
        AlienImGui::InputFloat(
            AlienImGui::InputFloatParameters()
                .name("最小密度")
                .format("%.2f")
                .step(0.05f)
                .textWidth(CellFunctionTextWidth)
                .tooltip(Const::GenomeSensorMinDensityTooltip),
            sensor.minDensity);
        AlienImGui::InputOptionalInt(
            AlienImGui::InputIntParameters().name("最小范围").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeSensorMinRangeTooltip), sensor.minRange);
        AlienImGui::InputOptionalInt(
            AlienImGui::InputIntParameters().name("最大范围").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeSensorMaxRangeTooltip), sensor.maxRange);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processReconnectorContent(ReconnectorDescription& reconnector)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::ComboOptionalColor(
            AlienImGui::ComboColorParameters().name("根据颜色限制").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeReconnectorRestrictToColorTooltip),
            reconnector.restrictToColor);
        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("根据种群限制")
                .values({"无", "同一种群", "其他种群", "离散细胞", "人工制作的细胞", "低级种群", "高级种群"})
                .textWidth(CellFunctionTextWidth)
                .tooltip(Const::ReconnectorRestrictToMutantsTooltip),
            reconnector.restrictToMutants);

        ImGui::TreePop();
    }
}

void _InspectorWindow::processDetonatorContent(DetonatorDescription& detonator)
{
    if (ImGui::TreeNodeEx("属性", TreeNodeFlags)) {
        AlienImGui::Combo(
            AlienImGui::ComboParameters()
                .name("状态")
                .values({"准备中", "活动中", "已爆炸"})
                .textWidth(CellFunctionTextWidth)
                .tooltip(Const::DetonatorStateTooltip),
            detonator.state);

        AlienImGui::InputInt(
            AlienImGui::InputIntParameters().name("倒计时").textWidth(CellFunctionTextWidth).tooltip(Const::GenomeDetonatorCountdownTooltip),
            detonator.countdown);
        ImGui::TreePop();
    }
}

void _InspectorWindow::processParticle(ParticleDescription particle)
{
    auto origParticle = particle;
    auto energy = toFloat(particle.energy);
    AlienImGui::InputFloat(AlienImGui::InputFloatParameters().name("能量").textWidth(ParticleContentTextWidth), energy);

    particle.energy = energy;
    if (particle != origParticle) {
        _simulationFacade->changeParticle(particle);
    }
}

float _InspectorWindow::calcWindowWidth() const
{
    if (isCell()) {
        return StyleRepository::get().scale(CellWindowWidth);
    } else {
        return StyleRepository::get().scale(ParticleWindowWidth);
    }
}

void _InspectorWindow::validateAndCorrect(CellDescription& cell) const
{
    auto const& parameters = _simulationFacade->getSimulationParameters();

    cell.maxConnections = (cell.maxConnections + MAX_CELL_BONDS + 1) % (MAX_CELL_BONDS + 1);
    cell.executionOrderNumber = (cell.executionOrderNumber + parameters.cellNumExecutionOrderNumbers) % parameters.cellNumExecutionOrderNumbers;
    if (cell.inputExecutionOrderNumber) {
        cell.inputExecutionOrderNumber = (*cell.inputExecutionOrderNumber + parameters.cellNumExecutionOrderNumbers) % parameters.cellNumExecutionOrderNumbers;
    }
    cell.stiffness = std::max(0.0f, std::min(1.0f, cell.stiffness));
    cell.energy = std::max(0.0f, cell.energy);
    switch (cell.getCellFunctionType()) {
    case CellFunction_Constructor: {
        auto& constructor = std::get<ConstructorDescription>(*cell.cellFunction);
        auto numNodes = GenomeDescriptionService::get().convertNodeAddressToNodeIndex(constructor.genome, toInt(constructor.genome.size()));
        if (numNodes > 0) {
            constructor.genomeCurrentNodeIndex = ((constructor.genomeCurrentNodeIndex % numNodes) + numNodes) % numNodes;
        } else {
            constructor.genomeCurrentNodeIndex = 0;
        }

        auto numRepetitions = GenomeDescriptionService::get().getNumRepetitions(constructor.genome);
        if (numRepetitions != std::numeric_limits<int>::max()) {
            constructor.genomeCurrentRepetition = ((constructor.genomeCurrentRepetition % numRepetitions) + numRepetitions) % numRepetitions;
        } else {
            constructor.genomeCurrentRepetition = 0;
        }

        constructor.constructionActivationTime = ((constructor.constructionActivationTime % MaxActivationTime) + MaxActivationTime) % MaxActivationTime;
        if (constructor.constructionActivationTime < 0) {
            constructor.constructionActivationTime = 0;
        }
        if (constructor.activationMode < 0) {
            constructor.activationMode = 0;
        }
        //if (constructor.maxConnections) {
        //    constructor.maxConnections = (*constructor.maxConnections + MAX_CELL_BONDS + 1) % (MAX_CELL_BONDS + 1);
        //}
        //constructor.stiffness = std::max(0.0f, std::min(1.0f, constructor.stiffness));
        constructor.genomeGeneration = std::max(0, constructor.genomeGeneration);
    } break;
    case CellFunction_Sensor: {
        auto& sensor = std::get<SensorDescription>(*cell.cellFunction);
        sensor.minDensity = std::max(0.0f, std::min(1.0f, sensor.minDensity));
        if (sensor.minRange) {
            sensor.minRange = std::max(0, std::min(127, *sensor.minRange));
        }
        if (sensor.maxRange) {
            sensor.maxRange = std::max(0, std::min(127, *sensor.maxRange));
        }
    } break;
    case CellFunction_Nerve: {
        auto& nerve = std::get<NerveDescription>(*cell.cellFunction);
        nerve.pulseMode = std::max(0, nerve.pulseMode);
        nerve.alternationMode = std::max(0, nerve.alternationMode);
    } break;
    case CellFunction_Detonator: {
        auto& detonator = std::get<DetonatorDescription>(*cell.cellFunction);
        detonator.countdown = std::min(65535, std::max(0, detonator.countdown));
    } break;
    }
}
