#include <imgui.h>

#include "ExitDialog.h"
#include "AlienImGui.h"
#include "MainLoopController.h"

ExitDialog::ExitDialog()
    : AlienDialog("退出")
{}

void ExitDialog::processIntern()
{
    ImGui::TextWrapped("%s", "你真的想要终止程序吗？");

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienImGui::Separator();

    if (AlienImGui::Button("确认")) {
        MainLoopController::get().scheduleClosing();
        close();
    }
    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
    ImGui::SetItemDefaultFocus();
}
