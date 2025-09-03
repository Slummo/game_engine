#include "systems/render_system.h"
#include "managers/asset_manager.h"

#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>
#include <GLFW/glfw3.h>

void RenderSystem::init(EntityManager& em, InputContext& ic) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    create_wire_cube();
    create_arrow(em);
    m_colored_line_shader_id = AssetManager::instance().load_asset<ShaderAsset>("colored_line/");

    ic.register_action("ToggleWiremode", InputType::Key, GLFW_KEY_M, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleWiremode", [this]() {
        m_wiremode = !m_wiremode;
        glPolygonMode(GL_FRONT_AND_BACK, m_wiremode ? GL_LINE : GL_FILL);
    });
    ic.register_action("ToggleHitboxes", InputType::Key, GLFW_KEY_H, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleHitboxes", [this]() { m_hitbox_render_enabled = !m_hitbox_render_enabled; });
    ic.register_action("ToggleDebug", InputType::Key, GLFW_KEY_F, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleDebug", [this]() { m_debug_render_enabled = !m_debug_render_enabled; });
}

void RenderSystem::update(EntityManager& em, InputContext& /*ic*/) {
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Only get the main camera for now
    for (auto [_e, _pl, tr, cam] : em.entities_with<Player, Transform, Camera>()) {
        render_scene(em, cam);
        render_hitboxes(em, cam);
        render_debug(em, cam);
    }
}

void RenderSystem::shutdown(EntityManager& /*em*/, InputContext& /*ic*/) {
    AssetManager::instance().get_asset<ShaderAsset>(m_colored_line_shader_id).destroy();
    destroy_wire_cube();
    destroy_arrow();
}

void RenderSystem::render_scene(EntityManager& em, Camera& cam) {
    AssetManager& am = AssetManager::instance();

    // Find a directional light
    // TODO update this
    Light light;
    for (auto [_e, tr, l] : em.entities_with<Transform, Light>()) {
        if (l.type == LightType::Directional) {
            light = l;
            break;
        }
    }

    for (auto [e, tr, m] : em.entities_with<Transform, Model>()) {
        if (em.has_component<Light>(e) && !m_debug_render_enabled) {
            // Render light models only if debug render is enabled
            continue;
        }

        // From frustum culling
        if (!m.visible) {
            continue;
        }

        ModelAsset& model = am.get_asset<ModelAsset>(m.asset_id);
        for (AssetID mesh_id : model.meshes()) {
            MeshAsset& mesh = am.get_asset<MeshAsset>(mesh_id);
            Material& mat = am.get_asset<Material>(mesh.material_id());
            const ShaderAsset& shader = mat.bound_shader();

            // Vertex shader
            const glm::mat4 model_mat = tr.model_matrix();
            glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(model_mat)));
            shader.set_matrix_4f("Projection", cam.proj_matrix());
            shader.set_matrix_4f("View", cam.view_matrix());
            shader.set_matrix_4f("Model", model_mat);
            shader.set_matrix_3f("Normal", normal_mat);

            // Fragment shader
            mat.set_uniforms();

            shader.set_vec_3f("light.direction", light.direction);
            shader.set_vec_3f("light.color", light.color);
            shader.set_float("light.intensity", light.intensity);
            shader.set_bool("light.is_directional", true);

            shader.set_vec_3f("camera_world_pos", cam.world_position());

            mesh.draw();

            TextureAsset::unbind();
        }
    }
    // TODO UI, post-processing, debug overlays
}

void RenderSystem::render_hitboxes(EntityManager& em, Camera& cam) {
    if (!m_hitbox_render_enabled) {
        return;
    }

    AssetManager& am = AssetManager::instance();

    ShaderAsset& shader = am.get_asset<ShaderAsset>(m_colored_line_shader_id);
    if (am.last_used_shader() != m_colored_line_shader_id) {
        shader.use();
        am.set_last_used_shader(m_colored_line_shader_id);
    }

    shader.set_matrix_4f("Projection", cam.proj_matrix());
    shader.set_matrix_4f("View", cam.view_matrix());

    glLineWidth(2.0f);

    for (auto [_e, tr, col] : em.entities_with<Transform, Collider>()) {
        // Red = physical, green = trigger
        glm::vec3 color = col.is_trigger ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        shader.set_vec_3f("color", color);

        glm::quat rot = tr.rotation();
        glm::vec3 scale = tr.scale();
        glm::vec3 center = tr.position() + (rot * (col.offset * scale));
        glm::vec3 half_extents = (col.size * 0.5f) * scale;

        // Build model matrix
        glm::mat4 model(1.0f);
        model = glm::translate(model, center);
        model *= glm::mat4_cast(rot);
        model = glm::scale(model, half_extents * 2.0f);  // full size

        shader.set_matrix_4f("Model", model);

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(m_h_vao);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
        glEnable(GL_DEPTH_TEST);
    }

    glLineWidth(1.0f);
}

void RenderSystem::render_debug(EntityManager& /*em*/, Camera& cam) {
    if (!m_debug_render_enabled) {
        return;
    }

    AssetManager& am = AssetManager::instance();

    ShaderAsset& shader = am.get_asset<ShaderAsset>(m_colored_line_shader_id);
    if (am.last_used_shader() != m_colored_line_shader_id) {
        shader.use();
        am.set_last_used_shader(m_colored_line_shader_id);
    }

    shader.set_matrix_4f("Projection", cam.proj_matrix());
    shader.set_matrix_4f("View", cam.view_matrix());
    shader.set_matrix_4f("Model", glm::mat4(1.0f));           // identity, since positions are in world space
    shader.set_vec_3f("color", glm::vec3(1.0f, 1.0f, 0.0f));  // yellow

    // Draw directional light arrow
    glLineWidth(5.0f);
    glBindVertexArray(m_a_vao);
    glDrawArrays(GL_LINES, 0, 2);
    glLineWidth(1.0f);
}

void RenderSystem::set_environment(AssetID hdr_env) {
    m_environment = hdr_env;
}

void RenderSystem::create_wire_cube() {
    // 8 verts of unit cube centered at origin
    float verts[] = {
        -0.5f, -0.5f, -0.5f,  // 0
        0.5f,  -0.5f, -0.5f,  // 1
        0.5f,  0.5f,  -0.5f,  // 2
        -0.5f, 0.5f,  -0.5f,  // 3
        -0.5f, -0.5f, 0.5f,   // 4
        0.5f,  -0.5f, 0.5f,   // 5
        0.5f,  0.5f,  0.5f,   // 6
        -0.5f, 0.5f,  0.5f    // 7
    };

    uint32_t inds[] = {
        0, 1, 1, 2, 2, 3, 3, 0,  // back rectangle
        4, 5, 5, 6, 6, 7, 7, 4,  // front rectangle
        0, 4, 1, 5, 2, 6, 3, 7   // connections
    };

    glGenVertexArrays(1, &m_h_vao);
    glGenBuffers(1, &m_h_vbo);
    glGenBuffers(1, &m_h_ebo);

    glBindVertexArray(m_h_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_h_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_h_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glBindVertexArray(0);
}

void RenderSystem::destroy_wire_cube() {
    if (m_h_ebo) {
        glDeleteBuffers(1, &m_h_ebo);
        m_h_ebo = 0;
    }
    if (m_h_vbo) {
        glDeleteBuffers(1, &m_h_vbo);
        m_h_vbo = 0;
    }
    if (m_h_vao) {
        glDeleteVertexArrays(1, &m_h_vao);
        m_h_vao = 0;
    }
}

void RenderSystem::create_arrow(EntityManager& em) {
    Transform tr;
    Light light;
    for (auto [_e, l_tr, l] : em.entities_with<Transform, Light>()) {
        if (l.type == LightType::Directional) {
            tr = l_tr;
            light = l;
            break;
        }
    }

    glm::vec3 start = tr.position();
    glm::vec3 dir = light.direction;
    float length = 7.0f;
    glm::vec3 end = start + dir * length;

    float vertices[] = {start.x, start.y, start.z, end.x, end.y, end.z};

    glGenVertexArrays(1, &m_a_vao);
    glGenBuffers(1, &m_a_vbo);

    glBindVertexArray(m_a_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_a_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}

void RenderSystem::destroy_arrow() {
    if (m_a_vbo) {
        glDeleteBuffers(1, &m_a_vbo);
        m_a_vbo = 0;
    }
    if (m_a_vao) {
        glDeleteVertexArrays(1, &m_a_vao);
        m_a_vao = 0;
    }
}