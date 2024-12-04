#include "MainLoopController.h"

#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <Fonts/IconsFontAwesome5.h>

#include "Base/Definitions.h"
#include "Base/GlobalSettings.h"
#include "Base/Resources.h"
#include "Base/LoggingService.h"
#include "PersisterInterface/SerializerService.h"
#include "EngineInterface/SimulationFacade.h"
#include "PersisterInterface/PersisterFacade.h"

#include "OpenGLHelper.h"
#include "Viewport.h"
#include "StyleRepository.h"
#include "TemporalControlWindow.h"
#include "GenericMessageDialog.h"
#include "OverlayController.h"
#include "SimulationView.h"
#include "FpsController.h"
#include "GenomeEditorWindow.h"
#include "GettingStartedWindow.h"
#include "GpuSettingsDialog.h"
#include "ImageToPatternDialog.h"
#include "LoginDialog.h"
#include "LogWindow.h"
#include "MassOperationsDialog.h"
#include "MultiplierWindow.h"
#include "NetworkSettingsDialog.h"
#include "NewSimulationDialog.h"
#include "PatternAnalysisDialog.h"
#include "PatternEditorWindow.h"
#include "RadiationSourcesWindow.h"
#include "SelectionWindow.h"
#include "ShaderWindow.h"
#include "SimulationInteractionController.h"
#include "SimulationParametersWindow.h"
#include "SpatialControlWindow.h"
#include "StatisticsWindow.h"
#include "UiController.h"
#include "UploadSimulationDialog.h"
#include "AboutDialog.h"
#include "AlienImGui.h"
#include "AutosaveWindow.h"
#include "BrowserWindow.h"
#include "CreatorWindow.h"
#include "DeleteUserDialog.h"
#include "DisplaySettingsDialog.h"
#include "EditorController.h"
#include "ExitDialog.h"
#include "FileTransferController.h"

namespace
{
    std::chrono::milliseconds::rep const FadeOutDuration = 500;
    std::chrono::milliseconds::rep const FadeInDuration = 500;

    auto const StartupSenderId = "Startup";
}

void MainLoopController::setup(SimulationFacade const& simulationFacade, PersisterFacade const& persisterFacade)
{
    _simulationFacade = simulationFacade;
    _persisterFacade = persisterFacade;

    _logo = OpenGLHelper::loadTexture(Const::LogoFilename);
    _saveOnExit = GlobalSettings::get().getValue("controllers.main loop.save on exit", _saveOnExit);
}

void MainLoopController::process()
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //   ImGui::ShowDemoWindow(NULL);

    int display_w, display_h;
    glfwGetFramebufferSize(WindowController::get().getWindowData().window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    if (_programState == ProgramState::FirstTick) {
        processFirstTick();
    } else if (_programState == ProgramState::LoadingScreen) {
        processLoadingScreen();
    } else if (_programState == ProgramState::FadeOutLoadingScreen) {
        processFadeOutLoadingScreen();
    } else if (_programState == ProgramState::FadeInUI) {
        processFadeInUI();
    } else if (_programState == ProgramState::OperatingMode) {
        processOperatingMode();
    } else if (_programState == ProgramState::ScheduleExit) {
        processScheduleExit();
    } else if (_programState == ProgramState::Exiting) {
        processExiting();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(WindowController::get().getWindowData().window);
}

void MainLoopController::shutdown()
{
    GlobalSettings::get().setValue("controllers.main loop.save on exit", _saveOnExit);
}

void MainLoopController::scheduleClosing()
{
    _programState = ProgramState::ScheduleExit;
}

bool MainLoopController::shouldClose() const
{
    return _programState == ProgramState::Finished;
}

void MainLoopController::processFirstTick()
{
    drawLoadingScreen();

    auto senderInfo = SenderInfo{.senderId = SenderId{StartupSenderId}, .wishResultData = true, .wishErrorInfo = true};
    auto readData = ReadSimulationRequestData{Const::AutosaveFile};
    _loadSimRequestId = _persisterFacade->scheduleReadSimulation(senderInfo, readData);
    _programState = ProgramState::LoadingScreen;

    OverlayController::get().process();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MainLoopController::processLoadingScreen()
{
    drawLoadingScreen();

    auto requestedSimState = _persisterFacade->getRequestState(_loadSimRequestId).value();
    if (requestedSimState == PersisterRequestState::Finished) {
        auto const& data = _persisterFacade->fetchReadSimulationData(_loadSimRequestId);
        auto const& deserializedSim = data.deserializedSimulation;
        _simulationFacade->newSimulation(
            deserializedSim.auxiliaryData.timestep, deserializedSim.auxiliaryData.generalSettings, deserializedSim.auxiliaryData.simulationParameters);
        _simulationFacade->setClusteredSimulationData(deserializedSim.mainData);
        _simulationFacade->setStatisticsHistory(deserializedSim.statistics);
        _simulationFacade->setRealTime(deserializedSim.auxiliaryData.realTime);
        Viewport::get().setCenterInWorldPos(deserializedSim.auxiliaryData.center);
        Viewport::get().setZoomFactor(deserializedSim.auxiliaryData.zoom);
        TemporalControlWindow::get().onSnapshot();

        _simulationLoadedTimepoint = std::chrono::steady_clock::now();
        _programState = ProgramState::FadeOutLoadingScreen;
    }
    if (requestedSimState == PersisterRequestState::Error) {
        GenericMessageDialog::get().information("错误", "默认的模拟器文件无法读取。\n一个空的模拟器文件将被创建。");

        DeserializedSimulation deserializedSim;
        deserializedSim.auxiliaryData.generalSettings.worldSizeX = 1000;
        deserializedSim.auxiliaryData.generalSettings.worldSizeY = 500;
        deserializedSim.auxiliaryData.timestep = 0;
        deserializedSim.auxiliaryData.zoom = 12.0f;
        deserializedSim.auxiliaryData.center = {500.0f, 250.0f};
        deserializedSim.auxiliaryData.realTime = std::chrono::milliseconds(0);

        _simulationFacade->newSimulation(
            deserializedSim.auxiliaryData.timestep, deserializedSim.auxiliaryData.generalSettings, deserializedSim.auxiliaryData.simulationParameters);
        _simulationFacade->setClusteredSimulationData(deserializedSim.mainData);
        _simulationFacade->setStatisticsHistory(deserializedSim.statistics);
        _simulationFacade->setRealTime(deserializedSim.auxiliaryData.realTime);
        Viewport::get().setCenterInWorldPos(deserializedSim.auxiliaryData.center);
        Viewport::get().setZoomFactor(deserializedSim.auxiliaryData.zoom);
        TemporalControlWindow::get().onSnapshot();

        _simulationLoadedTimepoint = std::chrono::steady_clock::now();
        _programState = ProgramState::FadeOutLoadingScreen;
    }

    OverlayController::get().process();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MainLoopController::processFadeOutLoadingScreen()
{
    drawLoadingScreen();

    SimulationView::get().draw();

    decreaseAlphaForFadeOutLoadingScreen();
    if (ImGui::GetStyle().Alpha == 0.0f) {
        _programState = ProgramState::FadeInUI;
        _fadedOutTimepoint = std::chrono::steady_clock::now();
    }
}

void MainLoopController::processFadeInUI()
{
    SimulationView::get().draw();

    pushGlobalStyle();
    processMenubar();
    MainLoopEntityController::get().process();
    OverlayController::get().process();
    SimulationView::get().processSimulationScrollbars();
    popGlobalStyle();

    increaseAlphaForFadeInUI();
    if (ImGui::GetStyle().Alpha == 1.0f) {
        printOverlayMessage(Const::AutosaveFileWithoutPath.string());
        _programState = ProgramState::OperatingMode;
    }
}

void MainLoopController::processOperatingMode()
{
    SimulationView::get().draw();

    pushGlobalStyle();
    processMenubar();
    MainLoopEntityController::get().process();
    OverlayController::get().process();
    SimulationView::get().processSimulationScrollbars();
    popGlobalStyle();

    FpsController::get().processForceFps(WindowController::get().getFps());

    if (glfwWindowShouldClose(WindowController::get().getWindowData().window)) {
        scheduleClosing();
    }
}

void MainLoopController::processScheduleExit()
{
    if (_saveOnExit) {
        printOverlayMessage("正在退出前保存文件...");

        auto senderInfo = SenderInfo{.senderId = SenderId{StartupSenderId}, .wishResultData = true, .wishErrorInfo = false};
        auto saveData = SaveSimulationRequestData{Const::AutosaveFile, Viewport::get().getZoomFactor(), Viewport::get().getCenterInWorldPos()};
        _saveSimRequestId = _persisterFacade->scheduleSaveSimulation(senderInfo, saveData);
        _programState = ProgramState::Exiting;
    } else {
        _programState = ProgramState::Finished;
    }
}

void MainLoopController::processExiting()
{
    SimulationView::get().draw();

    pushGlobalStyle();
    processMenubar();
    MainLoopEntityController::get().process();
    OverlayController::get().process();
    SimulationView::get().processSimulationScrollbars();
    popGlobalStyle();

    FpsController::get().processForceFps(WindowController::get().getFps());

    auto requestedSimState = _persisterFacade->getRequestState(_saveSimRequestId).value();
    if (requestedSimState == PersisterRequestState::Finished) {
        _persisterFacade->fetchSaveSimulationData(_saveSimRequestId);
        _programState = ProgramState::Finished;
    } else if (requestedSimState == PersisterRequestState::Error) {
        _programState = ProgramState::Finished;
    }
}

void MainLoopController::drawLoadingScreen()
{
    //background color
    glClearColor(0, 0, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto& styleRep = StyleRepository::get();
    auto center = ImGui::GetMainViewport()->GetCenter();
    auto bottom = ImGui::GetMainViewport()->Pos.y + ImGui::GetMainViewport()->Size.y;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    auto imageScale = scale(1.0f);
    ImGui::SetNextWindowSize(ImVec2(_logo.width * imageScale + 10.0f, _logo.height * imageScale + 10.0f));

    ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("##startup", NULL, windowFlags);
    ImGui::Image((void*)(intptr_t)_logo.textureId, ImVec2(_logo.width * imageScale, _logo.height * imageScale));
    ImGui::End();

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImColor textColor = Const::ProgramVersionTextColor;
    textColor.Value.w *= ImGui::GetStyle().Alpha;

    drawList->AddText(styleRep.getReefLargeFont(), scale(48.0f), {center.x - scale(175), bottom - scale(200)}, textColor, "Artificial Life Environment");

    auto versionString = "版本号 " + Const::ProgramVersion;
    drawList->AddText(
        styleRep.getReefMediumFont(),
        scale(24.0f),
        {center.x - scale(toFloat(versionString.size()) * 3.4f), bottom - scale(140)},
        textColor,
        versionString.c_str());

    if (GlobalSettings::get().isDebugMode()) {
        drawList->AddText(
            styleRep.getReefMediumFont(),
            scale(24.0f),
            {center.x - scale(24.0f),  bottom - scale(100)},
            textColor,
            "DEBUG");
    }
}

void MainLoopController::decreaseAlphaForFadeOutLoadingScreen()
{
    auto now = std::chrono::steady_clock::now();
    auto millisecSinceActivation = std::chrono::duration_cast<std::chrono::milliseconds>(now - *_simulationLoadedTimepoint).count();
    millisecSinceActivation = std::min(FadeOutDuration, millisecSinceActivation);
    auto alphaFactor = 1.0f - toFloat(millisecSinceActivation) / FadeOutDuration;
    ImGui::GetStyle().Alpha = alphaFactor;
}

void MainLoopController::increaseAlphaForFadeInUI()
{
    auto now = std::chrono::steady_clock::now();
    auto millisecSinceActivation = std::chrono::duration_cast<std::chrono::milliseconds>(now - *_fadedOutTimepoint).count();
    millisecSinceActivation = std::min(FadeInDuration, millisecSinceActivation);
    auto alphaFactor = toFloat(millisecSinceActivation) / FadeInDuration;
    ImGui::GetStyle().Alpha = alphaFactor;
}

void MainLoopController::processMenubar()
{
    auto& io = ImGui::GetIO();

    AlienImGui::BeginMenuBar();
    AlienImGui::MenuShutdownButton([&] { ExitDialog::get().open(); });
    ImGui::Dummy(ImVec2(scale(10.0f), 0.0f));

    AlienImGui::BeginMenu(" " ICON_FA_GAMEPAD "  模拟器 ", _simulationMenuOpened);
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("新建").keyCtrl(true).key(ImGuiKey_N), [&] { NewSimulationDialog::get().open(); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("打开").keyCtrl(true).key(ImGuiKey_O), [&] { FileTransferController::get().onOpenSimulationDialog(); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("保存").keyCtrl(true).key(ImGuiKey_S), [&] { FileTransferController::get().onSaveSimulationDialog(); });
    AlienImGui::MenuSeparator();
    auto running = _simulationFacade->isSimulationRunning();
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("运行").key(ImGuiKey_Space).disabled(running).closeMenuWhenItemClicked(false), [&] {
        _simulationFacade->runSimulation();
        printOverlayMessage("运行");
    });
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("暂停").key(ImGuiKey_Space).disabled(!running).closeMenuWhenItemClicked(false), [&] {
        _simulationFacade->pauseSimulation();
        printOverlayMessage("暂停");
    });
    AlienImGui::EndMenu();

    AlienImGui::BeginMenu(" " ICON_FA_GLOBE "  网络 ", _networkMenuOpened);
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("浏览器").keyAlt(true).key(ImGuiKey_W).closeMenuWhenItemClicked(false).selected(BrowserWindow::get().isOn()),
        [&] { BrowserWindow::get().setOn(!BrowserWindow::get().isOn()); });
    AlienImGui::MenuSeparator();
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("登录").keyAlt(true).key(ImGuiKey_L).disabled(NetworkService::get().isLoggedIn()), [&] {
        LoginDialog::get().open();
    });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("登出")
            .keyAlt(true)
            .key(ImGuiKey_T)
            .closeMenuWhenItemClicked(false)
            .disabled(!NetworkService::get().isLoggedIn()),
        [&] {
            NetworkService::get().logout();
            BrowserWindow::get().onRefresh();
        });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("上传模拟器").keyAlt(true).key(ImGuiKey_D).disabled(!NetworkService::get().isLoggedIn()),
        [&] { UploadSimulationDialog::get().open(NetworkResourceType_Simulation); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("上传基因组").keyAlt(true).key(ImGuiKey_Q).disabled(!NetworkService::get().isLoggedIn()),
        [&] { UploadSimulationDialog::get().open(NetworkResourceType_Genome); });
    AlienImGui::MenuSeparator();
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("删除用户").keyAlt(true).key(ImGuiKey_J).disabled(!NetworkService::get().isLoggedIn()), [&] {
        DeleteUserDialog::get().open();
    });
    AlienImGui::EndMenu();

    AlienImGui::BeginMenu(" " ICON_FA_WINDOW_RESTORE "  窗口 ", _windowMenuOpened);
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("时间控制器")
            .keyAlt(true)
            .key(ImGuiKey_1)
            .selected(TemporalControlWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { TemporalControlWindow::get().setOn(!TemporalControlWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("空间控制器")
            .keyAlt(true)
            .key(ImGuiKey_2)
            .selected(SpatialControlWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { SpatialControlWindow::get().setOn(!SpatialControlWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("统计数据")
            .keyAlt(true)
            .key(ImGuiKey_3)
            .selected(StatisticsWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { StatisticsWindow::get().setOn(!StatisticsWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("模拟器参数")
            .keyAlt(true)
            .key(ImGuiKey_4)
            .selected(SimulationParametersWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationParametersWindow::get().setOn(!SimulationParametersWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("放射粒子源")
            .keyAlt(true)
            .key(ImGuiKey_5)
            .selected(RadiationSourcesWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { RadiationSourcesWindow::get().setOn(!RadiationSourcesWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("着色器参数")
            .keyAlt(true)
            .key(ImGuiKey_6)
            .selected(ShaderWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { ShaderWindow::get().setOn(!ShaderWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("自动保存").keyAlt(true).key(ImGuiKey_7).selected(AutosaveWindow::get().isOn()).closeMenuWhenItemClicked(false),
        [&] { AutosaveWindow::get().setOn(!AutosaveWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("日志").keyAlt(true).key(ImGuiKey_8).selected(LogWindow::get().isOn()).closeMenuWhenItemClicked(false),
        [&] { LogWindow::get().setOn(!LogWindow::get().isOn()); });
    AlienImGui::EndMenu();

    AlienImGui::BeginMenu(" " ICON_FA_PEN_ALT "  编辑器 ", _editorMenuOpened);
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("启用")
            .keyAlt(true)
            .key(ImGuiKey_E)
            .selected(SimulationInteractionController::get().isEditMode())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationInteractionController::get().setEditMode(!SimulationInteractionController::get().isEditMode()); });
    AlienImGui::MenuSeparator();
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("所选项")
            .keyAlt(true)
            .key(ImGuiKey_S)
            .selected(SelectionWindow::get().isOn())
            .disabled(!SimulationInteractionController::get().isEditMode())
            .closeMenuWhenItemClicked(false),
        [&] { SelectionWindow::get().setOn(!SelectionWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("制作器")
            .keyAlt(true)
            .key(ImGuiKey_R)
            .selected(CreatorWindow::get().isOn())
            .disabled(!SimulationInteractionController::get().isEditMode())
            .closeMenuWhenItemClicked(false),
        [&] { CreatorWindow::get().setOn(!CreatorWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("图式编辑器")
            .keyAlt(true)
            .key(ImGuiKey_M)
            .selected(PatternEditorWindow::get().isOn())
            .disabled(!SimulationInteractionController::get().isEditMode())
            .closeMenuWhenItemClicked(false),
        [&] { PatternEditorWindow::get().setOn(!PatternEditorWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("基因组编辑器")
            .keyAlt(true)
            .key(ImGuiKey_B)
            .selected(GenomeEditorWindow::get().isOn())
            .disabled(!SimulationInteractionController::get().isEditMode())
            .closeMenuWhenItemClicked(false),
        [&] { GenomeEditorWindow::get().setOn(!GenomeEditorWindow::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("复制器")
            .keyAlt(true)
            .key(ImGuiKey_A)
            .selected(MultiplierWindow::get().isOn())
            .disabled(!SimulationInteractionController::get().isEditMode())
            .closeMenuWhenItemClicked(false),
        [&] { MultiplierWindow::get().setOn(!MultiplierWindow::get().isOn()); });
    AlienImGui::MenuSeparator();
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("查看物体")
            .keyAlt(true)
            .key(ImGuiKey_N)
            .disabled(!SimulationInteractionController::get().isEditMode() || !PatternEditorWindow::get().isObjectInspectionPossible()),
        [&] { EditorController::get().onInspectSelectedObjects(); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("查看目标基因组")
            .keyAlt(true)
            .key(ImGuiKey_F)
            .disabled(!SimulationInteractionController::get().isEditMode() || !PatternEditorWindow::get().isGenomeInspectionPossible()),
        [&] { EditorController::get().onInspectSelectedGenomes(); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("关闭查看")
            .key(ImGuiKey_Escape)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().areInspectionWindowsActive()),
        [&] { EditorController::get().onCloseAllInspectorWindows(); });
    AlienImGui::MenuSeparator();
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("复制")
            .keyCtrl(true)
            .key(ImGuiKey_C)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().isCopyingPossible()),
        [&] { EditorController::get().onCopy(); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("粘贴")
            .keyCtrl(true)
            .key(ImGuiKey_V)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().isPastingPossible()),
        [&] { EditorController::get().onPaste(); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("删除")
            .key(ImGuiKey_Delete)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().isCopyingPossible()),
        [&] { EditorController::get().onDelete(); });
    AlienImGui::EndMenu();

    AlienImGui::BeginMenu(" " ICON_FA_EYE "  视图 ", _viewMenuOpened);
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("显示细胞信息")
            .keyAlt(true)
            .key(ImGuiKey_O)
            .selected(SimulationView::get().isOverlayActive())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationView::get().setOverlayActive(!SimulationView::get().isOverlayActive()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("显示消息")
            .keyAlt(true)
            .key(ImGuiKey_X)
            .selected(OverlayController::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { OverlayController::get().setOn(!OverlayController::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("绘制UI").keyAlt(true).key(ImGuiKey_U).selected(UiController::get().isOn()).closeMenuWhenItemClicked(false),
        [&] { UiController::get().setOn(!UiController::get().isOn()); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters()
            .name("绘制模拟器画面")
            .keyAlt(true)
            .key(ImGuiKey_I)
            .selected(SimulationView::get().isRenderSimulation())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationView::get().setRenderSimulation(!SimulationView::get().isRenderSimulation()); });
    AlienImGui::EndMenu();

    AlienImGui::BeginMenu(" " ICON_FA_TOOLS "  工具 ", _toolsMenuOpened);
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("大量操作").keyAlt(true).key(ImGuiKey_H), [&] { MassOperationsDialog::get().open(); });
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("图式分析").keyAlt(true).key(ImGuiKey_P), [&] { PatternAnalysisDialog::get().show(); });
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("从图像生成图示").keyAlt(true).key(ImGuiKey_G), [&] { ImageToPatternDialog::get().show(); });
    AlienImGui::EndMenu();

    AlienImGui::BeginMenu(" " ICON_FA_COG "  设置 ", _settingsMenuOpened, false);
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("退出时保存").selected(_saveOnExit).closeMenuWhenItemClicked(false), [&] { _saveOnExit = !_saveOnExit; });
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("CUDA设置").keyAlt(true).key(ImGuiKey_C), [&] { GpuSettingsDialog::get().open(); });
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("显示设置").keyAlt(true).key(ImGuiKey_V), [&] { DisplaySettingsDialog::get().open(); });
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("网络设置").keyAlt(true).key(ImGuiKey_K), [&] { NetworkSettingsDialog::get().open(); });
    AlienImGui::EndMenu();

    AlienImGui::BeginMenu(" " ICON_FA_LIFE_RING "  帮助 ", _helpMenuOpened);
    AlienImGui::MenuItem(AlienImGui::MenuItemParameters().name("关于ALIEN"), [&] { AboutDialog::get().open(); });
    AlienImGui::MenuItem(
        AlienImGui::MenuItemParameters().name("入门手册").selected(GettingStartedWindow::get().isOn()).closeMenuWhenItemClicked(false),
        [&] { GettingStartedWindow::get().setOn(!GettingStartedWindow::get().isOn()); });
    AlienImGui::EndMenu();
    AlienImGui::EndMenuBar();

    //further hotkeys
    if (!io.WantCaptureKeyboard) {
        if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
            if (WindowController::get().isDesktopMode()) {
                WindowController::get().setWindowedMode();
            } else {
                WindowController::get().setDesktopMode();
            }
        }
    }
}

void MainLoopController::pushGlobalStyle()
{
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, Const::SliderBarWidth);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, Const::WindowsRounding);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)Const::HeaderHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)Const::HeaderActiveColor);
    ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)Const::HeaderColor);
}

void MainLoopController::popGlobalStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
