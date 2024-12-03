#include "NewSimulationDialog.h"

#include <imgui.h>

#include "Base/GlobalSettings.h"
#include "EngineInterface/SimulationFacade.h"

#include "Viewport.h"
#include "StatisticsWindow.h"
#include "TemporalControlWindow.h"
#include "AlienImGui.h"
#include "StyleRepository.h"

namespace
{
    auto const ContentTextInputWidth = 100.0f;
    auto const ProjectNameSize = sizeof(Char64) / sizeof(char);
}

void NewSimulationDialog::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;
    _adoptSimulationParameters =
            GlobalSettings::get().getValue("dialogs.new simulation.adopt simulation parameters", true);
}

void NewSimulationDialog::shutdownIntern()
{
    GlobalSettings::get().setValue("dialogs.new simulation.adopt simulation parameters", _adoptSimulationParameters);
}

NewSimulationDialog::NewSimulationDialog()
    : AlienDialog("新建模拟器")
{}

void NewSimulationDialog::processIntern()
{
    AlienImGui::InputText(AlienImGui::InputTextParameters().name("模拟器名称").textWidth(ContentTextInputWidth), _projectName, ProjectNameSize);
    AlienImGui::InputInt(AlienImGui::InputIntParameters().name("宽度").textWidth(ContentTextInputWidth), _width);
    AlienImGui::InputInt(AlienImGui::InputIntParameters().name("高度").textWidth(ContentTextInputWidth), _height);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters().name("继承当前的模拟器参数集").textWidth(0), _adoptSimulationParameters);

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienImGui::Separator();
    if (AlienImGui::Button("确认")) {
        ImGui::CloseCurrentPopup();
        onNewSimulation();
        close();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        ImGui::CloseCurrentPopup();
        close();
    }

    _width = std::max(1, _width);
    _height = std::max(1, _height);
}

void NewSimulationDialog::openIntern()
{
    auto worldSize = _simulationFacade->getWorldSize();
    _width = worldSize.x;
    _height = worldSize.y;
}

void NewSimulationDialog::onNewSimulation()
{
    SimulationParameters parameters;
    if (_adoptSimulationParameters) {
        parameters = _simulationFacade->getSimulationParameters();
    }
    for (int i = 0; i < ProjectNameSize; ++i) {
        parameters.projectName[i] = _projectName[i];
    }
    _simulationFacade->closeSimulation();

    GeneralSettings generalSettings;
    generalSettings.worldSizeX = _width;
    generalSettings.worldSizeY = _height;
    _simulationFacade->newSimulation(0, generalSettings, parameters);
    Viewport::get().setCenterInWorldPos({toFloat(_width) / 2, toFloat(_height) / 2});
    Viewport::get().setZoomFactor(4.0f);
    TemporalControlWindow::get().onSnapshot();
}
