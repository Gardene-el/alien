#include "NetworkSettingsDialog.h"

#include <imgui.h>

#include "Network/NetworkService.h"

#include "AlienImGui.h"
#include "BrowserWindow.h"
#include "StyleRepository.h"

namespace
{
    auto const RightColumnWidth = 150.0f;
}

NetworkSettingsDialog::NetworkSettingsDialog()
    : AlienDialog("网络设置")
{}

void NetworkSettingsDialog::processIntern()
{
    AlienImGui::InputText(
        AlienImGui::InputTextParameters().name("Blocks").defaultValue(_origServerAddress).name("服务器网址").textWidth(RightColumnWidth), _serverAddress);

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienImGui::Separator();

    if (AlienImGui::Button("确认")) {
        close();
        onChangeSettings();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
}

void NetworkSettingsDialog::openIntern()
{
    _origServerAddress = NetworkService::get().getServerAddress();
    _serverAddress = _origServerAddress;
}

void NetworkSettingsDialog::onChangeSettings()
{
    NetworkService::get().setServerAddress(_serverAddress);
    BrowserWindow::get().onRefresh();
}
