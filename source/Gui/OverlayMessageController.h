#pragma once

#include <chrono>

#include "PersisterInterface/PersisterFacade.h"

#include "Definitions.h"

class OverlayMessageController
{
    MAKE_SINGLETON(OverlayMessageController);

public:
    void setup(PersisterFacade const& persisterFacade);
    void process();

    void show(std::string const& message, bool withLightning = false);

    void setOn(bool value);

private:
    void processLoadingBar();
    void processMessage();

    PersisterFacade _persisterFacade;

    bool _show = false;
    bool _withLightning = false;
    bool _on = true;
    std::string _message;
    int _counter = 0;

    std::optional<std::chrono::steady_clock::time_point> _progressBarRefTimepoint;
    std::optional<std::chrono::steady_clock::time_point> _messageStartTimepoint;
    std::optional<std::chrono::steady_clock::time_point> _ticksLaterTimepoint;
};

inline void printOverlayMessage(std::string const& message, bool withLightning = false)
{
    OverlayMessageController::get().show(message, withLightning);
}