#include "systems/render_system.h"
#include "managers/asset_manager.h"

#include <format>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

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

    glPolygonMode(GL_FRONT_AND_BACK, rc.wiremode ? GL_LINE : GL_FILL);

    // Only get the main camera for now
    Camera& cam = cc.main_camera;
    render_scene(em, cam, rc);
    render_hitboxes(em, cam, rc);
    render_debug(em, cam, rc);

    // Don't use wiremode for gui
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    render_gui(em, rc);
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

void RenderSystem::render_debug(EntityManager& em, Camera& cam, RenderContext& rc) {
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

    // Render debug GUI
    ImGui::GetIO().MouseDrawCursor = true;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Draw FPS counter
    ImVec2 pos(10, 10);
    ImU32 color = IM_COL32(255, 255, 255, 255);
    std::string fps_text = std::format("FPS: {}", rc.fps);
    ImGui::GetForegroundDrawList()->AddText(pos, color, fps_text.c_str());

    ImGui::Begin("Transforms");
    ImGui::BeginChild("Scrolling");
    for (auto [e, tr] : em.entities_with<Transform>()) {
        ImGui::Text("Entity %d", e);
        ImGui::SliderFloat3(("Position##" + std::to_string(e)).c_str(), glm::value_ptr(tr.position_mut()), 0.0f,
                            100.0f);
        ImGui::SliderFloat3(("Scale##" + std::to_string(e)).c_str(), glm::value_ptr(tr.scale_mut()), 0.0f, 100.0f);
    }
    ImGui::EndChild();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RenderSystem::render_gui(EntityManager& em, RenderContext& rc) {
    // TODO
}