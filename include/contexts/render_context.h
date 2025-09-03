#pragma once

#include "contexts/icontext.h"
#include "core/types/id.h"
#include "managers/asset_manager.h"

struct RenderContext : public IContext {
    RenderContext() {
        colored_line_shader_id = AssetManager::instance().load_asset<ShaderAsset>("colored_line/");
    }

    bool wiremode = false;

    AssetID environment_id;

    bool hitbox_render_enabled = false;
    // Hitbox buffers
    uint32_t h_vao = 0;
    uint32_t h_vbo = 0;
    uint32_t h_ebo = 0;

    bool debug_render_enabled = false;
    // Arrow buffers
    uint32_t a_vao = 0;
    uint32_t a_vbo = 0;

    AssetID colored_line_shader_id;
};