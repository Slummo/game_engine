#pragma once

#include "contexts/icontext.h"
#include "core/types/id.h"
#include "managers/entity_manager.h"

#include <glad/glad.h>

struct DebugObject {
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ebo = 0;

    ~DebugObject();
};

struct RenderContext : public IContext {
    RenderContext(EntityManager& em);

    bool wiremode = false;
    bool hitbox_render_enabled = false;
    bool debug_render_enabled = false;
    AssetID environment_id;
    AssetID colored_line_shader_id;
    float fps = 0;

    DebugObject hitbox;
    DebugObject arrow;

private:
    void create_hitbox();
    void create_arrow(EntityManager& em);
};