#include "TemporalControlWindow.h"

#include <imgui.h>

#include "Fonts/IconsFontAwesome5.h"

#include "Base/Definitions.h"
#include "Base/StringHelper.h"
#include "EngineInterface/SimulationFacade.h"
#include "EngineInterface/SpaceCalculator.h"

#include "StyleRepository.h"
#include "StatisticsWindow.h"
#include "AlienImGui.h"
#include "DelayedExecutionController.h"
#include "OverlayController.h"

namespace
{
    auto constexpr LeftColumnWidth = 180.0f;
}

void TemporalControlWindow::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;
}

void TemporalControlWindow::onSnapshot()
{
    _snapshot = createSnapshot();
}

TemporalControlWindow::TemporalControlWindow()
    : AlienWindow("时间控制器", "windows.temporal control", true)
{
}

void TemporalControlWindow::processIntern()
{
    processRunButton();
    ImGui::SameLine();
    processPauseButton();
    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();
    ImGui::SameLine();
    processStepBackwardButton();
    ImGui::SameLine();
    processStepForwardButton();
    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();
    ImGui::SameLine();
    processCreateFlashbackButton();
    ImGui::SameLine();
    processLoadFlashbackButton();

    AlienImGui::Separator();

    if (ImGui::BeginChild("##", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        processTpsInfo();
        processTotalTimestepsInfo();
        processRealTimeInfo();

        AlienImGui::Separator();
        processTpsRestriction();
    }
    ImGui::EndChild();
}

void TemporalControlWindow::processTpsInfo()
{
    ImGui::Text("平均每秒运行的步数");

    ImGui::PushFont(StyleRepository::get().getLargeFont());
    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor /*0xffa07050*/);
    ImGui::TextUnformatted(StringHelper::format(_simulationFacade->getTps(), 1).c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

void TemporalControlWindow::processTotalTimestepsInfo()
{
    ImGui::Text("总计运行的步数");

    ImGui::PushFont(StyleRepository::get().getLargeFont());
    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor);
    ImGui::TextUnformatted(StringHelper::format(_simulationFacade->getCurrentTimestep()).c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

void TemporalControlWindow::processRealTimeInfo()
{
    ImGui::Text("真实时间");

    ImGui::PushFont(StyleRepository::get().getLargeFont());
    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor);
    ImGui::TextUnformatted(StringHelper::format(_simulationFacade->getRealTime()).c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

void TemporalControlWindow::processTpsRestriction()
{
    AlienImGui::ToggleButton(AlienImGui::ToggleButtonParameters().name("降速至"), _slowDown);
    ImGui::SameLine(scale(LeftColumnWidth) - (ImGui::GetWindowWidth() - ImGui::GetContentRegionAvail().x));
    ImGui::BeginDisabled(!_slowDown);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::SliderInt("##TPSRestriction", &_tpsRestriction, 1, 1000, "%d TPS", ImGuiSliderFlags_Logarithmic);
    if (_slowDown) {
        _simulationFacade->setTpsRestriction(_tpsRestriction);
    } else {
        _simulationFacade->setTpsRestriction(std::nullopt);
    }
    ImGui::PopItemWidth();
    ImGui::EndDisabled();

    auto syncSimulationWithRendering = _simulationFacade->isSyncSimulationWithRendering();
    if (AlienImGui::ToggleButton(AlienImGui::ToggleButtonParameters().name("与渲染同步"), syncSimulationWithRendering)) {
        _simulationFacade->setSyncSimulationWithRendering(syncSimulationWithRendering);
    }

    ImGui::BeginDisabled(!syncSimulationWithRendering);
    ImGui::SameLine(scale(LeftColumnWidth) - (ImGui::GetWindowWidth() - ImGui::GetContentRegionAvail().x));
    auto syncSimulationWithRenderingRatio = _simulationFacade->getSyncSimulationWithRenderingRatio();
    if (AlienImGui::SliderInt(AlienImGui::SliderIntParameters().textWidth(0).min(1).max(40).logarithmic(true).format("%d TPS : FPS"), &syncSimulationWithRenderingRatio)) {
        _simulationFacade->setSyncSimulationWithRenderingRatio(syncSimulationWithRenderingRatio);
    }
    ImGui::EndDisabled();
}

void TemporalControlWindow::processRunButton()
{
    ImGui::BeginDisabled(_simulationFacade->isSimulationRunning());
    auto result = AlienImGui::ToolbarButton(ICON_FA_PLAY);
    AlienImGui::Tooltip("运行");
    if (result) {
        _history.clear();
        _simulationFacade->runSimulation();
        printOverlayMessage("运行");
    }
    ImGui::EndDisabled();
}

void TemporalControlWindow::processPauseButton()
{
    ImGui::BeginDisabled(!_simulationFacade->isSimulationRunning());
    auto result = AlienImGui::ToolbarButton(ICON_FA_PAUSE);
    AlienImGui::Tooltip("暂停");
    if (result) {
        _simulationFacade->pauseSimulation();
        printOverlayMessage("暂停");
    }
    ImGui::EndDisabled();
}

void TemporalControlWindow::processStepBackwardButton()
{
    ImGui::BeginDisabled(_history.empty() || _simulationFacade->isSimulationRunning());
    auto result = AlienImGui::ToolbarButton(ICON_FA_CHEVRON_LEFT);
    AlienImGui::Tooltip("载入上一个步数");
    if (result) {
        auto const& snapshot = _history.back();
        delayedExecution([this, snapshot] { applySnapshot(snapshot); });
        printOverlayMessage("载入上一个步数中...");

        _history.pop_back();
    }
    ImGui::EndDisabled();
}

void TemporalControlWindow::processStepForwardButton()
{
    ImGui::BeginDisabled(_simulationFacade->isSimulationRunning());
    auto result = AlienImGui::ToolbarButton(ICON_FA_CHEVRON_RIGHT);
    AlienImGui::Tooltip("执行单个步数");
    if (result) {
        _history.emplace_back(createSnapshot());
        _simulationFacade->calcTimesteps(1);
    }
    ImGui::EndDisabled();
}

void TemporalControlWindow::processCreateFlashbackButton()
{
    auto result = AlienImGui::ToolbarButton(ICON_FA_CAMERA);
    AlienImGui::Tooltip("创建内容快照：它将当前世界的内容保存到内存中。");
    if (result) {
        delayedExecution([this] { onSnapshot(); });
        
        printOverlayMessage("创建快照中...", true);
    }
}

void TemporalControlWindow::processLoadFlashbackButton()
{
    ImGui::BeginDisabled(!_snapshot);
    auto result = AlienImGui::ToolbarButton(ICON_FA_UNDO);
    AlienImGui::Tooltip("加载内容快照：它会从内存中加载已保存的世界。静态模拟参数不会被改变。非静态参数（例如移动区域的位置）也会被恢复。");
    if (result) {
        delayedExecution([this] { applySnapshot(*_snapshot); });
        _simulationFacade->removeSelection();
        _history.clear();

        printOverlayMessage("载入快照中...", true);
    }
    ImGui::EndDisabled();
}

TemporalControlWindow::Snapshot TemporalControlWindow::createSnapshot()
{
    Snapshot result;
    result.timestep = _simulationFacade->getCurrentTimestep();
    result.realTime = _simulationFacade->getRealTime();
    result.data = _simulationFacade->getSimulationData();
    result.parameters = _simulationFacade->getSimulationParameters();
    return result;
}


void TemporalControlWindow::applySnapshot(Snapshot const& snapshot)
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto const& origParameters = snapshot.parameters;

    if (origParameters.numRadiationSources == parameters.numRadiationSources) {
        for (int i = 0; i < parameters.numRadiationSources; ++i) {
            restorePosition(parameters.radiationSources[i], origParameters.radiationSources[i], snapshot.timestep);
        }
    }

    if (origParameters.numSpots == parameters.numSpots) {
        for (int i = 0; i < parameters.numSpots; ++i) {
            restorePosition(parameters.spots[i], origParameters.spots[i], snapshot.timestep);
        }
    }

    parameters.externalEnergy = origParameters.externalEnergy;
    if (parameters.cellMaxAgeBalancer || origParameters.cellMaxAgeBalancer) {
        for (int i = 0; i < MAX_COLORS; ++i) {
            parameters.cellMaxAge[i] = origParameters.cellMaxAge[i];
        }
    }
    _simulationFacade->setCurrentTimestep(snapshot.timestep);
    _simulationFacade->setRealTime(snapshot.realTime);
    _simulationFacade->clear();
    _simulationFacade->setSimulationData(snapshot.data);
    _simulationFacade->setSimulationParameters(parameters);
}

template <typename MovedObjectType>
void TemporalControlWindow::restorePosition(MovedObjectType& movedObject, MovedObjectType const& origMovedObject, uint64_t origTimestep)
{
    auto origMovedObjectClone = origMovedObject;
    auto movedObjectClone = movedObject;

    if (std::abs(movedObject.velX) > NEAR_ZERO || std::abs(movedObject.velY) > NEAR_ZERO || std::abs(origMovedObject.velX) > NEAR_ZERO
        || std::abs(origMovedObject.velY) > NEAR_ZERO) {
        movedObject.posX = origMovedObject.posX;
        movedObject.posY = origMovedObject.posY;
    }
}
