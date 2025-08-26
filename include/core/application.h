#pragma once

#include "core/window.h"
#include "core/ecs.h"
#include "managers/system_manager.h"
#include "managers/input_manager.h"

#include <memory>

class Application {
public:
    Application();

    bool init();
    void run();
    void shutdown();
    InputManager& get_input_manager();

private:
    Window m_window;
    ECS m_ecs;
    SystemManager m_sm;
    InputManager m_im;

    bool m_running;
    bool m_cursor_enabled;
    bool m_wiremode;
};
