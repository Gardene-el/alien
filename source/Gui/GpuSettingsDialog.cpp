#include "GpuSettingsDialog.h"

#include <imgui.h>

#include "Base/GlobalSettings.h"
#include "Base/StringHelper.h"
#include "EngineInterface/SimulationFacade.h"

#include "StyleRepository.h"
#include "AlienImGui.h"

namespace
{
    auto const RightColumnWidth = 110.0f;
}

void GpuSettingsDialog::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;

    GpuSettings gpuSettings;
    gpuSettings.numBlocks = GlobalSettings::get().getValue("settings.gpu.num blocks", gpuSettings.numBlocks);

    _simulationFacade->setGpuSettings_async(gpuSettings);
}

void GpuSettingsDialog::shutdownIntern()
{
    auto gpuSettings = _simulationFacade->getGpuSettings();
    GlobalSettings::get().setValue("settings.gpu.num blocks", gpuSettings.numBlocks);
}

GpuSettingsDialog::GpuSettingsDialog()
    : AlienDialog("CUDA设置")
{}

void GpuSettingsDialog::processIntern()
{
    auto gpuSettings = _simulationFacade->getGpuSettings();
    auto origGpuSettings = _simulationFacade->getOriginalGpuSettings();
    auto lastGpuSettings = gpuSettings;

    AlienImGui::InputInt(
        AlienImGui::InputIntParameters()
            .name("Blocks")
            .textWidth(RightColumnWidth)
            .defaultValue(origGpuSettings.numBlocks)
            .tooltip("此值指定 CUDA 线程块（blocks）的数量。如果您使用的是高端显卡，可以尝试增加块的数量。"),
        gpuSettings.numBlocks);

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienImGui::Separator();

    if (AlienImGui::Button("确认")) {
        close();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
        gpuSettings = _gpuSettings;
    }

    validateAndCorrect(gpuSettings);

    if (gpuSettings != lastGpuSettings) {
        _simulationFacade->setGpuSettings_async(gpuSettings);
    }
}

void GpuSettingsDialog::openIntern()
{
    _gpuSettings = _simulationFacade->getGpuSettings();
}

void GpuSettingsDialog::validateAndCorrect(GpuSettings& settings) const
{
    settings.numBlocks = std::min(1000000, std::max(16, settings.numBlocks));
}
