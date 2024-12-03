#include <imgui.h>

#include "ResizeWorldDialog.h"

#include "EngineInterface/DescriptionEditService.h"
#include "EngineInterface/SimulationFacade.h"

#include "AlienImGui.h"
#include "TemporalControlWindow.h"

void ResizeWorldDialog::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;
}

void ResizeWorldDialog::open()
{
    AlienDialog::open();

    auto worldSize = _simulationFacade->getWorldSize();

    _width = worldSize.x;
    _height = worldSize.y;
}

ResizeWorldDialog::ResizeWorldDialog()
    : AlienDialog("修改世界大小")
{}

void ResizeWorldDialog::processIntern()
{
    if (ImGui::BeginTable("##", 2, ImGuiTableFlags_SizingStretchProp)) {

        //width
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputInt("##width", &_width);
        ImGui::PopItemWidth();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("宽度");

        //height
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputInt("##height", &_height);
        ImGui::PopItemWidth();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("高度");

        ImGui::EndTable();
    }
    AlienImGui::ToggleButton(AlienImGui::ToggleButtonParameters().name("缩放内容"), _scaleContent);

    AlienImGui::Separator();

    if (AlienImGui::Button("确认")) {
        onResizing();
        close();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }

    _width = std::max(1, _width);
    _height = std::max(1, _height);
}

void ResizeWorldDialog::onResizing()
{
    auto timestep = _simulationFacade->getCurrentTimestep();
    auto generalSettings = _simulationFacade->getGeneralSettings();
    auto parameters = _simulationFacade->getSimulationParameters();
    auto content = _simulationFacade->getClusteredSimulationData();
    auto realtime = _simulationFacade->getRealTime();
    auto const& statistics = _simulationFacade->getStatisticsHistory().getCopiedData();
    _simulationFacade->closeSimulation();

    IntVector2D origWorldSize{generalSettings.worldSizeX, generalSettings.worldSizeY};
    generalSettings.worldSizeX = _width;
    generalSettings.worldSizeY = _height;

    _simulationFacade->newSimulation(timestep, generalSettings, parameters);

    DescriptionEditService::get().correctConnections(content, {_width, _height});
    if (_scaleContent) {
        DescriptionEditService::get().duplicate(content, origWorldSize, {_width, _height});
    }
    _simulationFacade->setClusteredSimulationData(content);
    _simulationFacade->setStatisticsHistory(statistics);
    _simulationFacade->setRealTime(realtime);
    TemporalControlWindow::get().onSnapshot();
}
