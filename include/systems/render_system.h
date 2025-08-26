#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <unordered_map>

class RenderSystem : public ISystem {
public:
    RenderSystem() = default;

    void init(ECS& ecs) override;
    void update(ECS&, float dt) override;
    void shutdown(ECS& ecs) override;

    void set_environment(AssetID hdr_env);

    bool is_hitbox_render_enabled();
    void toggle_hitbox_render_enabled();

    bool is_debug_render_enabled();
    void toggle_debug_render_enabled();

private:
    AssetID m_environment;

    bool m_hitbox_render_enabled = false;
    // Hitbox buffers
    unsigned int m_h_vao = 0;
    unsigned int m_h_vbo = 0;
    unsigned int m_h_ebo = 0;

    bool m_debug_render_enabled = false;
    // Arrow buffers
    unsigned int m_a_vao = 0;
    unsigned int m_a_vbo = 0;

    AssetID m_colored_line_shader_id;

    void render_scene(ECS& ecs, CameraComponent& camera);
    void render_hitboxes(ECS& ecs, CameraComponent& camera);
    void render_debug(ECS& ecs, CameraComponent& camera);

    void create_wire_cube();
    void destroy_wire_cube();
    void create_arrow(ECS& ecs);
    void destroy_arrow();
};
