#include "systems/render_system.h"
#include "managers/asset_manager.h"

#include <format>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

void RenderSystem::init(EntityManager& em, RenderContext& rc, CameraContext& /*cc*/, InputContext& ic) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    create_wire_cube(rc);
    create_arrow(em, rc);
    create_text(rc);

    ic.register_action("ToggleWiremode", InputType::Key, GLFW_KEY_M, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleWiremode", [&]() {
        rc.wiremode = !rc.wiremode;
        glPolygonMode(GL_FRONT_AND_BACK, rc.wiremode ? GL_LINE : GL_FILL);
    });
    ic.register_action("ToggleHitboxes", InputType::Key, GLFW_KEY_H, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleHitboxes", [&]() { rc.hitbox_render_enabled = !rc.hitbox_render_enabled; });
    ic.register_action("ToggleDebug", InputType::Key, GLFW_KEY_F, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleDebug", [&]() { rc.debug_render_enabled = !rc.debug_render_enabled; });
}

void RenderSystem::update(EntityManager& em, RenderContext& rc, CameraContext& cc, InputContext& /*ic*/) {
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Only get the main camera for now
    Camera& cam = cc.main_camera;
    render_scene(em, cam, rc);
    render_hitboxes(em, cam, rc);
    render_debug(em, cam, rc);

    std::string fps_string(std::format("FPS: {}", rc.fps));
    render_text(rc, fps_string, 10.0f, 560.0f, 0.5f, glm::vec3(1.0f));
}

void RenderSystem::shutdown(EntityManager& /*em*/, RenderContext& rc, CameraContext& /*cc*/, InputContext& /*ic*/) {
    AssetManager::instance().get_asset<ShaderAsset>(rc.colored_line_shader_id).destroy();
    destroy_wire_cube(rc);
    destroy_arrow(rc);
    destroy_text(rc);
}

void RenderSystem::render_scene(EntityManager& em, Camera& cam, RenderContext& rc) {
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
        if (em.has_component<Light>(e) && !rc.debug_render_enabled) {
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
}

void RenderSystem::render_hitboxes(EntityManager& em, Camera& cam, RenderContext& rc) {
    if (!rc.hitbox_render_enabled) {
        return;
    }

    AssetManager& am = AssetManager::instance();

    ShaderAsset& shader = am.get_asset<ShaderAsset>(rc.colored_line_shader_id);
    if (am.last_used_shader() != rc.colored_line_shader_id) {
        shader.use();
        am.set_last_used_shader(rc.colored_line_shader_id);
    }

    shader.set_matrix_4f("Projection", cam.proj_matrix());
    shader.set_matrix_4f("View", cam.view_matrix());

    glLineWidth(2.0f);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(rc.h_vao);

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

        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
    }

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

void RenderSystem::render_debug(EntityManager& /*em*/, Camera& cam, RenderContext& rc) {
    if (!rc.debug_render_enabled) {
        return;
    }

    AssetManager& am = AssetManager::instance();

    ShaderAsset& shader = am.get_asset<ShaderAsset>(rc.colored_line_shader_id);
    if (am.last_used_shader() != rc.colored_line_shader_id) {
        shader.use();
        am.set_last_used_shader(rc.colored_line_shader_id);
    }

    shader.set_matrix_4f("Projection", cam.proj_matrix());
    shader.set_matrix_4f("View", cam.view_matrix());
    shader.set_matrix_4f("Model", glm::mat4(1.0f));           // identity, since positions are in world space
    shader.set_vec_3f("color", glm::vec3(1.0f, 1.0f, 0.0f));  // yellow

    // Draw directional light arrow
    glLineWidth(5.0f);
    glBindVertexArray(rc.a_vao);
    glDrawArrays(GL_LINES, 0, 2);
    glLineWidth(1.0f);
    glBindVertexArray(0);
}

void RenderSystem::render_text(RenderContext& rc, const std::string& text, float x, float y, float scale,
                               const glm::vec3& color) {
    AssetManager& am = AssetManager::instance();

    ShaderAsset& shader = am.get_asset<ShaderAsset>(rc.text_shader_id);
    if (am.last_used_shader() != rc.text_shader_id) {
        shader.use();
        am.set_last_used_shader(rc.text_shader_id);
    }

    glm::mat4 proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    shader.set_matrix_4f("Projection", glm::value_ptr(proj));
    shader.set_vec_3f("color", color);

    FontAsset& font = am.get_asset<FontAsset>(rc.font_id);
    TextureAsset& font_tex = am.get_asset<TextureAsset>(font.texture_id());
    font_tex.bind(0);
    shader.set_int("font_texture", 0);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(rc.t_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rc.t_vbo);

    for (uint8_t c : text) {
        auto it = font.chars().find(c);
        if (it == font.chars().end()) {
            continue;
        }

        const CharacterGlyph& g = it->second;

        float xpos = x + g.bearing.x * scale;
        float ypos = y - (g.size.y - g.bearing.y) * scale;
        float w = g.size.x * scale;
        float h = g.size.y * scale;

        float vertices[6][4] = {
            // First triangle
            {xpos, ypos + h, g.uv_0.x, g.uv_1.y},  // bottom-left
            {xpos, ypos, g.uv_0.x, g.uv_0.y},      // top-left
            {xpos + w, ypos, g.uv_1.x, g.uv_0.y},  // top-right

            // Second triangle
            {xpos, ypos + h, g.uv_0.x, g.uv_1.y},     // bottom-left
            {xpos + w, ypos, g.uv_1.x, g.uv_0.y},     // top-right
            {xpos + w, ypos + h, g.uv_1.x, g.uv_1.y}  // bottom-right
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += g.advance * scale;  // move cursor
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    TextureAsset::unbind();
}

void RenderSystem::create_wire_cube(RenderContext& rc) {
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

    glGenVertexArrays(1, &rc.h_vao);
    glGenBuffers(1, &rc.h_vbo);
    glGenBuffers(1, &rc.h_ebo);

    glBindVertexArray(rc.h_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rc.h_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.h_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glBindVertexArray(0);
}

void RenderSystem::destroy_wire_cube(RenderContext& rc) {
    if (rc.h_ebo) {
        glDeleteBuffers(1, &rc.h_ebo);
        rc.h_ebo = 0;
    }
    if (rc.h_vbo) {
        glDeleteBuffers(1, &rc.h_vbo);
        rc.h_vbo = 0;
    }
    if (rc.h_vao) {
        glDeleteVertexArrays(1, &rc.h_vao);
        rc.h_vao = 0;
    }
}

void RenderSystem::create_arrow(EntityManager& em, RenderContext& rc) {
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

    glGenVertexArrays(1, &rc.a_vao);
    glGenBuffers(1, &rc.a_vbo);

    glBindVertexArray(rc.a_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rc.a_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glBindVertexArray(0);
}

void RenderSystem::destroy_arrow(RenderContext& rc) {
    if (rc.a_vbo) {
        glDeleteBuffers(1, &rc.a_vbo);
        rc.a_vbo = 0;
    }
    if (rc.a_vao) {
        glDeleteVertexArrays(1, &rc.a_vao);
        rc.a_vao = 0;
    }
}

void RenderSystem::create_text(RenderContext& rc) {
    glGenVertexArrays(1, &rc.t_vao);
    glGenBuffers(1, &rc.t_vbo);

    glBindVertexArray(rc.t_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rc.t_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(1);  // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

    glBindVertexArray(0);
}

void RenderSystem::destroy_text(RenderContext& rc) {
    if (rc.t_vbo) {
        glDeleteBuffers(1, &rc.t_vbo);
        rc.t_vbo = 0;
    }
    if (rc.t_vao) {
        glDeleteVertexArrays(1, &rc.t_vao);
        rc.t_vao = 0;
    }
}
