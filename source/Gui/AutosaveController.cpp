#include "AutosaveController.h"

#include <imgui.h>

#include "Base/Resources.h"
#include "Base/GlobalSettings.h"
#include "PersisterInterface/SerializerService.h"
#include "EngineInterface/SimulationFacade.h"

#include "Viewport.h"
#include "DelayedExecutionController.h"
#include "OverlayController.h"
#include "SerializationHelperService.h"
#include "MainLoopEntityController.h"

namespace
{
    auto constexpr MinutesForAutosave = 40;
}

void AutosaveController::init(SimulationFacade simulationFacade)
{
    _simulationFacade = simulationFacade;
    _startTimePoint = std::chrono::steady_clock::now();
    _on = GlobalSettings::get().getBool("controllers.auto save.active", true);
}

void AutosaveController::shutdown()
{
    GlobalSettings::get().setBool("controllers.auto save.active", _on);
    if (!_on) {
        return;
    }
    //onSave();
}

bool AutosaveController::isOn() const
{
    return _on;
}

void AutosaveController::setOn(bool value)
{
    _on = value;
}

void AutosaveController::process()
{
    if (!_on) {
        return;
    }

    auto durationSinceStart = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - *_startTimePoint).count();
    if (durationSinceStart > 0 && durationSinceStart % MinutesForAutosave == 0 && !_alreadySaved) {
        printOverlayMessage("Auto saving ...");
        delayedExecution([&] { onSave(); });
        _alreadySaved = true;
    }
    if (durationSinceStart > 0 && durationSinceStart % MinutesForAutosave == 1 && _alreadySaved) {
        _alreadySaved = false;
    }
}

void AutosaveController::onSave()
{
    DeserializedSimulation sim = SerializationHelperService::get().getDeserializedSerialization(_simulationFacade);
    SerializerService::get().serializeSimulationToFiles(Const::AutosaveFile, sim);
}
