#include "core/application.h"
#include "assets/shader_asset.h"
#include "assets/texture_asset.h"
#include "assets/material_asset.h"
#include "assets/mesh_asset.h"
#include "assets/model_asset.h"
#include "assets/sound_asset.h"
#include "components/transform.h"
#include "components/fp_controller.h"
#include "components/rigidbody.h"
#include "components/model.h"
#include "components/collider.h"
#include "components/light.h"
#include "components/camera.h"
#include "components/rotator.h"
#include "components/sound_source.h"
#include "components/sound_listener.h"
#include "components/player.h"
#include "contexts/event_context.h"
#include "contexts/physics_context.h"
#include "contexts/collision_context.h"
#include "contexts/input_context.h"
#include "contexts/camera_context.h"
#include "contexts/render_context.h"
#include "contexts/debug_context.h"
#include "systems/fp_controller_system.h"
#include "systems/rigidbody_system.h"
#include "systems/collision_detection_system.h"
#include "systems/collision_resolution_system.h"
#include "systems/light_system.h"
#include "systems/camera_system.h"
#include "systems/rotation_system.h"
#include "systems/render_system.h"
#include "systems/sound_system.h"
#include "core/window.h"
#include "core/engine.h"
#include "managers/entity_manager.h"
#include "managers/system_manager.h"
#include "managers/context_manager.h"
#include "managers/asset_manager.h"

#include <iostream>
#include <cmath>

#include <imgui/imgui.h>

Application::Application() = default;

bool Application::init() {
    window = std::make_unique<Window>();
    engine = std::make_unique<Engine>();

    // Create window
    if (!window->create("Engine", 800, 600)) {
        ERR("[Application] Window creation failed!");
        return false;
    }

    // Create models with single meshes

    // Block model
    AssetID block_mat_id = engine->am()
                               .create<MaterialAsset>("block_mat")
                               .add_dep(MaterialDepSlot::Shader, LoadDep(LoadableAssetType::Shader, "full", "full"))
                               .add_dep(MaterialDepSlot::Diffuse, LoadDep(LoadableAssetType::Texture, "wood_box_tex",
                                                                          "wood_box.jpg", true, TextureType::Diffuse))
                               .finish();

    AssetID cube_mesh_id = engine->am()
                               .create<MeshAsset>("cube_pnt_mesh")
                               .add_dep(MeshDepSlot::Material, CreateDep{block_mat_id})
                               .set_mesh_type(MeshType::CUBE_PNT)
                               .finish();

    AssetID block_model_id =
        engine->am().create<ModelAsset>("block_model").add_dep(ModelDepSlot::Mesh, CreateDep{cube_mesh_id}).finish();

    // Floor model
    AssetID floor_mat_id = engine->am()
                               .create<MaterialAsset>("floor_mat")
                               .add_dep(MaterialDepSlot::Shader, LoadDep(LoadableAssetType::Shader, "full", "full"))
                               .add_dep(MaterialDepSlot::Diffuse, LoadDep(LoadableAssetType::Texture, "floor2_tex",
                                                                          "floor2.jpg", true, TextureType::Diffuse))
                               .finish();

    AssetID floor_mesh_id = engine->am()
                                .create<MeshAsset>("plane_pnt_mesh")
                                .add_dep(MeshDepSlot::Material, CreateDep{floor_mat_id})
                                .set_mesh_type(MeshType::CUBE_PNT)
                                .set_uv_scale(glm::vec2(100.0f))
                                .finish();

    AssetID floor_model_id =
        engine->am().create<ModelAsset>("floor_model").add_dep(ModelDepSlot::Mesh, CreateDep{floor_mesh_id}).finish();

    // Light model
    AssetID light_mat_id =
        engine->am()
            .create<MaterialAsset>("lightbulb_mat")
            .add_dep(MaterialDepSlot::Shader, LoadDep(LoadableAssetType::Shader, "lightbulb", "lightbulb"))
            .add_dep(MaterialDepSlot::Diffuse,
                     LoadDep(LoadableAssetType::Texture, "lightbulb_tex", "lightbulb.jpeg", true, TextureType::Diffuse))
            .finish();

    AssetID light_mesh_id = engine->am()
                                .create<MeshAsset>("cube_pt_mesh")
                                .add_dep(MeshDepSlot::Material, CreateDep{light_mat_id})
                                .set_mesh_type(MeshType::CUBE_PT)
                                .finish();

    AssetID light_model_id =
        engine->am().create<ModelAsset>("light_model").add_dep(ModelDepSlot::Mesh, CreateDep{light_mesh_id}).finish();

    // Load models
    AssetID spider_model_id = engine->am().load<ModelAsset>("spider_model", "obj/spider/spider.obj").finish();
    AssetID backpack_model_id = engine->am().load<ModelAsset>("backpack_model", "obj/backpack/backpack.obj").finish();
    AssetID player_model_id = engine->am().load<ModelAsset>("player_model", "obj/player/player.obj").finish();

    // Load sounds
    AssetID jump_sound_id = engine->am().load<SoundAsset>("cartoon_jump", "cartoon_jump.mp3").finish();
    AssetID block_collision_sound_id = engine->am().load<SoundAsset>("block_collision", "block_collision.mp3").finish();

    // Create entities
    EntityID cube_id = engine->em().create_entity("box");
    EntityID plane_id = engine->em().create_entity("floor");
    EntityID player_id = engine->em().create_entity("player");
    EntityID spider_id = engine->em().create_entity("spider");
    EntityID backpack_id = engine->em().create_entity("backpack");
    EntityID main_light_id = engine->em().create_entity("main_light");

    // Attach components

    // Block
    engine->em().add<Transform>(cube_id);
    engine->em().add<Model>(cube_id, block_model_id);
    engine->em().add<RigidBody>(cube_id, 10.0f);
    engine->em().add<Collider>(cube_id);
    auto& bl_ss = engine->em().add<SoundSource>(cube_id);
    bl_ss.register_sound("Collision", block_collision_sound_id);

    // Floor
    auto& floor_tr = engine->em().add<Transform>(plane_id, glm::vec3(0.0f, -0.25f, 0.0f));
    floor_tr.set_scale(glm::vec3(200.0f, 0.5f, 200.0f));
    engine->em().add<Model>(plane_id, floor_model_id);
    engine->em().add<RigidBody>(plane_id, 0.0f, true);
    auto& floor_col = engine->em().add<Collider>(plane_id);
    floor_col.layer = Layers::Ground;

    // Spider
    engine->em().add<Transform>(spider_id, glm::vec3(4.0f, 0.0f, 0.0f), glm::quat(1, 0, 0, 0), glm::vec3(0.005f));
    engine->em().add<Model>(spider_id, spider_model_id);
    engine->em().add<RigidBody>(spider_id, 0.5f);
    engine->em().add<Collider>(spider_id);

    // Backpack
    engine->em().add<Transform>(backpack_id, glm::vec3(2.0f, 0.0f, 2.0f), glm::quat(1, 0, 0, 0), glm::vec3(0.5f));
    engine->em().add<Model>(backpack_id, backpack_model_id);
    engine->em().add<RigidBody>(backpack_id, 3.0f);
    engine->em().add<Collider>(backpack_id);
    auto& rot = engine->em().add<Rotator>(backpack_id);
    rot.speed_deg = 60.0f;

    // Player
    auto& pl_tr = engine->em().add<Transform>(player_id, glm::vec3(0.0f, 0.0f, 4.0f));
    pl_tr.set_scale(glm::vec3(1.0f, 2.0f, 1.0f));
    pl_tr.update_position(glm::vec3(0.0f, pl_tr.scale().y * 0.5f, 0.0f));  // feet on ground
    // engine->em().add<Model>(player_id, player_model_id);
    engine->em().add<RigidBody>(player_id, 60.0f);
    auto& pl_col = engine->em().add<Collider>(player_id);
    pl_col.layer = Layers::Player;
    pl_col.collides_with = Layers::Ground;
    auto& main_camera = engine->em().add<Camera>(player_id, glm::vec3(0.0f, 0.4f, 0.0f));
    main_camera.is_active = true;
    engine->em().add<Player>(player_id, "main_player");
    engine->em().add<FPController>(player_id);
    engine->em().add<SoundListener>(player_id);
    auto& pl_ss = engine->em().add<SoundSource>(player_id);
    pl_ss.register_sound("Jump", jump_sound_id);

    // Light
    engine->em().add<Transform>(main_light_id, glm::vec3(0.0f, 10.0f, 30.0f),
                                glm::angleAxis(glm::radians(-30.0f), glm::vec3(1, 0, 0)), glm::vec3(3.0f));
    engine->em().add<Model>(main_light_id, light_model_id);
    engine->em().add<Light>(main_light_id);

    // Add contexts
    engine->cm().add<EventContext>();
    engine->cm().add<PhysicsContext>();
    engine->cm().add<CollisionContext>();
    auto& ic = engine->cm().add<InputContext>();
    engine->cm().add<CameraContext>(main_camera);
    engine->cm().add<RenderContext>();
    engine->cm().add<DebugContext>(*engine, *window);

    // Add systems
    engine->sm().add<RigidBodySystem>();
    engine->sm().add<CollisionDetectionSystem>();
    engine->sm().add<CollisionResolutionSystem>();
    engine->sm().add<FirstPersonControllerSystem>();
    engine->sm().add<SoundSystem>();
    engine->sm().add<LightSystem>();
    engine->sm().add<CameraSystem>();
    engine->sm().add<RotationSystem>();
    engine->sm().add<RenderSystem>();
    engine->sm().init_all(*engine);

    // Link callbacks and define actions
    ic.link_callbacks(*window);

    ic.register_action("Quit", InputType::Key, GLFW_KEY_ESCAPE);
    ic.on_action_pressed("Quit", [&]() { window->close(); });

    window->set_capture(true);

    return true;
}

void Application::run() {
    window->show();

    double last = window->time();

    // FPS tracking
    int32_t frames = 0;
    double fps_time = 0.0;

    // Contexts
    auto& pc = engine->cm().get<PhysicsContext>();
    auto& ic = engine->cm().get<InputContext>();
    auto& dc = engine->cm().get<DebugContext>();

    while (!window->should_close()) {
        double now = window->time();
        float dt = float(now - last);
        dt = std::clamp(dt, 0.0f, 0.25f);  // avoid spiral of death
        last = now;

        pc.dt = dt;

        // FPS calculation
        frames++;
        fps_time += dt;
        if (fps_time >= 0.5) {                                       // every 0.5 second
            dc.fps = std::round(frames / fps_time * 10.0f) / 10.0f;  // round to first decimal digit
            frames = 0;
            fps_time = 0.0;
        }

        window->poll_events();
        ic.consume();
        engine->sm().update_all(*engine);
        window->swap_buffers();
    }
}

Application::~Application() {
    engine->sm().shutdown_all(*engine);
}