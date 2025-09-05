#include "systems/render_system.h"
#include "managers/asset_manager.h"

#include <format>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

void RenderSystem::init(EntityManager& /*em*/, RenderContext& rc, CameraContext& /*cc*/, InputContext& ic) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    ic.register_action("ToggleWiremode", InputType::Key, GLFW_KEY_M, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleWiremode", [&]() { rc.wiremode = !rc.wiremode; });
    ic.register_action("ToggleHitboxes", InputType::Key, GLFW_KEY_H, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleHitboxes", [&]() { rc.hitbox_render_enabled = !rc.hitbox_render_enabled; });
    ic.register_action("ToggleDebug", InputType::Key, GLFW_KEY_F, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleDebug", [&]() { rc.debug_render_enabled = !rc.debug_render_enabled; });
}

void RenderSystem::update(EntityManager& em, RenderContext& rc, CameraContext& cc, InputContext& /*ic*/) {
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Don't use wiremode for text
    glPolygonMode(GL_FRONT_AND_BACK, rc.wiremode ? GL_LINE : GL_FILL);

    // Only get the main camera for now
    Camera& cam = cc.main_camera;
    render_scene(em, cam, rc);
    render_hitboxes(em, cam, rc);
    render_debug(em, cam, rc);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    std::string fps_string(std::format("FPS: {}", rc.fps));
    render_text(rc, fps_string, 10.0f, 560.0f, 0.5f, glm::vec3(1.0f));
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
        model.draw(tr, cam, light);
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

    glBindVertexArray(rc.hitbox.vao);

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
    glBindVertexArray(rc.arrow.vao);
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

    glBindVertexArray(rc.text.vao);
    glBindBuffer(GL_ARRAY_BUFFER, rc.text.vbo);

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

        float vertices[4][4] = {
            {xpos, ypos + h, g.uv_0.x, g.uv_1.y},     // bottom-left
            {xpos, ypos, g.uv_0.x, g.uv_0.y},         // top-left
            {xpos + w, ypos, g.uv_1.x, g.uv_0.y},     // top-right
            {xpos + w, ypos + h, g.uv_1.x, g.uv_1.y}  // bottom-right
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        x += g.advance * scale;  // move cursor
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    TextureAsset::unbind();
}