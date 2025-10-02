#include "systems/fp_controller_system.h"
#include "components/transform.h"
#include "components/rigidbody.h"
#include "components/camera.h"
#include "components/fp_controller.h"
#include "contexts/input_context.h"
#include "contexts/event_context.h"
#include "core/engine.h"
#include "managers/context_manager.h"
#include "managers/entity_manager.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui/imgui.h>

void FirstPersonControllerSystem::init(Engine& engine) {
    auto& ic = engine.cm().get<InputContext>();

    ic.register_action("MoveLeft", InputType::Key, GLFW_KEY_A);
    ic.register_action("MoveRight", InputType::Key, GLFW_KEY_D);
    ic.register_action("MoveForwards", InputType::Key, GLFW_KEY_W);
    ic.register_action("MoveBackwards", InputType::Key, GLFW_KEY_S);
    ic.register_action("Jump", InputType::Key, GLFW_KEY_SPACE);
}

void FirstPersonControllerSystem::update(Engine& engine) {
    auto& ic = engine.cm().get<InputContext>();
    auto& ec = engine.cm().get<EventContext>();

    EntityManager& em = engine.em();

    auto io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
        return;
    }

    for (auto [e, tr, rb, cam, fpc] : em.entities_with<Transform, RigidBody, Camera, FPController>()) {
        // Mouse look
        glm::dvec2 cursor_pos_delta = ic.cursor_pos_delta();
        cam.update_yaw(cursor_pos_delta.x * fpc.look_speed);     // + for right
        cam.update_pitch(-cursor_pos_delta.y * fpc.look_speed);  // - for down

        // Apply yaw only to the transform
        glm::quat q_yaw = glm::angleAxis(glm::radians(cam.yaw()), glm::vec3(0.0f, 1.0f, 0.0f));
        tr.set_rotation(q_yaw);

        glm::vec3 forward = cam.front();
        forward.y = 0.0f;
        forward = glm::normalize(forward);

        // Right vector stays horizontal too
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        glm::vec3 move_dir(0.0f);
        if (ic.is_action_down("MoveLeft")) {
            move_dir -= right;
        }
        if (ic.is_action_down("MoveRight")) {
            move_dir += right;
        }
        if (ic.is_action_down("MoveForwards")) {
            move_dir += forward;
        }
        if (ic.is_action_down("MoveBackwards")) {
            move_dir -= forward;
        }

        // Normalize to avoid faster diagonal movement
        move_dir = glm::normalize(move_dir);
        ec.emit(MoveEvent{e, std::move(move_dir)});

        if (ic.was_action_pressed("Jump") && fpc.is_grounded) {
            ec.emit(JumpEvent{e});
        }
    }

    ec.dispatch();
}