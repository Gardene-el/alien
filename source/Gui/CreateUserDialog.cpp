#include "CreateUserDialog.h"

#include <imgui.h>
#include <Fonts/IconsFontAwesome5.h>

#include "Network/NetworkService.h"

#include "AlienImGui.h"
#include "GenericMessageDialog.h"
#include "ActivateUserDialog.h"

CreateUserDialog::CreateUserDialog()
    : AlienDialog("创建用户")
{}

void CreateUserDialog::open(std::string const& userName, std::string const& password, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _password = password;
    _email.clear();
    _userInfo = userInfo;
}

void CreateUserDialog::processIntern()
{
    AlienImGui::Text("安全信息");
    AlienImGui::HelpMarker("传输至服务器的数据已通过https加密。在服务端，邮箱地址并不储存在明文中，是以加密为SHA-256哈希值的形式储存在数据库中。");
    AlienImGui::Text("数据隐私政策");
    AlienImGui::HelpMarker("被输入的电子邮箱地址将不会被发送给第三方，并且仅用于下述两个原因："
                           "1)用于发送确认码。 "
                           "2)将以SHA-256哈希值加密的形式储存在服务器，用于确认该电子邮箱是否已经在ALIEN项目中被使用。");
    AlienImGui::Separator();
    AlienImGui::Text("请输入您的电子邮箱地址以接收用于新用户注册的确认码。");
    AlienImGui::Separator();
    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("电子邮箱").textWidth(0), _email);

    AlienImGui::Separator();

    ImGui::BeginDisabled(_email.empty());
    if (AlienImGui::Button("创建用户")) {
        close();

        onCreateUser();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
}

void CreateUserDialog::onCreateUser()
{
    if (NetworkService::get().createUser(_userName, _password, _email)) {
        ActivateUserDialog::get().open(_userName, _password, _userInfo);
    } else {
        GenericMessageDialog::get().information(
            "错误",
            "服务器中发生了一个错误。这可被认为是\n" ICON_FA_CARET_RIGHT
            "您的用户名或电子邮箱已经在被使用，\n" ICON_FA_CARET_RIGHT "或者您的用户名包括空格。");
    }
}
