#include "AboutDialog.h"

#include <imgui.h>

#include "Base/Resources.h"

#include "AlienImGui.h"
#include "StyleRepository.h"

AboutDialog::AboutDialog()
    : AlienDialog("关于ALIEN")
{}

void AboutDialog::processIntern()
{
    ImGui::Text("Artificial Life Environment, 版本号 %s\n\n是一个由 Christian Heinemann 开发和维护的开源项目。\n\nALIEN项目的中文翻译部分由Garden_eel维护。\n\nArtificial Life Environment, version %s\n\nis an open source project initiated and maintained by\nChristian Heinemann.\n\n The Chinese translation is maintained by Garden_eel.", Const::ProgramVersion.c_str(),Const::ProgramVersion.c_str());

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienImGui::Separator();

    if (AlienImGui::Button("确认")) {
        close();
    }
    ImGui::SetItemDefaultFocus();
}
