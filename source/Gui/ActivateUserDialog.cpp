#include "ActivateUserDialog.h"

#include <imgui.h>

#include "EngineInterface/SimulationFacade.h"
#include "Network/NetworkService.h"

#include "AlienImGui.h"
#include "GenericMessageDialog.h"
#include "BrowserWindow.h"
#include "CreateUserDialog.h"
#include "StyleRepository.h"

void ActivateUserDialog::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;
}

void ActivateUserDialog::open(std::string const& userName, std::string const& password, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _password = password;
    _userInfo = userInfo;
}

ActivateUserDialog::ActivateUserDialog()
    : AlienDialog("启用用户")
{}

void ActivateUserDialog::processIntern()
{
    AlienImGui::Text("请输入发送至您邮箱的确认码");
    AlienImGui::HelpMarker(
        "如果您没有找到对应的邮件，请查看您邮箱的垃圾邮件文件夹。 如果您确实没有收到邮件，请尝试以其他可能的邮箱地址注册。"
        "如果您的邮箱仍然无法接收邮件，请联系通知@alien-project.org。");
    AlienImGui::Separator();
    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("确认码（区分大小写）").textWidth(0), _confirmationCode);

    AlienImGui::Separator();

    ImGui::BeginDisabled(_confirmationCode.empty());
    if (AlienImGui::Button("确认")) {
        close();
        onActivateUser();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    AlienImGui::VerticalSeparator();

    ImGui::SameLine();
    if (AlienImGui::Button("重新发送")) {
        CreateUserDialog::get().onCreateUser();
    }

    ImGui::SameLine();
    if (AlienImGui::Button("重新发送至其他邮箱")) {
        close();
        CreateUserDialog::get().open(_userName, _password, _userInfo);
    }

    ImGui::SameLine();
    AlienImGui::VerticalSeparator();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
}

void ActivateUserDialog::onActivateUser()
{
    auto result = NetworkService::get().activateUser(_userName, _password, _userInfo, _confirmationCode);
    if (result) {
        LoginErrorCode errorCode;
        result |= NetworkService::get().login(errorCode, _userName, _password, _userInfo);
    }
    if (!result) {
        GenericMessageDialog::get().information("错误", "服务器中发生了一个错误。你输入的确认码可能不正确。\n请重新尝试注册。");
    } else {
        GenericMessageDialog::get().information(
            "信息",
            "用户'" + _userName
                + "'账户已经成功创建。\n你已经登录并且现在能够上传你的模拟器\n或者根据喜好点在别人的模拟器。");
        BrowserWindow::get().onRefresh();
    }
}
