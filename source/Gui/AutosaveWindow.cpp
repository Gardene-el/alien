#include "AutosaveWindow.h"

#include <filesystem>

#include <Fonts/IconsFontAwesome5.h>

#include "Base/Resources.h"
#include "Base/GlobalSettings.h"
#include "Base/StringHelper.h"
#include "PersisterInterface/SavepointTableService.h"
#include "PersisterInterface/SerializerService.h"
#include "PersisterInterface/TaskProcessor.h"

#include "AlienImGui.h"
#include "FileTransferController.h"
#include "GenericMessageDialog.h"
#include "OverlayController.h"
#include "StyleRepository.h"
#include "Viewport.h"

namespace
{
    auto constexpr RightColumnWidth = 200.0f;
    auto constexpr AutosaveSenderId = "Autosave";
    auto constexpr PeakDetectionInterval = 30;  //in seconds
}

AutosaveWindow::AutosaveWindow()
    : AlienWindow("自动保存", "windows.autosave", false)
{}

void AutosaveWindow::initIntern(SimulationFacade simulationFacade, PersisterFacade persisterFacade)
{
    _simulationFacade = simulationFacade;
    _persisterFacade = persisterFacade;
    _settingsOpen = GlobalSettings::get().getValue("windows.autosave.settings.open", _settingsOpen);
    _settingsHeight = GlobalSettings::get().getValue("windows.autosave.settings.height", _settingsHeight);
    _autosaveEnabled = GlobalSettings::get().getValue("windows.autosave.enabled", _autosaveEnabled);
    _origAutosaveInterval = GlobalSettings::get().getValue("windows.autosave.interval", _origAutosaveInterval);
    _autosaveInterval = _origAutosaveInterval;

    _origSaveMode = GlobalSettings::get().getValue("windows.autosave.mode", _origSaveMode);
    _saveMode = _origSaveMode;

    _origNumberOfFiles = GlobalSettings::get().getValue("windows.autosave.number of files", _origNumberOfFiles);
    _numberOfFiles = _origNumberOfFiles;

    _origDirectory = GlobalSettings::get().getValue("windows.autosave.directory", (std::filesystem::current_path() / Const::ResourcePath).string());
    _directory = _origDirectory;

    _origCatchPeaks = GlobalSettings::get().getValue("windows.autosave.catch peaks", _origCatchPeaks);
    _catchPeaks = _origCatchPeaks;

    _lastAutosaveTimepoint = std::chrono::steady_clock::now();
    _lastPeakTimepoint = std::chrono::steady_clock::now();

    _peakProcessor = _TaskProcessor::createTaskProcessor(_persisterFacade);
    _peakDeserializedSimulation = std::make_shared<_SharedDeserializedSimulation>();
    updateSavepointTableFromFile();
}

void AutosaveWindow::shutdownIntern()
{
    GlobalSettings::get().setValue("windows.autosave.settings.open", _settingsOpen);
    GlobalSettings::get().setValue("windows.autosave.settings.height", _settingsHeight);
    GlobalSettings::get().setValue("windows.autosave.enabled", _autosaveEnabled);
    GlobalSettings::get().setValue("windows.autosave.interval", _autosaveInterval);
    GlobalSettings::get().setValue("windows.autosave.mode", _saveMode);
    GlobalSettings::get().setValue("windows.autosave.number of files", _numberOfFiles);
    GlobalSettings::get().setValue("windows.autosave.directory", _directory);
    GlobalSettings::get().setValue("windows.autosave.catch peaks", _catchPeaks);
}

void AutosaveWindow::processIntern()
{
    try {
        processToolbar();

        if (ImGui::BeginChild("##child1", {0, -scale(44.0f)})) {
            processHeader();

            //AlienImGui::Separator();
            if (ImGui::BeginChild("##child2", {0, _settingsOpen ? -_settingsHeight : -scale(35.0f)})) {
                processTable();
            }
            ImGui::EndChild();

            processSettings();
        }
        ImGui::EndChild();

        processStatusBar();

        validateAndCorrect();
    } catch (std::runtime_error const& error) {
        GenericMessageDialog::get().information("错误", error.what());
    }
}

void AutosaveWindow::processBackground()
{
    processDeleteNonPersistentSavepoint();
    processCleanup();
    processAutomaticSavepoints();
    _peakProcessor->process();
}

void AutosaveWindow::processToolbar()
{
    ImGui::SameLine();
    ImGui::BeginDisabled(!_savepointTable.has_value());
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters().text(ICON_FA_PLUS))) {
        onCreateSavepoint(false);
    }
    ImGui::EndDisabled();
    AlienImGui::Tooltip("创建新的保存点");

    ImGui::SameLine();
    ImGui::BeginDisabled(!static_cast<bool>(_selectedEntry));
    if (AlienImGui::ToolbarButton(AlienImGui::ToolbarButtonParameters().text(ICON_FA_MINUS))) {
        onDeleteSavepoint(_selectedEntry);
    }
    AlienImGui::Tooltip("删除保存点");
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!_savepointTable.has_value() || _savepointTable->isEmpty());
    if (AlienImGui::ToolbarButton(ICON_FA_BROOM)) {
        GenericMessageDialog::get().yesNo("删除", "你是否真的想要删除所有的保存点？", [&]() { scheduleCleanup(); });
    }
    AlienImGui::Tooltip("删除所有的保存点");
    ImGui::EndDisabled();

    AlienImGui::Separator();
}

void AutosaveWindow::processHeader()
{
}

void AutosaveWindow::processTable()
{
    if (!_savepointTable.has_value()) {
        AlienImGui::Text("错误：保存点文件无法被读取或创建在指定的文件夹中。");
        return;
    }
    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX;

    if (ImGui::BeginTable("保存文件", 4, flags, ImVec2(0, 0), 0.0f)) {
        ImGui::TableSetupColumn("模拟器", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn("时间戳", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn("步数", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(100.0f));
        ImGui::TableSetupColumn("峰值", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(200.0f));
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Const::TableHeaderColor);

        ImGuiListClipper clipper;
        clipper.Begin(_savepointTable->getSize());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                updateSavepoint(row);
                auto const& entry = _savepointTable->at(row);

                ImGui::PushID(row);
                ImGui::TableNextRow(0, scale(23.0f));

                // project name
                ImGui::TableNextColumn();
                if (entry->state == SavepointState_InQueue) {
                    AlienImGui::Text("在队列中");
                }
                if (entry->state == SavepointState_InProgress) {
                    AlienImGui::Text("在进程中");
                }
                if (entry->state == SavepointState_Persisted) {
                    auto triggerLoadSavepoint = AlienImGui::ActionButton(AlienImGui::ActionButtonParameters().buttonText(ICON_FA_DOWNLOAD));
                    AlienImGui::Tooltip("载入保存点", false);
                    if (triggerLoadSavepoint) {
                        onLoadSavepoint(entry);
                    }

                    ImGui::SameLine();
                    AlienImGui::Text(entry->name);
                }
                if (entry->state == SavepointState_Error) {
                    AlienImGui::Text("错误");
                }

                ImGui::SameLine();
                auto selected = _selectedEntry == entry;
                if (ImGui::Selectable(
                        "",
                        &selected,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                        ImVec2(0, scale(ImGui::GetTextLineHeightWithSpacing()) - ImGui::GetStyle().FramePadding.y))) {
                    _selectedEntry = selected ? entry : nullptr;
                }

                // timestamp
                ImGui::TableNextColumn();
                if (entry->state == SavepointState_Persisted) {
                    AlienImGui::Text(entry->timestamp);
                }

                // timestep
                ImGui::TableNextColumn();
                if (entry->state == SavepointState_Persisted) {
                    AlienImGui::Text(StringHelper::format(entry->timestep));
                }

                // peak
                ImGui::TableNextColumn();
                AlienImGui::Text(entry->peak);

                if (!entry->peakType.empty()) {
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextLightDecentColor.Value);
                    AlienImGui::Text(" (" + entry->peakType + ")");
                    ImGui::PopStyleColor();
                }

                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void AutosaveWindow::processSettings()
{
    ImGui::Spacing();
    ImGui::Spacing();
    if (_settingsOpen) {
        AlienImGui::MovableSeparator(AlienImGui::MovableSeparatorParameters().additive(false), _settingsHeight);
    }

    _settingsOpen = AlienImGui::BeginTreeNode(AlienImGui::TreeNodeParameters().text("设置").highlighted(true).defaultOpen(_settingsOpen));
    if (_settingsOpen) {
        if (ImGui::BeginChild("##autosaveSettings", {scale(0), 0})) {
            if (AlienImGui::InputInt(
                    AlienImGui::InputIntParameters().name("每多少分钟自动保存").textWidth(RightColumnWidth).defaultValue(_origAutosaveInterval),
                    _autosaveInterval,
                    &_autosaveEnabled)) {
                if (_autosaveEnabled) {
                    _lastAutosaveTimepoint = std::chrono::steady_clock::now();
                }
            }
            if (AlienImGui::Switcher(
                    AlienImGui::SwitcherParameters()
                        .name("捕捉峰值")
                        .textWidth(RightColumnWidth)
                        .defaultValue(_origCatchPeaks)
                        .disabled(!_autosaveEnabled)
                        .values({
                            "无",
                            "基因组复杂度方差",
                        })
                        .tooltip("如果激活，此模拟将被持续监控。当自动保存间隔到期时，将保存所选测量值特别高的时间点。"),
                    _catchPeaks)) {
                _peakDeserializedSimulation->setDeserializedSimulation(DeserializedSimulation());
            }

            if (AlienImGui::InputText(
                    AlienImGui::InputTextParameters()
                        .name("文件夹目录")
                        .textWidth(RightColumnWidth)
                        .defaultValue(_origDirectory)
                        .folderButton(true)
                        .tooltip("可以在这里选择保存点存储的目录。这允许在单独的目录中为模拟运行创建保存点。保存点的命名使用当前的步数。"),
                    _directory)) {
                updateSavepointTableFromFile();
            }
            AlienImGui::Switcher(
                AlienImGui::SwitcherParameters()
                    .name("模式")
                    .values({"限制保存文件", "不限制保存文件"})
                    .textWidth(RightColumnWidth)
                    .defaultValue(_origSaveMode),
                _saveMode);
            if (_saveMode == SaveMode_Circular) {
                AlienImGui::InputInt(
                    AlienImGui::InputIntParameters().name("文件数量").textWidth(RightColumnWidth).defaultValue(_origNumberOfFiles), _numberOfFiles);
            }
        }
        ImGui::EndChild();
    }
    AlienImGui::EndTreeNode();
}

void AutosaveWindow::processStatusBar()
{
    std::vector<std::string> statusItems;
    if (!_savepointTable.has_value()) {
        statusItems.emplace_back("不是有效的文件夹");
    } else if (!_autosaveEnabled) {
        statusItems.emplace_back("没有规划中的自动保存");
    } else {
        auto secondsSinceLastAutosave = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _lastAutosaveTimepoint);
        statusItems.emplace_back("下一个自动保存在" + StringHelper::format(std::chrono::seconds(_autosaveInterval * 60) - secondsSinceLastAutosave));
    }
    if (_savepointTable.has_value()) {
        statusItems.emplace_back(std::to_string(_savepointTable->getSize()) + "保存点");
    }

    AlienImGui::StatusBar(statusItems);
}

void AutosaveWindow::onCreateSavepoint(bool usePeakSimulation)
{
    printOverlayMessage("创建保存点...");

    if (_saveMode == SaveMode_Circular) {
        auto nonPersistentEntries = SavepointTableService::get().truncate(_savepointTable.value(), _numberOfFiles - 1);
        scheduleDeleteNonPersistentSavepoint(nonPersistentEntries);
    }

    PersisterRequestId requestId;
    if (usePeakSimulation && !_peakDeserializedSimulation->isEmpty()) {
        auto senderInfo = SenderInfo{.senderId = SenderId{AutosaveSenderId}, .wishResultData = true, .wishErrorInfo = true};
        auto saveData = SaveDeserializedSimulationRequestData{
            .filename = _directory, .sharedDeserializedSimulation = _peakDeserializedSimulation, .generateNameFromTimestep = true, .resetDeserializedSimulation = true};
        requestId = _persisterFacade->scheduleSaveDeserializedSimulation(senderInfo, saveData);
    } else {
        auto senderInfo = SenderInfo{.senderId = SenderId{AutosaveSenderId}, .wishResultData = true, .wishErrorInfo = true};
        auto saveData = SaveSimulationRequestData{
            .filename = _directory, .zoom = Viewport::get().getZoomFactor(), .center = Viewport::get().getCenterInWorldPos(), .generateNameFromTimestep = true};
        requestId = _persisterFacade->scheduleSaveSimulation(senderInfo, saveData);
    }

    auto entry = std::make_shared<_SavepointEntry>(
        _SavepointEntry{.filename = "", .state = SavepointState_InQueue, .timestamp = "", .name = "", .timestep = 0, .requestId = requestId.value});
    SavepointTableService::get().insertEntryAtFront(_savepointTable.value(), entry);
}

void AutosaveWindow::onDeleteSavepoint(SavepointEntry const& entry)
{
    printOverlayMessage("删除保存点...");

    SavepointTableService::get().deleteEntry(_savepointTable.value(), entry);

    if (entry->state != SavepointState_Persisted) {
        scheduleDeleteNonPersistentSavepoint({entry});
    }
    _selectedEntry.reset();
}

void AutosaveWindow::onLoadSavepoint(SavepointEntry const& entry)
{
    auto path = SavepointTableService::get().calcAbsolutePath(_savepointTable.value(), entry);
    FileTransferController::get().onOpenSimulation(path);
}

void AutosaveWindow::processCleanup()
{
    if (_scheduleCleanup) {
        printOverlayMessage("清除保存点...");

        auto nonPersistentEntries = SavepointTableService::get().truncate(_savepointTable.value(), 0);
        scheduleDeleteNonPersistentSavepoint(nonPersistentEntries);
        _scheduleCleanup = false;
    }
}

void AutosaveWindow::processAutomaticSavepoints()
{
    if (!_autosaveEnabled) {
        return;
    }

    if (!_lastSessionId.has_value() || _lastSessionId.value() != _simulationFacade->getSessionId()) {
        _lastAutosaveTimepoint = std::chrono::steady_clock::now();
        _lastSessionId = _simulationFacade->getSessionId();
        _peakDeserializedSimulation->reset();
    }

    auto minSinceLastAutosave = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - _lastAutosaveTimepoint).count();
    if (minSinceLastAutosave >= _autosaveInterval && _savepointTable.has_value()) {
        onCreateSavepoint(_catchPeaks != CatchPeaks_None);
        _lastAutosaveTimepoint = std::chrono::steady_clock::now();
        _lastPeakTimepoint = std::chrono::steady_clock::now();
    }

    if (_catchPeaks != CatchPeaks_None) {
        auto minSinceLastCatchPeak = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _lastPeakTimepoint).count();
        if (minSinceLastCatchPeak >= PeakDetectionInterval) {
            _peakProcessor->executeTask(
                [&](auto const& senderId) {
                    return _persisterFacade->scheduleGetPeakSimulation(
                        SenderInfo{.senderId = senderId, .wishResultData = false, .wishErrorInfo = true},
                        GetPeakSimulationRequestData{
                            .peakDeserializedSimulation = _peakDeserializedSimulation,
                            .zoom = Viewport::get().getZoomFactor(),
                            .center = Viewport::get().getCenterInWorldPos()});
                },
                [&](auto const& requestId) {},
                [](auto const& errors) { GenericMessageDialog::get().information("错误", errors); });
            _lastPeakTimepoint = std::chrono::steady_clock::now();
        }
    }
}

void AutosaveWindow::scheduleDeleteNonPersistentSavepoint(std::vector<SavepointEntry> const& entries)
{
    for (auto const& entry : entries) {
        if (!entry->requestId.empty() && (entry->state == SavepointState_InQueue || entry->state == SavepointState_InProgress)) {
            _savepointsInProgressToDelete.emplace_back(entry);
        }
    }
}

void AutosaveWindow::processDeleteNonPersistentSavepoint()
{
    std::vector<SavepointEntry> newRequestsToDelete;
    for (auto const& entry : _savepointsInProgressToDelete) {
        if (auto requestState = _persisterFacade->getRequestState(PersisterRequestId{entry->requestId})) {
            if (requestState.value() == PersisterRequestState::Finished) {
                auto resultData = _persisterFacade->fetchSaveSimulationData(PersisterRequestId{entry->requestId});
                SerializerService::get().deleteSimulation(resultData.filename);
            } else if (requestState.value() == PersisterRequestState::Error) {
                // do nothing
            } else {
                newRequestsToDelete.emplace_back(entry);
            }
        }
    }
    _savepointsInProgressToDelete = newRequestsToDelete;
}

void AutosaveWindow::scheduleCleanup()
{
    _scheduleCleanup = true;
}

void AutosaveWindow::updateSavepoint(int row)
{
    auto entry = _savepointTable->at(row);
    if (entry->state != SavepointState_Persisted) {
        auto newEntry = _savepointTable->at(row);
        auto requestState = _persisterFacade->getRequestState(PersisterRequestId{newEntry->requestId});
        if (requestState.has_value()) {
            if (requestState.value() == PersisterRequestState::InProgress) {
                newEntry->state = SavepointState_InProgress;
            }
            if (requestState.value() == PersisterRequestState::Finished) {
                newEntry->state = SavepointState_Persisted;
                auto requestResult = _persisterFacade->fetchPersisterRequestResult(PersisterRequestId{newEntry->requestId});

                if (auto saveResult = std::dynamic_pointer_cast<_SaveSimulationRequestResult>(requestResult)) {
                    auto const& data = saveResult->getData();
                    newEntry->timestep = data.timestep;
                    newEntry->timestamp = StringHelper::format(data.timestamp);
                    newEntry->name = data.projectName;
                    newEntry->filename = SavepointTableService::get().calcEntryPath(_savepointTable.value(), data.filename);
                } else if (auto saveResult = std::dynamic_pointer_cast<_SaveDeserializedSimulationRequestResult>(requestResult)) {
                    auto const& data = saveResult->getData();
                    newEntry->timestep = data.timestep;
                    newEntry->timestamp = StringHelper::format(data.timestamp);
                    newEntry->name = data.projectName;
                    newEntry->filename = SavepointTableService::get().calcEntryPath(_savepointTable.value(), data.filename);
                    newEntry->peak = StringHelper::format(toFloat(sumColorVector(data.rawStatisticsData.timeline.timestep.genomeComplexityVariance)), 2);
                    newEntry->peakType = "基因组复杂度方差";
                }
            }
            if (requestState.value() == PersisterRequestState::Error) {
                newEntry->state = SavepointState_Error;
            }
            SavepointTableService::get().updateEntry(_savepointTable.value(), row, newEntry);
        }
    }
}

void AutosaveWindow::updateSavepointTableFromFile()
{
    if (auto savepoint = SavepointTableService::get().loadFromFile(getSavepointFilename()); std::holds_alternative<SavepointTable>(savepoint)) {
        _savepointTable = std::get<SavepointTable>(savepoint);
    } else {
        _savepointTable.reset();
    }
    _selectedEntry.reset();
}

std::string AutosaveWindow::getSavepointFilename() const
{
    return (std::filesystem::path(_directory) / Const::SavepointTableFilename).string();
}

void AutosaveWindow::validateAndCorrect()
{
    _numberOfFiles = std::max(1, _numberOfFiles);
    _autosaveInterval = std::max(1, _autosaveInterval);
}
