#include "ShaderWindow.h"

#include "AlienImGui.h"
#include "SimulationView.h"

namespace
{
    auto const RightColumnWidth = 140.0f;
}

ShaderWindow::ShaderWindow()
: AlienWindow("着色器参数", "windows.shader", false)
{
}

void ShaderWindow::processIntern()
{
    auto const defaultBrightness = SimulationView::DefaultBrightness;
    auto const defaultContrast = SimulationView::DefaultContrast;
    auto const defaultMotionBlur = SimulationView::DefaultMotionBlur;

    auto brightness = SimulationView::get().getBrightness();
    auto contrast = SimulationView::get().getContrast();
    auto motionBlur = SimulationView::get().getMotionBlur();
    if (AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters().name("亮度").min(0).max(3.0f).textWidth(RightColumnWidth).defaultValue(&defaultBrightness), &brightness)) {
        SimulationView::get().setBrightness(brightness);
    }
    if (AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters().name("对比度").min(0).max(2.0f).textWidth(RightColumnWidth).defaultValue(&defaultContrast), &contrast)) {
        SimulationView::get().setContrast(contrast);
    }
    if (AlienImGui::SliderFloat(
            AlienImGui::SliderFloatParameters()
                .name("高斯模糊")
                .min(0)
                .max(10.0f)
                .textWidth(RightColumnWidth)
                .logarithmic(true)
                .defaultValue(&defaultMotionBlur),
            &motionBlur)) {
        SimulationView::get().setMotionBlur(motionBlur);
    }
}
