#include "SelectionWindow.h"

#include <imgui.h>

#include "Base/StringHelper.h"

#include "EditorController.h"
#include "StyleRepository.h"
#include "EditorModel.h"

SelectionWindow::SelectionWindow()
    : AlienWindow("所选项", "windows.selection", true)
{}

void SelectionWindow::processIntern()
{
    auto selection = EditorModel::get().getSelectionShallowData();
    ImGui::Text("细胞数量");
    ImGui::PushFont(StyleRepository::get().getLargeFont());
    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor);
    ImGui::TextUnformatted(StringHelper::format(selection.numCells).c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Text("连接的细胞数量");
    ImGui::PushFont(StyleRepository::get().getLargeFont());
    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor);
    ImGui::TextUnformatted(StringHelper::format(selection.numClusterCells).c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Text("能量粒子细胞数量");
    ImGui::PushFont(StyleRepository::get().getLargeFont());
    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor);
    ImGui::TextUnformatted(StringHelper::format(selection.numParticles).c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

bool SelectionWindow::isShown()
{
    return _on && EditorController::get().isOn();
}
