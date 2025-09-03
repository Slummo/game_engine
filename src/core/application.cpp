#include "core/application.h"
#include "core/log.h"
#include "managers/asset_manager.h"
#include "components/components.h"

#include <iostream>
#include <cmath>
#include <cstdint>

Application::Application() : m_running(true), m_cursor_enabled(true) {
}

bool Application::init() {
    // Create window
    if (!window.create(this, 800, 600, "Engine")) {
        ERR("[Application] Window creation failed!");
        return false;
    }

    // Create static instance
    AssetManager& am = AssetManager::instance();

    // Create materials
    AssetID block_mat_id = am.add_asset<Material>("block_mat", TextureType::Diffuse, "wood_box.jpg", "full");
    AssetID floor_mat_id = am.add_asset<Material>("floor_mat", TextureType::Diffuse, "floor2.jpg", "full");
    AssetID lightbulb_mat_id =
        am.add_asset<Material>("lightbulb_mat", TextureType::Diffuse, "lightbulb.jpeg", "lightbulb");

    // Create meshes with materials
    AssetID cube_pnt_mesh_id = MeshAsset::create_cube_PNT(block_mat_id);
    AssetID plane_mesh_id = MeshAsset::create_cube_PNT(floor_mat_id, 100.0f, 100.0f);
    AssetID cube_pt_mesh_id = MeshAsset::create_cube_PT(lightbulb_mat_id);

    // Create models with single meshes
    AssetID block_model_id = am.add_asset<ModelAsset>("block_model", cube_pnt_mesh_id);
    AssetID floor_model_id = am.add_asset<ModelAsset>("floor_model", plane_mesh_id);
    AssetID light_model_id = am.add_asset<ModelAsset>("light_model", cube_pt_mesh_id);

    // Load models
    AssetID spider_model_id = am.load_asset<ModelAsset>("obj/spider/spider.obj", "spider_model");
    AssetID backpack_model_id = am.load_asset<ModelAsset>("obj/backpack/backpack.obj", "backpack_model");
    AssetID player_model_id = am.load_asset<ModelAsset>("obj/player/player.obj", "player_model");

    // Load sounds
    AssetID jump_sound_id = am.load_asset<SoundAsset>("cartoon_jump.mp3");
    AssetID block_collision_sound_id = am.load_asset<SoundAsset>("block_collision.mp3");

    // Create entities
    EntityID cube_id = em.create_entity();
    EntityID plane_id = em.create_entity();
    EntityID player_id = em.create_entity();
    EntityID spider_id = em.create_entity();
    EntityID backpack_id = em.create_entity();
    EntityID main_light_id = em.create_entity();

    // Attach components

    // Block
    em.add_component<Transform>(cube_id);
    em.add_component<Model>(cube_id, block_model_id);
    em.add_component<RigidBody>(cube_id, 10.0f);
    em.add_component<Collider>(cube_id);
    auto& bl_ss = em.add_component<SoundSource>(cube_id);
    bl_ss.register_sound("Collision", block_collision_sound_id);

    // Floor
    auto& floor_tr = em.add_component<Transform>(plane_id, glm::vec3(0.0f, -0.25f, 0.0f));
    floor_tr.set_scale(glm::vec3(200.0f, 0.5f, 200.0f));
    em.add_component<Model>(plane_id, floor_model_id);
    em.add_component<RigidBody>(plane_id, 0.0f, true);
    auto& floor_col = em.add_component<Collider>(plane_id);
    floor_col.layer = Layers::Ground;

    // Spider
    em.add_component<Transform>(spider_id, glm::vec3(4.0f, 0.0f, 0.0f), glm::quat(1, 0, 0, 0), glm::vec3(0.005f));
    em.add_component<Model>(spider_id, spider_model_id);
    em.add_component<RigidBody>(spider_id, 0.5f);
    em.add_component<Collider>(spider_id);

    // Backpack
    em.add_component<Transform>(backpack_id, glm::vec3(2.0f, 0.0f, 2.0f), glm::quat(1, 0, 0, 0), glm::vec3(0.5f));
    em.add_component<Model>(backpack_id, backpack_model_id);
    em.add_component<RigidBody>(backpack_id, 3.0f);
    em.add_component<Collider>(backpack_id);
    auto& rot = em.add_component<Rotator>(backpack_id);
    rot.speed_deg = 60.0f;

    // Player
    auto& pl_tr = em.add_component<Transform>(player_id, glm::vec3(0.0f, 0.0f, 4.0f));
    pl_tr.set_scale(glm::vec3(1.0f, 2.0f, 1.0f));
    pl_tr.update_position(glm::vec3(0.0f, pl_tr.scale().y * 0.5f, 0.0f));  // feet on ground
    // em.add_component<Model>(player_id, player_model_id);
    em.add_component<RigidBody>(player_id, 60.0f);
    auto& pl_col = em.add_component<Collider>(player_id);
    pl_col.layer = Layers::Player;
    pl_col.collides_with = Layers::Ground;
    auto& main_camera = em.add_component<Camera>(player_id, glm::vec3(0.0f, 0.4f, 0.0f));
    main_camera.is_active = true;
    em.add_component<Player>(player_id, "main_player");
    em.add_component<FPController>(player_id);
    em.add_component<SoundListener>(player_id);
    auto& pl_ss = em.add_component<SoundSource>(player_id);
    pl_ss.register_sound("Jump", jump_sound_id);

    // Light
    em.add_component<Transform>(main_light_id, glm::vec3(0.0f, 10.0f, 30.0f),
                                glm::angleAxis(glm::radians(-30.0f), glm::vec3(1, 0, 0)), glm::vec3(3.0f));
    em.add_component<Model>(main_light_id, light_model_id);
    em.add_component<Light>(main_light_id);

    // Add contexts
    cm.add_context<EventContext>();
    cm.add_context<PhysicsContext>();
    cm.add_context<CollisionContext>();
    auto& ic = cm.add_context<InputContext>();
    cm.add_context<CameraContext>(main_camera);
    cm.add_context<RenderContext>();

    // Add systems
    sm.add_system<RigidBodySystem>();
    sm.add_system<CollisionDetectionSystem>();
    sm.add_system<CollisionResolutionSystem>();
    sm.add_system<FirstPersonControllerSystem>();
    sm.add_system<SoundSystem>();
    sm.add_system<LightSystem>();
    sm.add_system<CameraSystem>();
    sm.add_system<RotationSystem>();
    sm.add_system<RenderSystem>();
    sm.init_all(em, cm);

    // Define actions
    ic.register_action("Quit", InputType::Key, GLFW_KEY_ESCAPE);
    ic.on_action_pressed("Quit", [&]() { window.close(); });
    ic.register_action("ToggleCursor", InputType::Key, GLFW_KEY_C);
    ic.on_action_pressed("ToggleCursor", [&]() {
        m_cursor_enabled = !m_cursor_enabled;
        window.set_input_mode(GLFW_CURSOR, m_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        if (m_cursor_enabled) {
            // Reset delta and move cursor to center
            glm::ivec2 size = window.get_size();
            ic.set_mouse_delta(glm::dvec2{0.0});
            ic.set_mouse_pos(glm::dvec2{size.x / 2.0, size.y / 2.0});
        }
    });

    window.set_input_mode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    window.show();

    return true;
}

void Application::run() {
    double last = window.get_time();

    // FPS tracking
    int32_t frames = 0;
    double fps_time = 0.0;

    // Contexts
    auto& pc = cm.get_context<PhysicsContext>();
    auto& ic = cm.get_context<InputContext>();

    while (!window.should_close()) {
        double now = window.get_time();
        float dt = float(now - last);
        dt = std::clamp(dt, 0.0f, 0.25f);  // avoid spiral of death
        last = now;

        pc.dt = dt;

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

        window.poll_events();
        ic.begin_frame();
        sm.update_all(em, cm);
        ic.end_frame();
        window.swap_buffers();
    }
}

void Application::shutdown() {
    window.destroy();
    sm.shutdown_all(em, cm);
}

InputContext& Application::get_input_context() {
    return cm.get_context<InputContext>();
}