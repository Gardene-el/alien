#include "NewPasswordDialog.h"

#include <imgui.h>

#include "EngineInterface/SimulationFacade.h"
#include "Network/NetworkService.h"

#include "AlienImGui.h"
#include "BrowserWindow.h"
#include "GenericMessageDialog.h"

void NewPasswordDialog::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;
}

void NewPasswordDialog::open(std::string const& userName, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _newPassword.clear();
    _confirmationCode.clear();
    _userInfo = userInfo;
}

NewPasswordDialog::NewPasswordDialog()
    : AlienDialog("新密码")
{}

void NewPasswordDialog::processIntern()
{
    AlienImGui::Text("安全信息");
    AlienImGui::HelpMarker(
        "与服务器的数据传输通过 https 加密。在服务器端，密码不是以明文存储，而是以加盐的 SHA-256 哈希值存储在数据库中。");

    AlienImGui::Separator();

    AlienImGui::Text("请输入新密码以及发送到您邮箱的确认码。");
    AlienImGui::Separator();
    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("新密码").password(true).textWidth(0), _newPassword);
    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("确认码（区分大小写）").textWidth(0), _confirmationCode);

    AlienImGui::Separator();

    ImGui::BeginDisabled(_confirmationCode.empty());
    if (AlienImGui::Button("确定")) {
        close();
        onNewPassword();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
}

void NewPasswordDialog::onNewPassword()
{
    auto result = NetworkService::get().setNewPassword(_userName, _newPassword, _confirmationCode);
    if (result) {
        LoginErrorCode errorCode;
        result |= NetworkService::get().login(errorCode, _userName, _newPassword, _userInfo);
    }
    if (!result) {
        GenericMessageDialog::get().information("错误", "服务器上发生错误。您输入的确认码可能不正确。\n请尝试重新重置密码。");
        return;
    }
    GenericMessageDialog::get().information("信息", "密码已成功设置。\n您已登录。");
    BrowserWindow::get().onRefresh();
}
