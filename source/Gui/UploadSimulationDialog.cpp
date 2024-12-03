#include "UploadSimulationDialog.h"

#include <imgui.h>

#include "Base/GlobalSettings.h"
#include "PersisterInterface/SerializerService.h"
#include "Network/NetworkService.h"
#include "Network/NetworkValidationService.h"

#include "AlienImGui.h"
#include "GenericMessageDialog.h"
#include "StyleRepository.h"
#include "BrowserWindow.h"
#include "EditorController.h"
#include "Viewport.h"
#include "GenomeEditorWindow.h"
#include "HelpStrings.h"
#include "LoginDialog.h"
#include "NetworkTransferController.h"

namespace
{
    auto constexpr FolderWidgetHeight = 50.0f;

    std::map<NetworkResourceType, std::string> const BrowserDataTypeToLowerString = {
        {NetworkResourceType_Simulation, "模拟器"},
        {NetworkResourceType_Genome, "基因组"}};
    std::map<NetworkResourceType, std::string> const BrowserDataTypeToUpperString = {
        {NetworkResourceType_Simulation, "模拟器"},
        {NetworkResourceType_Genome, "基因组"}};
}

void UploadSimulationDialog::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;

    auto& settings = GlobalSettings::get();
    _share = settings.getValue("dialogs.upload.share", _share);
}

void UploadSimulationDialog::shutdownIntern()
{
    auto& settings = GlobalSettings::get();
    settings.setValue("dialogs.upload.share", _share);
}


void UploadSimulationDialog::open(NetworkResourceType resourceType, std::string const& folder)
{
    if (NetworkService::get().getLoggedInUserName()) {
        changeTitle("上传" + BrowserDataTypeToLowerString.at(resourceType));
        _resourceType = resourceType;
        _folder = folder;
        _resourceName = _resourceNameByFolder[_folder];
        _resourceDescription = _resourceDescriptionByFolder[_folder];
        AlienDialog::open();
    } else {
        LoginDialog::get().open();
    }
}

UploadSimulationDialog::UploadSimulationDialog()
    : AlienDialog("")
{}

void UploadSimulationDialog::processIntern()
{
    auto resourceTypeString = BrowserDataTypeToLowerString.at(_resourceType);
    if (ImGui::BeginChild("##header", ImVec2(0, scale(52.0f)), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        AlienImGui::Text("数据隐私政策");
        AlienImGui::HelpMarker(
            resourceTypeString + " 的文件,名称和描述将被储存在服务器。服务端无法保证文件一定不会被删除。");

        AlienImGui::Text("如何使用或创建一个文件夹？");
        AlienImGui::HelpMarker(
            "如果你想要上传" + resourceTypeString
            + "到一个文件夹，你可以使用‘/’字符。文件夹将被自动创建如果不存在。\n例如，为一个模拟器文件命名为‘Biome/Water world/Initial/Variant 1’ 将会创建相嵌的文件夹‘Biome’, ‘Water world’和‘Initial’。");
    }
    ImGui::EndChild();

    if (!_folder.empty()) {
        if (ImGui::BeginChild("##folder info", ImVec2(0, scale(85.0f)), true, ImGuiWindowFlags_HorizontalScrollbar)) {
            AlienImGui::Text("相应的文件夹已在浏览器中被选中\n并且将要用于储存上传的文件:\n\n");
            AlienImGui::BoldText(_folder);
        }
        ImGui::EndChild();
    }

    AlienImGui::Separator();

    AlienImGui::InputText(AlienImGui::InputTextParameters().hint(BrowserDataTypeToUpperString.at(_resourceType)  + " 名称").textWidth(0), _resourceName);

    AlienImGui::Separator();

    ImGui::PushID("描述");
    AlienImGui::InputTextMultiline(
        AlienImGui::InputTextMultilineParameters()
            .hint("描述（可选项）")
            .textWidth(0)
            .height(ImGui::GetContentRegionAvail().y - StyleRepository::get().scale(70.0f)),
        _resourceDescription);
    ImGui::PopID();

    AlienImGui::ToggleButton(
        AlienImGui::ToggleButtonParameters()
            .name("是否公开")
            .tooltip(
                "如是，" + resourceTypeString + " 将对所有用户可见。如否，" + resourceTypeString
                + "将仅在您的私人空间中可见。 这个属性可以在稍后更改，如果需要的话。"),
        _share);

    AlienImGui::Separator();

    ImGui::BeginDisabled(_resourceName.empty());
    if (AlienImGui::Button("确认")) {
        if (NetworkValidationService::get().isStringValidForDatabase(_resourceName) && NetworkValidationService::get().isStringValidForDatabase(_resourceDescription)) {
            close();
            onUpload();
        } else {
            showMessage("错误", Const::NotAllowedCharacters);
        }
        _resourceNameByFolder[_folder] = _resourceName;
        _resourceDescriptionByFolder[_folder] = _resourceDescription;
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
    }
}

void UploadSimulationDialog::onUpload()
{
    auto data = [&]() -> std::variant<UploadNetworkResourceRequestData::SimulationData, UploadNetworkResourceRequestData::GenomeData> {
        if (_resourceType == NetworkResourceType_Simulation) {
            return UploadNetworkResourceRequestData::SimulationData{.zoom = Viewport::get().getZoomFactor(), .center = Viewport::get().getCenterInWorldPos()};
        } else {
            return UploadNetworkResourceRequestData::GenomeData{.description = GenomeEditorWindow::get().getCurrentGenome()};
        }
    }();
    auto workspaceType = _share ? WorkspaceType_Public : WorkspaceType_Private;
    NetworkTransferController::get().onUpload(UploadNetworkResourceRequestData{
        .folderName = _folder,
        .resourceWithoutFolderName = _resourceName,
        .resourceDescription = _resourceDescription,
        .workspaceType = workspaceType,
        .downloadCache = BrowserWindow::get().getSimulationCache(),
        .data = data});
}
