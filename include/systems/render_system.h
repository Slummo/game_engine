#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <cstdint>

class RenderSystem : public ISystem<> {
public:
    RenderSystem() = default;

    void init(EntityManager& em) override;
    void update(EntityManager& em) override;
    void shutdown(EntityManager& em) override;

    void set_environment(AssetID hdr_env);

    bool is_hitbox_render_enabled();
    void toggle_hitbox_render_enabled();

    bool is_debug_render_enabled();
    void toggle_debug_render_enabled();

private:
    AssetID m_environment;

    bool m_hitbox_render_enabled = false;
    // Hitbox buffers
    uint32_t m_h_vao = 0;
    uint32_t m_h_vbo = 0;
    uint32_t m_h_ebo = 0;

    bool m_debug_render_enabled = false;
    // Arrow buffers
    uint32_t m_a_vao = 0;
    uint32_t m_a_vbo = 0;

    AssetID m_colored_line_shader_id;

    void render_scene(EntityManager& em, Camera& cam_c);
    void render_hitboxes(EntityManager& em, Camera& cam_c);
    void render_debug(EntityManager& em, Camera& cam_c);

    void create_wire_cube();
    void destroy_wire_cube();
    void create_arrow(EntityManager& em);
    void destroy_arrow();
};
