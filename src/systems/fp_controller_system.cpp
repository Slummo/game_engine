#include "systems/fp_controller_system.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <GLFW/glfw3.h>
#include <iostream>

#define FP_EPS 1e-6f
#define JMSRF 0.65f  // Jumping movement speed reduction factor

void FirstPersonControllerSystem::init(EntityManager& em, PhysicsContext& /*pc*/, InputContext& ic) {
    ic.register_action("MoveLeft", InputType::Key, GLFW_KEY_A);
    ic.register_action("MoveRight", InputType::Key, GLFW_KEY_D);
    ic.register_action("MoveForwards", InputType::Key, GLFW_KEY_W);
    ic.register_action("MoveBackwards", InputType::Key, GLFW_KEY_S);
    ic.register_action("Jump", InputType::Key, GLFW_KEY_SPACE);
    ic.on_action_pressed("Jump", [this, &em]() {
        for (auto [e, fpc] : em.entities_with<FPController>()) {
            fpc.should_jump = true;
        }
    });
}

void FirstPersonControllerSystem::update(EntityManager& em, PhysicsContext& pc, InputContext& ic) {
    for (auto [e, tr, rb, col, cam, fpc] : em.entities_with<Transform, RigidBody, Collider, Camera, FPController>()) {
        // Mouse look
        glm::vec2 mouse_delta = ic.mouse_delta();
        cam.update_yaw(mouse_delta.x * fpc.look_speed);     // + for right
        cam.update_pitch(-mouse_delta.y * fpc.look_speed);  // - for down

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

        // Current horizontal velocity and speed
        glm::vec3 horiz_vel = rb.velocity;
        horiz_vel.y = 0.0f;
        float speed = glm::length(horiz_vel);

        float ground_brake_accel = 20.0f;   // m/s^2 large to stop quickly
        float stop_speed_threshold = 0.2f;  // below this, snap to zero

        // Movement
        bool is_moving = glm::dot(move_dir, move_dir) > FP_EPS;
        if (is_moving) {
            glm::vec3 desired_vel_h = move_dir * fpc.move_speed;

            if (!fpc.is_grounded) {
                // Allow a reduced movement when in air
                desired_vel_h *= JMSRF;
            }

            glm::vec3 dv = desired_vel_h - horiz_vel;
            rb.apply_impulse(dv * rb.mass);
        } else {
            // No movement input so brake
            float max_dv = ground_brake_accel * pc.dt;

            if (speed <= stop_speed_threshold) {
                if (speed > 0.0f) {
                    rb.apply_impulse(-horiz_vel * rb.mass);
                }
            } else {
                // reduce speed by up to max_dv
                glm::vec3 dv = -horiz_vel * (max_dv / speed);
                rb.apply_impulse(dv * rb.mass);
            }
        }

        // Jump
        if (fpc.should_jump && fpc.is_grounded) {
            // Desired upward velocity
            float desired_vy = fpc.jump_speed;
            float current_vy = rb.velocity.y;

            // Compute needed delta-v to reach desired jump speed
            float dv_y = desired_vy - current_vy;

            // Only apply positive dv (don't remove upward velocity)
            if (dv_y > 0.0f) {
                glm::vec3 jump_impulse(0.0f, dv_y * rb.mass, 0.0f);
                rb.apply_impulse(jump_impulse);
            }

            // Play jump sound
            if (em.has_component<SoundSource>(e)) {
                auto& ss = em.get_component<SoundSource>(e);
                ss.set_sound("Jump");
                ss.should_play = true;
            }

            fpc.should_jump = false;
            fpc.is_grounded = false;
        }
    }
}