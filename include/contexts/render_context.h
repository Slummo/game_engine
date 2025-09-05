#pragma once

#include "contexts/icontext.h"
#include "core/types/id.h"
#include "managers/asset_manager.h"

struct RenderContext : public IContext {
    RenderContext() {
        AssetManager& am = AssetManager::instance();
        colored_line_shader_id = am.load_asset<ShaderAsset>("colored_line/");
        text_shader_id = am.load_asset<ShaderAsset>("text/");
        font_id = am.load_asset<FontAsset>("arial/ARIAL.TTF", "arial");
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

    // Text buffers
    uint32_t t_vao = 0;
    uint32_t t_vbo = 0;

    AssetID colored_line_shader_id;
    AssetID text_shader_id;

    AssetID font_id;

    float fps = 0;
};