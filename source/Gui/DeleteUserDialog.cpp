#include "DeleteUserDialog.h"

#include <imgui.h>

#include "Network/NetworkService.h"

#include "AlienImGui.h"
#include "BrowserWindow.h"
#include "CreateUserDialog.h"
#include "GenericMessageDialog.h"

DeleteUserDialog::DeleteUserDialog()
    : AlienDialog("删除用户")
{}

void DeleteUserDialog::processIntern()
{
    AlienImGui::Text(
        "警告: 该用户 '" + *NetworkService::get().getLoggedInUserName()
        + "'的所有数据将在服务器中删除。\n这些包括收藏，模拟器和账户数据。");
    AlienImGui::Separator();

    AlienImGui::InputText(AlienImGui::InputTextParameters().hint("重新输入密码").password(true).textWidth(0), _reenteredPassword);
    AlienImGui::Separator();

    ImGui::BeginDisabled(_reenteredPassword.empty());
    if (AlienImGui::Button("删除")) {
        close();
        if (_reenteredPassword == *NetworkService::get().getPassword()) {
            onDelete();
        } else {
            GenericMessageDialog::get().information("错误", "密码不匹配。");
        }
        _reenteredPassword.clear();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
        _reenteredPassword.clear();
    }
}

void DeleteUserDialog::onDelete()
{
    auto userName = *NetworkService::get().getLoggedInUserName();
    if (NetworkService::get().deleteUser()) {
        BrowserWindow::get().onRefresh();
        GenericMessageDialog::get().information("信息", "用户'" + userName + "'已经被删除\n你已经登出。");
    } else {
        GenericMessageDialog::get().information("错误", "一个错误发生在服务器。");
    }
}
