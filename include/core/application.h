#pragma once

#include "core/window.h"
#include "managers/entity_manager.h"
#include "managers/system_manager.h"
#include "managers/context_manager.h"

#include <memory>
#include <vector>

class Application {
public:
    Application();

    bool init();
    void run();
    void shutdown();

    InputContext& get_input_context();

private:
    Window window;

    EntityManager em;
    SystemManager sm;
    ContextManager cm;

    bool m_running;
    bool m_cursor_enabled;
    bool m_wiremode;
};
