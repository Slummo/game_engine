#include "core/application.h"
#include "core/log.h"
#include "managers/asset_manager.h"
#include "components/components.h"

#include <iostream>
#include <cmath>

Application::Application() : m_running(true), m_cursor_enabled(true), m_wiremode(false) {
}

bool Application::init() {
    if (!m_window.create(this, 800, 600, "Engine")) {
        ERR("[Application] Window creation failed!");
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ERR("[Application] Failed to init GLAD");
        return false;
    }

    int fb_w, fb_h;
    m_window.get_framebuffer_size(&fb_w, &fb_h);
    m_window.set_viewport(fb_w, fb_h);

    // Create static instance
    AssetManager& am = AssetManager::instance();

    // Create materials
    AssetID block_mat_id = am.add_asset<Material>("block_mat", TextureType::Diffuse, "wood_box.jpg", "full");
    AssetID floor_mat_id = am.add_asset<Material>("floor_mat", TextureType::Diffuse, "floor2.jpg", "full");
    AssetID lightbulb_mat_id =
        am.add_asset<Material>("lightbulb_mat", TextureType::Diffuse, "lightbulb.jpeg", "lightbulb");

    // Create meshes with materials
    AssetID cube_pnt_mesh_id = Mesh::create_cube_PNT(block_mat_id);
    AssetID plane_mesh_id = Mesh::create_cube_PNT(floor_mat_id, 100.0f, 100.0f);
    AssetID cube_pt_mesh_id = Mesh::create_cube_PT(lightbulb_mat_id);

    // Create models with single meshes
    AssetID block_model_id = am.add_asset<Model>("block_model", cube_pnt_mesh_id);
    AssetID floor_model_id = am.add_asset<Model>("floor_model", plane_mesh_id);
    AssetID light_model_id = am.add_asset<Model>("light_model", cube_pt_mesh_id);

    // Load models
    AssetID spider_model_id = am.load_asset<Model>("obj/spider/spider.obj", "spider_model");
    AssetID backpack_model_id = am.load_asset<Model>("obj/backpack/backpack.obj", "backpack_model");
    AssetID player_model_id = am.load_asset<Model>("obj/player/player.obj", "player_model");

    EntityID cube_id = m_ecs.create_entity();
    EntityID plane_id = m_ecs.create_entity();
    EntityID player_id = m_ecs.create_entity();
    EntityID spider_id = m_ecs.create_entity();
    EntityID backpack_id = m_ecs.create_entity();
    EntityID main_light_id = m_ecs.create_entity();

    // Attach components

    // Block
    m_ecs.add_component<TransformComponent>(cube_id);
    m_ecs.add_component<ModelComponent>(cube_id, block_model_id);
    m_ecs.add_component<RigidBodyComponent>(cube_id, 10.0f);
    m_ecs.add_component<ColliderComponent>(cube_id);
    m_ecs.add_component<RotatorComponent>(cube_id);

    // Floor
    auto& floor_tr = m_ecs.add_component<TransformComponent>(plane_id, glm::vec3(0.0f, -0.25f, 0.0f));
    floor_tr.set_scale(glm::vec3(200.0f, 0.5f, 200.0f));
    m_ecs.add_component<ModelComponent>(plane_id, floor_model_id);
    m_ecs.add_component<RigidBodyComponent>(plane_id, 0.0f, true);
    m_ecs.add_component<ColliderComponent>(plane_id);

    // Player
    auto& pl_tr = m_ecs.add_component<TransformComponent>(player_id, glm::vec3(0.0f, 0.0f, 4.0f));
    pl_tr.set_scale(glm::vec3(1.0f, 2.0f, 1.0f));
    pl_tr.update_position(glm::vec3(0.0f, pl_tr.scale().y * 0.5f, 0.0f));  // feet on ground
    // m_ecs.add_component<ModelComponent>(player_id, player_model_id);
    m_ecs.add_component<RigidBodyComponent>(player_id, 60.0f);
    m_ecs.add_component<ColliderComponent>(player_id);
    m_ecs.add_component<CameraComponent>(player_id);
    m_ecs.add_component<PlayerComponent>(player_id, "main_player");
    m_ecs.add_component<FPControllerComponent>(player_id);

    // Spider
    m_ecs.add_component<TransformComponent>(spider_id, glm::vec3(4.0f, 0.0f, 0.0f), glm::quat(1, 0, 0, 0),
                                            glm::vec3(0.005f));
    m_ecs.add_component<ModelComponent>(spider_id, spider_model_id);
    m_ecs.add_component<RigidBodyComponent>(spider_id, 0.5f);
    m_ecs.add_component<ColliderComponent>(spider_id);

    // Backpack
    m_ecs.add_component<TransformComponent>(backpack_id, glm::vec3(2.0f, 0.0f, 2.0f), glm::quat(1, 0, 0, 0),
                                            glm::vec3(0.5f));
    m_ecs.add_component<ModelComponent>(backpack_id, backpack_model_id);
    m_ecs.add_component<RigidBodyComponent>(backpack_id, 3.0f);
    m_ecs.add_component<ColliderComponent>(backpack_id);

    // Light
    m_ecs.add_component<TransformComponent>(main_light_id, glm::vec3(0.0f, 10.0f, 30.0f),
                                            glm::angleAxis(glm::radians(-30.0f), glm::vec3(1, 0, 0)), glm::vec3(3.0f));
    m_ecs.add_component<ModelComponent>(main_light_id, light_model_id);
    m_ecs.add_component<LightComponent>(main_light_id);

    // Add Systems
    m_sm.add_system<FirstPersonControllerSystem>(m_im);
    m_sm.add_system<RigidBodySystem>();
    m_sm.add_system<CollisionSystem>();
    m_sm.add_system<LightSystem>();
    m_sm.add_system<RotationSystem>();
    m_sm.add_system<RenderSystem>();
    m_sm.init_all(m_ecs);

    // Define actions
    m_im.register_action("Quit", InputType::Key, GLFW_KEY_ESCAPE);
    m_im.on_action_pressed("Quit", [this]() { m_window.close(); });
    m_im.register_action("ToggleCursor", InputType::Key, GLFW_KEY_C);
    m_im.on_action_pressed("ToggleCursor", [this]() {
        m_cursor_enabled = !m_cursor_enabled;
        m_window.set_input_mode(GLFW_CURSOR, m_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        if (m_cursor_enabled) {
            // Reset delta and move cursor to center
            glm::ivec2 size = m_window.get_size();
            m_im.set_mouse_delta(glm::dvec2{0.0});
            m_im.set_mouse_pos(glm::dvec2{size.x / 2.0, size.y / 2.0});
        }
    });
    m_im.register_action("ToggleWiremode", InputType::Key, GLFW_KEY_M, GLFW_MOD_CONTROL);
    m_im.on_action_pressed("ToggleWiremode", [this]() {
        m_wiremode = !m_wiremode;
        m_window.set_wiremode(m_wiremode);
    });
    m_im.register_action("ToggleHitboxes", InputType::Key, GLFW_KEY_H, GLFW_MOD_CONTROL);
    m_im.on_action_pressed("ToggleHitboxes",
                           [this]() { m_sm.get_system<RenderSystem>().toggle_hitbox_render_enabled(); });
    m_im.register_action("Jump", InputType::Key, GLFW_KEY_SPACE);
    m_im.on_action_pressed("Jump", [this]() {
        for (auto [_e, _pl, fpc] : m_ecs.entities_with<PlayerComponent, FPControllerComponent>()) {
            fpc.should_jump = true;
        }
    });
    m_im.register_action("ToggleDebug", InputType::Key, GLFW_KEY_F, GLFW_MOD_CONTROL);
    m_im.on_action_pressed("ToggleDebug", [this]() { m_sm.get_system<RenderSystem>().toggle_debug_render_enabled(); });

    m_window.set_input_mode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    m_window.show();

    return true;
}

void Application::run() {
    double last = m_window.get_time();
    double accumulator = 0.0;
    const double fixed_dt = 1.0 / 60.0;  // 60Hz

    // FPS tracking
    int frames = 0;
    double fps_time = 0.0;

    while (!m_window.should_close()) {
        double now = m_window.get_time();
        float dt = float(now - last);
        dt = std::clamp(dt, 0.0f, 0.25f);  // avoid spiral of death
        last = now;

        accumulator += dt;

        // FPS calculation
        frames++;
        fps_time += dt;
        if (fps_time >= 0.5) {  // every 0.5 second
            float fps = frames / fps_time;
            fps = std::round(fps * 10.0f) / 10.0f;  // round to first decimal digit
            LOG("FPS: " << fps);
            frames = 0;
            fps_time = 0.0;
        }

        m_window.poll_events();

        m_im.begin_frame(now);

        // Update systems
        while (accumulator >= fixed_dt) {
            m_sm.fixed_update_all(m_ecs, fixed_dt);
            accumulator -= fixed_dt;
        }
        m_sm.update_all(m_ecs, dt);

        m_im.end_frame();

        m_window.swap_buffers();
        m_window.poll_events();
    }
}

void Application::shutdown() {
    m_window.destroy();
    m_sm.shutdown_all(m_ecs);
}

InputManager& Application::get_input_manager() {
    return m_im;
}