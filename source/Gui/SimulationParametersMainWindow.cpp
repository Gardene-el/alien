#include "SimulationParametersMainWindow.h"

#include <ImFileDialog.h>

#include <Fonts/IconsFontAwesome5.h>

#include "Base/StringHelper.h"
#include "EngineInterface/SimulationFacade.h"
#include "EngineInterface/SimulationParametersEditService.h"
#include "PersisterInterface/SerializerService.h"

#include "GenericFileDialog.h"
#include "GenericMessageDialog.h"
#include "LocationController.h"
#include "LocationHelper.h"
#include "OverlayController.h"
#include "SimulationParametersSourceWidgets.h"
#include "SimulationParametersZoneWidgets.h"
#include "AlienImGui.h"
#include "Viewport.h"

namespace
{
    auto constexpr MasterHeight = 130.0f;
    auto constexpr MasterMinHeight = 50.0f;
    auto constexpr MasterRowHeight = 25.0f;

    auto constexpr DetailWidgetMinHeight = 0.0f;

    auto constexpr ExpertWidgetHeight = 130.0f;
    auto constexpr ExpertWidgetMinHeight = 60.0f;
}

SimulationParametersMainWindow::SimulationParametersMainWindow()
    : AlienWindow("模拟器参数", "windows.simulation parameters", false)
{}

void SimulationParametersMainWindow::initIntern(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;

    _masterWidgetOpen = GlobalSettings::get().getValue("windows.simulation parameters.master widget.open", _masterWidgetOpen);
    _detailWidgetOpen = GlobalSettings::get().getValue("windows.simulation parameters.detail widget.open", _detailWidgetOpen);
    _expertWidgetOpen = GlobalSettings::get().getValue("windows.simulation parameters.expert widget.open", _expertWidgetOpen);
    _masterWidgetHeight = GlobalSettings::get().getValue("windows.simulation parameters.master widget.height", scale(MasterHeight));
    _expertWidgetHeight = GlobalSettings::get().getValue("windows.simulation parameters.expert widget height", scale(ExpertWidgetHeight));

    auto baseWidgets = std::make_shared<_SimulationParametersBaseWidgets>();
    baseWidgets->init(_simulationFacade);
    _baseWidgets = baseWidgets;

    auto zoneWidgets = std::make_shared<_SimulationParametersZoneWidgets>();
    zoneWidgets->init(_simulationFacade, 0);
    _zoneWidgets = zoneWidgets;


    auto sourceWidgets = std::make_shared<_SimulationParametersSourceWidgets>();
    sourceWidgets->init(_simulationFacade, 0);
    _sourceWidgets = sourceWidgets;
}

void SimulationParametersMainWindow::processIntern()
{
    if (!_sessionId.has_value() || _sessionId.value() != _simulationFacade->getSessionId()) {
        _selectedLocationIndex = 0;
    }

    processToolbar();

    if (ImGui::BeginChild("##content", {0, -scale(50.0f)})) {

        updateLocations();

        auto origMasterHeight = _masterWidgetHeight;
        auto origExpertWidgetHeight = _expertWidgetHeight;

        processMasterWidget();
        processDetailWidget();
        processExpertWidget();

        correctLayout(origMasterHeight, origExpertWidgetHeight);
    }
    ImGui::EndChild();

    processStatusBar();

    _sessionId = _simulationFacade->getSessionId();
}

void SimulationParametersMainWindow::shutdownIntern()
{
    GlobalSettings::get().setValue("windows.simulation parameters.master widget.open", _masterWidgetOpen);
    GlobalSettings::get().setValue("windows.simulation parameters.detail widget.open", _detailWidgetOpen);
    GlobalSettings::get().setValue("windows.simulation parameters.expert widget.open", _expertWidgetOpen);
    GlobalSettings::get().setValue("windows.simulation parameters.master widget.height", _masterWidgetHeight);
    GlobalSettings::get().setValue("windows.simulation parameters.expert widget height", _expertWidgetHeight);
}

void SimulationParametersMainWindow::processToolbar()
{
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters().text(ICON_FA_FOLDER_OPEN).tooltip("从文件中打开模拟器参数"))) {
        onOpenParameters();
    }

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters().text(ICON_FA_SAVE).tooltip("将模拟器参数保存到文件"))) {
        onSaveParameters();
    }

    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters().text(ICON_FA_COPY).tooltip("将模拟器参数复制到剪贴板"))) {
        _copiedParameters = _simulationFacade->getSimulationParameters();
        printOverlayMessage("模拟器参数已复制");
    }

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(
            AlienImGui::ToolbarButtonParameters().text(ICON_FA_PASTE).tooltip("从剪贴板粘贴模拟器参数").disabled(!_copiedParameters))) {
        _simulationFacade->setSimulationParameters(*_copiedParameters);
        _simulationFacade->setOriginalSimulationParameters(*_copiedParameters);
        printOverlayMessage("模拟器参数已粘贴");
    }

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters()
                                      .text(ICON_FA_PASTE)
                                      .secondText(ICON_FA_UNDO)
                                      .secondTextOffset(RealVector2D{32.0f, 28.0f})
                                      .secondTextScale(0.3f)
                                      .tooltip("用剪贴板中的值替换参考值。这有助于查看当前参数与剪贴板中参数之间的差异。")
                                      .disabled(!_copiedParameters))) {
        auto parameters = _simulationFacade->getSimulationParameters();
        if (_copiedParameters->numZones == parameters.numZones && _copiedParameters->numRadiationSources == parameters.numRadiationSources) {
            _simulationFacade->setOriginalSimulationParameters(*_copiedParameters);
            printOverlayMessage("参考模拟器参数已替换");
        } else {
            GenericMessageDialog::get().information(
                "错误", "当前模拟器参数的区域和辐射源数量必须与剪贴板中的参数一致。");
        }
    }

    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters().text(ICON_FA_PLUS).secondText(ICON_FA_LAYER_GROUP).tooltip("添加参数区域"))) {
        onAddZone();
    }

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters().text(ICON_FA_PLUS).secondText(ICON_FA_SUN).tooltip("添加辐射源"))) {
        onAddSource();
    }

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters()
                                      .text(ICON_FA_PLUS)
                                      .secondText(ICON_FA_CLONE)
                                      .disabled(_selectedLocationIndex == 0)
                                      .tooltip("克隆选中的区域/辐射源"))) {
        onCloneLocation();
    }

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(
            AlienImGui::ToolbarButtonParameters().text(ICON_FA_MINUS).disabled(_selectedLocationIndex == 0).tooltip("删除选中的区域/辐射源"))) {
        onDeleteLocation();
    }

    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters()
                                      .text(ICON_FA_CHEVRON_UP)
                                      .disabled(_selectedLocationIndex <= 1)
                                      .tooltip("将选中的区域/辐射源上移"))) {
        onDecreaseLocationIndex();
    }

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters()
                                      .text(ICON_FA_CHEVRON_DOWN)
                                      .tooltip("将选中的区域/辐射源下移")
                                      .disabled(_selectedLocationIndex >= _locations.size() - 1 || _selectedLocationIndex == 0))) {
        onIncreaseLocationIndex();
    }

    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();

    ImGui::SameLine();
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters()
                                      .text(ICON_FA_EXTERNAL_LINK_SQUARE_ALT)
                                      .tooltip("在新窗口中打开选中区域/辐射源的参数"))) {
        onOpenInLocationWindow();
    }

    AlienImGui::Separator();
}

void SimulationParametersMainWindow::processMasterWidget()
{
    if (ImGui::BeginChild("##master", {0, getMasterWidgetHeight()})) {

        if (_masterWidgetOpen = AlienImGui::BeginTreeNode(
                AlienImGui::TreeNodeParameters().name("概览").rank(AlienImGui::TreeNodeRank::High).defaultOpen(_masterWidgetOpen))) {
            ImGui::Spacing();
            if (ImGui::BeginChild("##master2", {0, -ImGui::GetStyle().FramePadding.y})) {
                processLocationTable();
            }
            ImGui::EndChild();
        }
        AlienImGui::EndTreeNode();
    }
    ImGui::EndChild();

    if (_masterWidgetOpen && (_detailWidgetOpen || _expertWidgetOpen)) {
        ImGui::PushID("master");
        AlienImGui::MovableSeparator(AlienImGui::MovableSeparatorParameters(), _masterWidgetHeight);
        ImGui::PopID();
    }
}

void SimulationParametersMainWindow::processDetailWidget()
{
    auto height = getDetailWidgetHeight();
    if (ImGui::BeginChild("##detail", {0, height})) {
        auto title = _filter.empty() ? "参数" : "参数（已过滤）";
        if (_detailWidgetOpen = AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters()
                                                              .name((std::string(title) + "###parameters").c_str())
                                                              .rank(AlienImGui::TreeNodeRank::High)
                                                              .defaultOpen(_detailWidgetOpen))) {
            ImGui::Spacing();
            AlienImGui::SetFilterText(_filter);
            if (ImGui::BeginChild(
                    "##detail2", {0, -ImGui::GetStyle().FramePadding.y - scale(33.0f)}, ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {
                auto type = _locations.at(_selectedLocationIndex).type;
                if (type == LocationType::Base) {
                    _baseWidgets->process();
                } else if (type == LocationType::ParameterZone) {
                    _zoneWidgets->setLocationIndex(_selectedLocationIndex);
                    _zoneWidgets->process();
                } else if (type == LocationType::RadiationSource) {
                    _sourceWidgets->setLocationIndex(_selectedLocationIndex);
                    _sourceWidgets->process();
                }
            }
            ImGui::EndChild();
            AlienImGui::ResetFilterText();

            ImGui::Spacing();
            AlienImGui::InputFilter(AlienImGui::InputFilterParameters().width(250.0f), _filter);
        }
        AlienImGui::EndTreeNode();
    }
    ImGui::EndChild();

    if (_detailWidgetOpen && _expertWidgetOpen) {
        ImGui::PushID("detail");
        AlienImGui::MovableSeparator(AlienImGui::MovableSeparatorParameters().additive(false), _expertWidgetHeight);
        ImGui::PopID();
    }
}

void SimulationParametersMainWindow::processExpertWidget()
{
    if (ImGui::BeginChild("##expert", {0, 0})) {
        if (_expertWidgetOpen = AlienImGui::BeginTreeNode(
                AlienImGui::TreeNodeParameters().name("专家设置").rank(AlienImGui::TreeNodeRank::High).defaultOpen(_expertWidgetOpen))) {
            if (ImGui::BeginChild("##expert2", {0, 0}, ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {
                processExpertSettings();
            }
            ImGui::EndChild();
        }
        AlienImGui::EndTreeNode();
    }
    ImGui::EndChild();
}

void SimulationParametersMainWindow::processStatusBar()
{
    std::vector<std::string> statusItems;
    statusItems.emplace_back("在滑块上按住 CTRL 并点击以输入精确值");

    AlienImGui::StatusBar(statusItems);
}

void SimulationParametersMainWindow::processLocationTable()
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX;

    if (ImGui::BeginTable("Locations", 4, flags, ImVec2(-1, -1), 0)) {

        ImGui::TableSetupColumn("名称", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn("类型", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn("位置", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(115.0f));
        ImGui::TableSetupColumn("强度", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(100.0f));
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Const::TableHeaderColor);

        ImGuiListClipper clipper;
        clipper.Begin(_locations.size());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                auto const& entry = _locations.at(row);

                ImGui::PushID(row);
                ImGui::TableNextRow(0, scale(MasterRowHeight));

                // name
                ImGui::TableNextColumn();
                auto selected = _selectedLocationIndex == row;
                if (ImGui::Selectable(
                        "",
                        &selected,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                        ImVec2(0, scale(MasterRowHeight) - ImGui::GetStyle().FramePadding.y))) {
                    _selectedLocationIndex = row;
                }
                ImGui::SameLine();
                AlienImGui::Text(entry.name);


                // type
                ImGui::TableNextColumn();
                if (entry.type == LocationType::Base) {
                    AlienImGui::Text("基础参数");
                } else if (entry.type == LocationType::ParameterZone) {
                    AlienImGui::Text("区域");
                } else if (entry.type == LocationType::RadiationSource) {
                    AlienImGui::Text("辐射");
                }

                // position
                ImGui::TableNextColumn();
                if (row > 0) {
                    if (AlienImGui::ActionButton(AlienImGui::ActionButtonParameters().buttonText(ICON_FA_SEARCH))) {
                        onCenterLocation(row);
                    }
                    ImGui::SameLine();
                }
                AlienImGui::Text(entry.position);

                // strength
                ImGui::TableNextColumn();
                AlienImGui::Text(entry.strength);

                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void SimulationParametersMainWindow::processExpertSettings()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origFeatures = _simulationFacade->getOriginalSimulationParameters().features;
    auto lastFeatures = parameters.features;

    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("高级吸收控制")
            .textWidth(0)
            .defaultValue(origFeatures.advancedAbsorptionControl)
            .tooltip("这些设置提供了控制细胞吸收能量粒子的扩展可能性。"),
        parameters.features.advancedAbsorptionControl);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("高级攻击者控制")
            .textWidth(0)
            .defaultValue(origFeatures.advancedAttackerControl)
            .tooltip("它包含进一步影响攻击细胞从攻击中获得多少能量的设置。"),
        parameters.features.advancedAttackerControl);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("细胞年龄限制器")
            .textWidth(0)
            .defaultValue(origFeatures.cellAgeLimiter)
            .tooltip("它启用了控制最大细胞年龄的额外可能性。"),
        parameters.features.cellAgeLimiter);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("细胞颜色转换规则")
            .textWidth(0)
            .defaultValue(origFeatures.cellColorTransitionRules)
            .tooltip("这可以根据细胞的年龄定义细胞的颜色转换。"),
        parameters.features.cellColorTransitionRules);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("细胞发光")
            .textWidth(0)
            .defaultValue(origFeatures.cellGlow)
            .tooltip("它启用一个额外的渲染步骤，使细胞发光。"),
        parameters.features.cellGlow);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("自定义删除变异")
            .textWidth(0)
            .defaultValue(origFeatures.customizeDeletionMutations)
            .tooltip("它启用了删除变异的进一步设置。如果禁用，则使用默认值（显示在具体参数的工具提示中）。"),
        parameters.features.customizeDeletionMutations);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("自定义神经元变异")
            .textWidth(0)
            .defaultValue(origFeatures.customizeNeuronMutations)
            .tooltip("它启用了神经元变异的进一步设置。如果禁用，则使用默认值（显示在具体参数的工具提示中）。"),
        parameters.features.customizeNeuronMutations);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("外部能量控制")
            .textWidth(0)
            .defaultValue(origFeatures.externalEnergyControl)
            .tooltip(
                "这些设置用于添加和控制外部能量源。其能量可以逐渐转移到模拟中的构建细胞。反之，辐射和垂死细胞的能量也可以转移回外部能量源。"),
        parameters.features.externalEnergyControl);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("基因组复杂度测量")
            .textWidth(0)
            .defaultValue(origFeatures.genomeComplexityMeasurement)
            .tooltip("在这里激活基因组复杂度的计算参数。该基因组复杂度可用于“高级吸收控制”和“高级攻击者控制”，以在自然选择中青睐更复杂的基因组。"
                     "如果停用，则使用仅考虑基因组大小的默认值。"),
        parameters.features.genomeComplexityMeasurement);
    AlienImGui::Checkbox(
        AlienImGui::CheckboxParameters()
            .name("旧版行为")
            .textWidth(0)
            .defaultValue(origFeatures.legacyModes)
            .tooltip("它包含用于与旧版本兼容的功能。"),
        parameters.features.legacyModes);

    if (parameters.features != lastFeatures) {
        _simulationFacade->setSimulationParameters(parameters);
    }
}

void SimulationParametersMainWindow::onOpenParameters()
{
    GenericFileDialog::get().showOpenFileDialog(
        "打开模拟器参数", "Simulation parameters (*.parameters){.parameters},.*", _fileDialogPath, [&](std::filesystem::path const& path) {
            auto firstFilename = ifd::FileDialog::Instance().GetResult();
            auto firstFilenameCopy = firstFilename;
            _fileDialogPath = firstFilenameCopy.remove_filename().string();

            SimulationParameters parameters;
            if (!SerializerService::get().deserializeSimulationParametersFromFile(parameters, firstFilename.string())) {
                GenericMessageDialog::get().information("打开模拟器参数", "所选文件无法打开。");
            } else {
                _simulationFacade->setSimulationParameters(parameters);
                _simulationFacade->setOriginalSimulationParameters(parameters);
            }
        });
}

void SimulationParametersMainWindow::onSaveParameters()
{
    GenericFileDialog::get().showSaveFileDialog(
        "保存模拟器参数", "Simulation parameters (*.parameters){.parameters},.*", _fileDialogPath, [&](std::filesystem::path const& path) {
            auto firstFilename = ifd::FileDialog::Instance().GetResult();
            auto firstFilenameCopy = firstFilename;
            _fileDialogPath = firstFilenameCopy.remove_filename().string();

            auto parameters = _simulationFacade->getSimulationParameters();
            if (!SerializerService::get().serializeSimulationParametersToFile(firstFilename.string(), parameters)) {
                GenericMessageDialog::get().information("保存模拟器参数", "所选文件无法保存。");
            }
        });
}

void SimulationParametersMainWindow::onAddZone()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();

    if (!checkNumZones(parameters)) {
        return;
    }

    ++_selectedLocationIndex;
    LocationHelper::adaptLocationIndex(parameters, _selectedLocationIndex, 1);
    LocationHelper::adaptLocationIndex(origParameters, _selectedLocationIndex, 1);

    auto worldSize = _simulationFacade->getWorldSize();

    SimulationParametersZone zone;
    StringHelper::copy(zone.name, sizeof(zone.name), LocationHelper::generateZoneName(parameters));
    zone.locationIndex = _selectedLocationIndex;
    zone.posX = toFloat(worldSize.x / 2);
    zone.posY = toFloat(worldSize.y / 2);
    auto maxRadius = toFloat(std::min(worldSize.x, worldSize.y)) / 2;
    zone.shapeType = SpotShapeType_Circular;
    zone.fadeoutRadius = maxRadius / 3;
    zone.color = _zoneColorPalette.getColor((2 + parameters.numZones) * 8);
    zone.values = parameters.baseValues;

    setDefaultShapeDataForZone(zone);

    int index = parameters.numZones;
    parameters.zone[index] = zone;
    origParameters.zone[index] = zone;
    ++parameters.numZones;
    ++origParameters.numZones;
    _simulationFacade->setSimulationParameters(parameters);
    _simulationFacade->setOriginalSimulationParameters(origParameters);
}

void SimulationParametersMainWindow::onAddSource()
{
    auto& editService = SimulationParametersEditService::get();

    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();

    if (!checkNumSources(parameters)) {
        return;
    }

    ++_selectedLocationIndex;
    LocationHelper::adaptLocationIndex(parameters, _selectedLocationIndex, 1);
    LocationHelper::adaptLocationIndex(origParameters, _selectedLocationIndex, 1);

    auto strengths = editService.getRadiationStrengths(parameters);
    auto newStrengths = editService.calcRadiationStrengthsForAddingZone(strengths);

    auto worldSize = _simulationFacade->getWorldSize();

    RadiationSource source;
    StringHelper::copy(source.name, sizeof(source.name), LocationHelper::generateSourceName(parameters));
    source.locationIndex = _selectedLocationIndex;
    source.posX = toFloat(worldSize.x / 2);
    source.posY = toFloat(worldSize.y / 2);

    auto index = parameters.numRadiationSources;
    parameters.radiationSource[index] = source;
    origParameters.radiationSource[index] = source;
    ++parameters.numRadiationSources;
    ++origParameters.numRadiationSources;

    editService.applyRadiationStrengths(parameters, newStrengths);
    editService.applyRadiationStrengths(origParameters, newStrengths);

    _simulationFacade->setSimulationParameters(parameters);
    _simulationFacade->setOriginalSimulationParameters(origParameters);
}

void SimulationParametersMainWindow::onCloneLocation()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();

    auto location = LocationHelper::findLocation(parameters, _selectedLocationIndex);

    if (std::holds_alternative<SimulationParametersZone*>(location)) {
        if (!checkNumZones(parameters)) {
            return;
        }
    } else {
        if (!checkNumSources(parameters)) {
            return;
        }
    }

    ++_selectedLocationIndex;
    LocationHelper::adaptLocationIndex(parameters, _selectedLocationIndex, 1);
    LocationHelper::adaptLocationIndex(origParameters, _selectedLocationIndex, 1);

    if (std::holds_alternative<SimulationParametersZone*>(location)) {
        auto zone = std::get<SimulationParametersZone*>(location);
        auto clone = *zone;

        StringHelper::copy(clone.name, sizeof(clone.name), LocationHelper::generateZoneName(parameters));
        clone.locationIndex = _selectedLocationIndex;

        int index = parameters.numZones;
        parameters.zone[index] = clone;
        origParameters.zone[index] = clone;
        ++parameters.numZones;
        ++origParameters.numZones;
    } else {
        auto source = std::get<RadiationSource*>(location);
        auto clone = *source;

        auto& editService = SimulationParametersEditService::get();
        auto strengths = editService.getRadiationStrengths(parameters);
        auto newStrengths = editService.calcRadiationStrengthsForAddingZone(strengths);

        StringHelper::copy(clone.name, sizeof(clone.name), LocationHelper::generateSourceName(parameters));
        clone.locationIndex = _selectedLocationIndex;
        auto index = parameters.numRadiationSources;
        parameters.radiationSource[index] = clone;
        origParameters.radiationSource[index] = clone;
        ++parameters.numRadiationSources;
        ++origParameters.numRadiationSources;

        editService.applyRadiationStrengths(parameters, newStrengths);
        editService.applyRadiationStrengths(origParameters, newStrengths);
    }

    _simulationFacade->setSimulationParameters(parameters);
    _simulationFacade->setOriginalSimulationParameters(origParameters);
}

void SimulationParametersMainWindow::onDeleteLocation()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto origParameters = _simulationFacade->getOriginalSimulationParameters();

    LocationController::get().deleteLocationWindow(_selectedLocationIndex);
    auto location = LocationHelper::findLocation(parameters, _selectedLocationIndex);

    if (std::holds_alternative<SimulationParametersZone*>(location)) {
        std::optional<int> zoneIndex;
        for (int i = 0; i < parameters.numZones; ++i) {
            if (parameters.zone[i].locationIndex == _selectedLocationIndex) {
                zoneIndex = i;
                break;
            }
        }
        if (zoneIndex.has_value()) {
            for (int i = zoneIndex.value(); i < parameters.numZones - 1; ++i) {
                parameters.zone[i] = parameters.zone[i + 1];
                origParameters.zone[i] = origParameters.zone[i + 1];
            }
            --parameters.numZones;
            --origParameters.numZones;
        }
    } else {
        std::optional<int> sourceIndex;
        for (int i = 0; i < parameters.numRadiationSources; ++i) {
            if (parameters.radiationSource[i].locationIndex == _selectedLocationIndex) {
                sourceIndex = i;
                break;
            }
        }
        if (sourceIndex.has_value()) {
            for (int i = sourceIndex.value(); i < parameters.numRadiationSources - 1; ++i) {
                parameters.radiationSource[i] = parameters.radiationSource[i + 1];
                origParameters.radiationSource[i] = origParameters.radiationSource[i + 1];
            }
            --parameters.numRadiationSources;
            --origParameters.numRadiationSources;
        }
    }

    auto newByOldLocationIndex = LocationHelper::adaptLocationIndex(parameters, _selectedLocationIndex, -1);
    LocationHelper::adaptLocationIndex(origParameters, _selectedLocationIndex, -1);

    if (_locations.size() - 1 == _selectedLocationIndex) {
        --_selectedLocationIndex;
    }

    _simulationFacade->setSimulationParameters(parameters);
    _simulationFacade->setOriginalSimulationParameters(origParameters);

    LocationController::get().remapLocationIndices(newByOldLocationIndex);
}

void SimulationParametersMainWindow::onDecreaseLocationIndex()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto newByOldLocationIndex = LocationHelper::onDecreaseLocationIndex(parameters, _selectedLocationIndex);
    _simulationFacade->setSimulationParameters(parameters);

    auto origParameters = _simulationFacade->getOriginalSimulationParameters();
    LocationHelper::onDecreaseLocationIndex(origParameters, _selectedLocationIndex);
    _simulationFacade->setOriginalSimulationParameters(parameters);

    --_selectedLocationIndex;
    LocationController::get().remapLocationIndices(newByOldLocationIndex);
}

void SimulationParametersMainWindow::onIncreaseLocationIndex()
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto newByOldLocationIndex = LocationHelper::onIncreaseLocationIndex(parameters, _selectedLocationIndex);
    _simulationFacade->setSimulationParameters(parameters);

    auto origParameters = _simulationFacade->getOriginalSimulationParameters();
    LocationHelper::onIncreaseLocationIndex(origParameters, _selectedLocationIndex);
    _simulationFacade->setOriginalSimulationParameters(parameters);

    ++_selectedLocationIndex;
    LocationController::get().remapLocationIndices(newByOldLocationIndex);
}

void SimulationParametersMainWindow::onOpenInLocationWindow()
{
    auto mousePos = ImGui::GetMousePos();
    auto offset = RealVector2D{50.0f + toFloat(_locationWindowCounter) * 15, toFloat(_locationWindowCounter) * 15};
    LocationController::get().addLocationWindow(_selectedLocationIndex, {mousePos.x + offset.x, mousePos.y + offset.y});
    _locationWindowCounter = (_locationWindowCounter + 1) % 8;
}

void SimulationParametersMainWindow::onCenterLocation(int locationIndex)
{
    auto parameters = _simulationFacade->getSimulationParameters();
    auto location = LocationHelper::findLocation(parameters, locationIndex);
    RealVector2D pos;
    if (std::holds_alternative<SimulationParametersZone*>(location)) {
        auto zone = std::get<SimulationParametersZone*>(location);
        pos = {zone->posX, zone->posY};
    } else {
        auto source = std::get<RadiationSource*>(location);
        pos = {source->posX, source->posY};
    }
    Viewport::get().setCenterInWorldPos(pos);
}

void SimulationParametersMainWindow::updateLocations()
{
    auto parameters = _simulationFacade->getSimulationParameters();

    _locations = std::vector<Location>(1 + parameters.numZones + parameters.numRadiationSources);
    auto strength = SimulationParametersEditService::get().getRadiationStrengths(parameters);
    auto pinnedString = strength.pinned.contains(0) ? ICON_FA_THUMBTACK " " : " ";
    _locations.at(0) = Location{"基础", LocationType::Base, "-", pinnedString + StringHelper::format(strength.values.front() * 100 + 0.05f, 1) + "%"};
    for (int i = 0; i < parameters.numZones; ++i) {
        auto const& zone = parameters.zone[i];
        auto position = "(" + StringHelper::format(zone.posX, 0) + ", " + StringHelper::format(zone.posY, 0) + ")";
        _locations.at(zone.locationIndex) = Location{zone.name, LocationType::ParameterZone, position};
    }
    for (int i = 0; i < parameters.numRadiationSources; ++i) {
        auto const& source = parameters.radiationSource[i];
        auto position = "(" + StringHelper::format(source.posX, 0) + ", " + StringHelper::format(source.posY, 0) + ")";
        auto pinnedString = strength.pinned.contains(i + 1) ? ICON_FA_THUMBTACK " " : " ";
        _locations.at(source.locationIndex) = Location{
            source.name, LocationType::RadiationSource, position, pinnedString + StringHelper::format(strength.values.at(i + 1) * 100 + 0.05f, 1) + "%"};
    }
}

void SimulationParametersMainWindow::setDefaultShapeDataForZone(SimulationParametersZone& spot) const
{
    auto worldSize = _simulationFacade->getWorldSize();

    auto maxRadius = toFloat(std::min(worldSize.x, worldSize.y)) / 2;
    if (spot.shapeType == SpotShapeType_Circular) {
        spot.shapeData.circularSpot.coreRadius = maxRadius / 3;
    } else {
        spot.shapeData.rectangularSpot.height = maxRadius / 3;
        spot.shapeData.rectangularSpot.width = maxRadius / 3;
    }
}

void SimulationParametersMainWindow::correctLayout(float origMasterHeight, float origExpertWidgetHeight)
{
    auto detailHeight = ImGui::GetWindowSize().y - getMasterWidgetRefHeight() - getExpertWidgetRefHeight();

    if (detailHeight < scale(DetailWidgetMinHeight) || _masterWidgetHeight < scale(MasterMinHeight) || _expertWidgetHeight < scale(ExpertWidgetMinHeight)) {
        _masterWidgetHeight = origMasterHeight;
        _expertWidgetHeight = origExpertWidgetHeight;
    }
}

bool SimulationParametersMainWindow::checkNumZones(SimulationParameters const& parameters)
{
    if (parameters.numZones == MAX_ZONES) {
        showMessage("错误", "已达到区域的最大数量。");
        return false;
    }
    return true;
}

bool SimulationParametersMainWindow::checkNumSources(SimulationParameters const& parameters)
{
    if (parameters.numRadiationSources == MAX_RADIATION_SOURCES) {
        showMessage("错误", "已达到辐射源的最大数量。");
        return false;
    }
    return true;
}

float SimulationParametersMainWindow::getMasterWidgetRefHeight() const
{
    return _masterWidgetOpen ? _masterWidgetHeight : scale(25.0f);
}

float SimulationParametersMainWindow::getExpertWidgetRefHeight() const
{
    return _expertWidgetOpen ? _expertWidgetHeight : scale(30.0f);
}

float SimulationParametersMainWindow::getMasterWidgetHeight() const
{
    if (_masterWidgetOpen && !_detailWidgetOpen && !_expertWidgetOpen) {
        return std::max(scale(MasterMinHeight), ImGui::GetContentRegionAvail().y - getDetailWidgetHeight() - getExpertWidgetRefHeight());
    }
    return getMasterWidgetRefHeight();
}

float SimulationParametersMainWindow::getDetailWidgetHeight() const
{
    return _detailWidgetOpen ? std::max(scale(MasterMinHeight), ImGui::GetContentRegionAvail().y - getExpertWidgetRefHeight() + scale(4.0f)) : scale(25.0f);
}
