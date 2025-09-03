#include "systems/camera_system.h"

#include <glm/glm.hpp>
#include <cstdint>

void transform_aabb(const glm::vec3& in_min, const glm::vec3& in_max, const glm::mat4& model, glm::vec3& out_min,
                    glm::vec3& out_max) {
    glm::vec3 corners[8] = {
        {in_min.x, in_min.y, in_min.z}, {in_max.x, in_min.y, in_min.z}, {in_min.x, in_max.y, in_min.z},
        {in_max.x, in_max.y, in_min.z}, {in_min.x, in_min.y, in_max.z}, {in_max.x, in_min.y, in_max.z},
        {in_min.x, in_max.y, in_max.z}, {in_max.x, in_max.y, in_max.z},
    };

    out_min = glm::vec3(std::numeric_limits<float>::infinity());
    out_max = glm::vec3(-std::numeric_limits<float>::infinity());

    for (int32_t i = 0; i < 8; i++) {
        glm::vec4 local_corner(corners[i], 1.0f);
        glm::vec3 corner(model * local_corner);
        out_min = glm::min(out_min, corner);
        out_max = glm::max(out_max, corner);
    }
}

void CameraSystem::update(EntityManager& em, CameraContext& cc) {
    if (!cc.main_camera.is_active) {
        return;
    }

    // Compute visible flags for frustum culling
    for (auto [_e, tr, m] : em.entities_with<Transform, Model>()) {
        const AABB& aabb = m.local_aabb;
        glm::vec3 world_min;
        glm::vec3 world_max;
        transform_aabb(aabb.min, aabb.max, tr.model_matrix(), world_min, world_max);

        m.visible = cc.main_camera.frustum().is_AABB_visible(world_min, world_max);
    }

    // Compute cameras world position
    for (auto [_e, tr, cam] : em.entities_with<Transform, Camera>()) {
        cam.set_world_position(tr.position() + tr.rotation() * cam.offset * tr.scale());
    }
}