#include "systems/render_system.h"
#include "assets/shader_asset.h"
#include "assets/model_asset.h"
#include "components/transform.h"
#include "components/model.h"
#include "components/collider.h"
#include "components/light.h"
#include "contexts/render_context.h"
#include "contexts/camera_context.h"
#include "contexts/input_context.h"
#include "contexts/debug_context.h"
#include "core/engine.h"
#include "core/window.h"
#include "managers/context_manager.h"
#include "managers/entity_manager.h"
#include "managers/asset_manager.h"

#include <format>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

void RenderSystem::init(Engine& engine) {
    auto& rc = engine.cm().get<RenderContext>();
    auto& ic = engine.cm().get<InputContext>();
    auto& dc = engine.cm().get<DebugContext>();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    rc.create_scene_panel_fbo(400, 300);

    ic.register_action("ToggleWiremode", InputType::Key, GLFW_KEY_M, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleWiremode", [&]() { dc.wiremode = !dc.wiremode; });
    ic.register_action("ToggleDebug", InputType::Key, GLFW_KEY_F, GLFW_MOD_CONTROL);
    ic.on_action_pressed("ToggleDebug", [&]() { dc.active = !dc.active; });
}

void RenderSystem::update(Engine& engine) {
    auto& rc = engine.cm().get<RenderContext>();
    auto& cc = engine.cm().get<CameraContext>();
    auto& dc = engine.cm().get<DebugContext>();

    EntityManager& em = engine.em();
    AssetManager& am = engine.am();

    // Only get the main camera for now
    Camera& cam = cc.main_camera;

    // Bind the scene panel fbo, set its viewport and clear
    glBindFramebuffer(GL_FRAMEBUFFER, rc.scene_panel_fbo);
    glViewport(0, 0, rc.scene_panel_w, rc.scene_panel_h);
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, dc.wiremode ? GL_LINE : GL_FILL);

    // Render scene and debug on the scene panel fbo
    render_scene(em, am, cam, dc);
    if (dc.active) {
        render_debug(em, am, cam, dc);
    }

    // Do the same for the default fbo
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    auto win_size = rc.win.size();
    glViewport(0, 0, win_size.x, win_size.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Don't use wiremode for gui
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    render_gui(em, rc, dc);
}

void RenderSystem::render_scene(EntityManager& em, AssetManager& am, Camera& cam, DebugContext& dc) {
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
        if (em.has_component<Light>(e) && !dc.active) {
            // Render light models only if debug render is enabled
            continue;
        }

        // From frustum culling
        if (!m.visible) {
            continue;
        }

        ModelAsset& model = am.get<ModelAsset>(m.asset_id);
        model.draw(am, tr, cam, light);
    }
}

void RenderSystem::render_debug(EntityManager& em, AssetManager& am, Camera& cam, DebugContext& dc) {
    // Draw hitboxes
    ShaderAsset& line_shader = am.get<ShaderAsset>(dc.colored_line_shader_id);
    if (am.last_used_shader() != dc.colored_line_shader_id) {
        line_shader.use();
        am.set_last_used_shader(dc.colored_line_shader_id);
    }

    line_shader.set_matrix_4f("Projection", cam.proj_matrix());
    line_shader.set_matrix_4f("View", cam.view_matrix());

    glLineWidth(2.0f);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(dc.hitbox.vao);

    for (auto [_e, tr, col] : em.entities_with<Transform, Collider>()) {
        // Red = physical, green = trigger
        glm::vec3 color = col.is_trigger ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        line_shader.set_vec_3f("color", color);

        glm::quat rot = tr.rotation();
        glm::vec3 scale = tr.scale();
        glm::vec3 center = tr.position() + (rot * (col.offset * scale));
        glm::vec3 half_extents = (col.size * 0.5f) * scale;

        // Build model matrix
        glm::mat4 model(1.0f);
        model = glm::translate(model, center);
        model *= glm::mat4_cast(rot);
        model = glm::scale(model, half_extents * 2.0f);  // full size

        line_shader.set_matrix_4f("Model", model);

        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
    }

    glEnable(GL_DEPTH_TEST);

    // Draw directional light arrow
    line_shader.set_matrix_4f("Projection", cam.proj_matrix());
    line_shader.set_matrix_4f("View", cam.view_matrix());
    line_shader.set_matrix_4f("Model", glm::mat4(1.0f));           // identity, since positions are in world space
    line_shader.set_vec_3f("color", glm::vec3(1.0f, 1.0f, 0.0f));  // yellow

    glLineWidth(5.0f);
    glBindVertexArray(dc.arrow.vao);
    glDrawArrays(GL_LINES, 0, 2);
    glLineWidth(1.0f);
    glBindVertexArray(0);
}

void RenderSystem::render_gui(EntityManager& em, RenderContext& rc, DebugContext& dc) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Draw FPS counter
    ImVec2 fps_pos(10, 10);
    ImU32 color = IM_COL32(255, 255, 255, 255);
    const char* fps_text = std::format("FPS: {}", dc.fps).c_str();

    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->AddText(fps_pos, color, fps_text);

    ImGui::Begin("Transforms");
    ImGui::BeginChild("Scrolling");
    for (auto [e, tr] : em.entities_with<Transform>()) {
        const std::string& name = em.get_name(e);
        ImGui::Text(name.c_str());
        ImGui::SliderFloat3(("Position##" + std::to_string(e)).c_str(), glm::value_ptr(tr.position_mut()), -100.0f,
                            100.0f);
        ImGui::SliderFloat3(("Scale##" + std::to_string(e)).c_str(), glm::value_ptr(tr.scale_mut()), 0.1f, 50.0f,
                            "%.1f");
        ImGui::SameLine();
        ImGui::InputFloat3(("##ScaleInput" + std::to_string(e)).c_str(), glm::value_ptr(tr.scale_mut()), "%.1f");
    }
    ImGui::EndChild();
    ImGui::End();

    ImGui::Begin("Scene");

    // Get the scene panel dimensions
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // Rescale the scene panel fbo
    rc.rescale_scene_panel_fbo((uint32_t)avail.x, (uint32_t)avail.y);

    // Add the scene's fbo texture as an imgui image
    ImGui::Image((void*)(intptr_t)rc.texture_id, ImVec2(avail.x, avail.y), ImVec2(0, 1), ImVec2(1, 0));

    // bool clicked = ImGui::IsItemClicked();

    // if (clicked) {
    //     rc.win.set_capture(true);
    // }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}