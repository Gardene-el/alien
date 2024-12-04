#include "DisplaySettingsDialog.h"

#include <sstream>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Base/LoggingService.h"

#include "AlienImGui.h"
#include "WindowController.h"
#include "StyleRepository.h"

namespace
{
    auto const RightColumnWidth = 185.0f;
}

void DisplaySettingsDialog::initIntern()
{
    auto primaryMonitor = glfwGetPrimaryMonitor();
    _videoModes = glfwGetVideoModes(primaryMonitor, &_videoModesCount);
    _videoModeStrings = createVideoModeStrings();
}

DisplaySettingsDialog::DisplaySettingsDialog()
    : AlienDialog("显示设置")
{}

void DisplaySettingsDialog::processIntern()
{
    auto isFullscreen = !WindowController::get().isWindowedMode();

    if (AlienImGui::ToggleButton(AlienImGui::ToggleButtonParameters().name("全屏"), isFullscreen)) {
        if (isFullscreen) {
            setFullscreen(_selectionIndex);
        } else {
            _origSelectionIndex = _selectionIndex;
            WindowController::get().setWindowedMode();
        }
    }

    ImGui::BeginDisabled(!isFullscreen);

    if (AlienImGui::Combo(
            AlienImGui::ComboParameters().name("分辨率").textWidth(RightColumnWidth).defaultValue(_origSelectionIndex).values(_videoModeStrings),
            _selectionIndex)) {

        setFullscreen(_selectionIndex);
    }
    ImGui::EndDisabled();

    auto fps = WindowController::get().getFps();
    if (AlienImGui::SliderInt(
            AlienImGui::SliderIntParameters()
                .name("每秒帧数（FPS）")
                .textWidth(RightColumnWidth)
                .defaultValue(&_origFps)
                .min(20)
                .max(100)
                .tooltip("高帧率会增加 GPU 渲染的工作负载，从而降低模拟速度（每秒时间步数）。"),
            &fps)) {
        WindowController::get().setFps(fps);
    }

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienImGui::Separator();

    if (AlienImGui::Button("确认")) {
        close();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienImGui::Button("取消")) {
        close();
        WindowController::get().setMode(_origMode);
        WindowController::get().setFps(_origFps);
        _selectionIndex = _origSelectionIndex;
    }
}

void DisplaySettingsDialog::openIntern()
{
    _selectionIndex = getSelectionIndex();
    _origSelectionIndex = _selectionIndex;
    _origMode = WindowController::get().getMode();
    _origFps = WindowController::get().getFps();
}

void DisplaySettingsDialog::setFullscreen(int selectionIndex)
{
    if (0 == selectionIndex) {
        WindowController::get().setDesktopMode();
    } else {
        WindowController::get().setUserDefinedResolution(_videoModes[selectionIndex - 1]);
    }
}

namespace
{
    bool operator==(GLFWvidmode const& m1, GLFWvidmode const& m2)
    {
        return m1.width == m2.width && m1.height == m2.height && m1.redBits == m2.redBits
            && m1.greenBits == m2.greenBits && m1.blueBits == m2.blueBits && m1.refreshRate == m2.refreshRate;
    }
}

int DisplaySettingsDialog::getSelectionIndex() const
{
    auto result = 0;
    if (!WindowController::get().isWindowedMode() && !WindowController::get().isDesktopMode()) {
        auto userMode = WindowController::get().getUserDefinedResolution();
        for (int i = 0; i < _videoModesCount; ++i) {
            if (_videoModes[i] == userMode) {
                return i + 1;
            }
        }
    }
    return result;
}

namespace
{
    std::string createVideoModeString(GLFWvidmode const& videoMode)
    {
        std::stringstream ss;
        ss << videoMode.width << " x " << videoMode.height << " @ " << videoMode.refreshRate << "Hz";
        return ss.str();
    }
}

std::vector<std::string> DisplaySettingsDialog::createVideoModeStrings() const
{
    std::vector<std::string> result;
    result.emplace_back("桌面");
    for (int i = 0; i < _videoModesCount; ++i) {
        result.emplace_back(createVideoModeString(_videoModes[i]));
    }

    return result;
}
