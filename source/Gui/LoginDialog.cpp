#include "LoginDialog.h"

#include <imgui.h>

#include "Base/GlobalSettings.h"
#include "EngineInterface/SimulationFacade.h"
#include "Network/NetworkService.h"

#include "AlienImGui.h"
#include "GenericMessageDialog.h"
#include "CreateUserDialog.h"
#include "BrowserWindow.h"
#include "ResetPasswordDialog.h"
#include "ActivateUserDialog.h"
#include "StyleRepository.h"
#include "HelpStrings.h"
#include "LoginController.h"

void LoginDialog::initIntern(SimulationFacade simulationFacade, PersisterFacade persisterFacade)
{
    _simulationFacade = simulationFacade;
    _persisterFacade = persisterFacade;
}

LoginDialog::LoginDialog()
    : AlienDialog("登录")
{}

void LoginDialog::processIntern()
{
    AlienImGui::Text("如何创建一个新用户？");
    AlienImGui::HelpMarker(Const::LoginHowToCreateNewUseTooltip);

    AlienImGui::Text("忘记密码?");
    AlienImGui::HelpMarker(Const::LoginForgotYourPasswordTooltip);

    AlienImGui::Text("安全信息");
    AlienImGui::HelpMarker(Const::LoginSecurityInformationTooltip);

    AlienImGui::Separator();

    auto& loginController = LoginController::get();
    auto userName = loginController.getUserName();
    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("用户名称").textWidth(0), userName);
    loginController.setUserName(userName);

    auto password= loginController.getPassword();
    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("密码").password(true).textWidth(0), password);
    loginController.setPassword(password);

    AlienImGui::Separator();
    ImGui::Spacing();

    auto remember = loginController.isRemember();
    AlienImGui::ToggleButton(AlienImGui::ToggleButtonParameters().name("记住").tooltip(Const::LoginRememberTooltip), remember);
    loginController.setRemember(remember);

    auto shareGpuInfo = loginController.shareGpuInfo();
    AlienImGui::ToggleButton(
        AlienImGui::ToggleButtonParameters()
            .name("分享GPU信息")
            .tooltip(Const::LoginShareGpuInfoTooltip1 + _simulationFacade->getGpuName() + "\n" + Const::LoginShareGpuInfoTooltip2),
        shareGpuInfo);
    loginController.setShareGpuInfo(shareGpuInfo);

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienImGui::Separator();

    ImGui::BeginDisabled(userName.empty() || password.empty());
    if (AlienImGui::Button("登录")) {
        close();
        loginController.onLogin();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    AlienImGui::VerticalSeparator();

    ImGui::SameLine();
    ImGui::BeginDisabled(userName.empty() || password.empty());
    if (AlienImGui::Button("创建用户")) {
        close();
        CreateUserDialog::get().open(userName, password, LoginController::get().getUserInfo());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(userName.empty());
    if (AlienImGui::Button("重设密码")) {
        close();
        ResetPasswordDialog::get().open(userName, LoginController::get().getUserInfo());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    AlienImGui::VerticalSeparator();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
}
