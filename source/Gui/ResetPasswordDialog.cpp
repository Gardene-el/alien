#include "ResetPasswordDialog.h"

#include <imgui.h>

#include "AlienImGui.h"
#include "GenericMessageDialog.h"
#include "NewPasswordDialog.h"

ResetPasswordDialog::ResetPasswordDialog()
    : AlienDialog("重设密码")
{}

void ResetPasswordDialog::open(std::string const& userName, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _email.clear();
    _userInfo = userInfo;
}

void ResetPasswordDialog::processIntern()
{
    AlienImGui::Text("安全信息");
    AlienImGui::HelpMarker("传输至服务器的数据已通过https加密。在服务端，邮箱地址并不储存在明文中，是以加密为SHA-256哈希值的形式储存在数据库中。");
    AlienImGui::Text("数据隐私政策");
    AlienImGui::HelpMarker("被输入的电子邮箱地址将不会被发送给第三方，并且仅用于下述两个原因："
                           "1)用于发送确认码。 "
                           "2)将以SHA-256哈希值加密的形式储存在服务器，用于确认该电子邮箱是否已经在ALIEN项目中被使用。");
    AlienImGui::Separator();
    AlienImGui::Text("请输入您的电子邮箱地址以接收用于重新设置密码的确认码。");
    AlienImGui::Separator();
    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("电子邮箱").textWidth(0), _email);

    AlienImGui::Separator();

    ImGui::BeginDisabled(_email.empty());
    if (AlienImGui::Button("确认")) {
        close();

        onResetPassword();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
}

void ResetPasswordDialog::onResetPassword()
{
    if (NetworkService::get().resetPassword(_userName, _email)) {
        NewPasswordDialog::get().open(_userName, _userInfo);
    } else {
        GenericMessageDialog::get().information(
           "错误","服务器中发生了一个错误。这可被认为是\n您给出的电子邮箱地址错误。");
    }
}
