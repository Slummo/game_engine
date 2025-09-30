#pragma once

#include "contexts/icontext.h"
#include "core/types/id.h"

#include <glm/glm.hpp>

class Window;
class Engine;
class EntityManager;

struct DebugObject {
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ebo = 0;

    ~DebugObject();
};

struct DebugContext : public IContext {
    DebugContext(Engine& engine, Window& window);

    Window& window;

    bool wiremode = false;
    bool active = false;

    AssetID colored_line_shader_id;

    DebugObject hitbox;
    DebugObject arrow;

    float fps = 0;

private:
    void create_hitbox();
    void create_arrow(EntityManager& em);
};