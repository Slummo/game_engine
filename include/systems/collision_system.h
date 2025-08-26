#pragma once

#include "systems/isystem.h"
#include "core/types.h"

#include <vector>
#include <glm/glm.hpp>

class CollisionSystem : public ISystem {
public:
    CollisionSystem() = default;

    void init(ECS& ecs) override;
    void update(ECS& ecs, float /*dt*/) override;

private:
    struct Contact {
        EntityID a = 0;
        EntityID b = 0;
        float penetration = 0.0f;
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.0f};  // points from A to B
        bool is_trigger = false;
    };
    struct OBB {
        glm::vec3 center;
        glm::vec3 half_extents;  // half extents in world units
        glm::vec3 axes[3];       // unit axes in world space
    };
    struct Sphere {
        glm::vec3 center;
        float radius;
    };
    struct Capsule {
        glm::vec3 p0;  // bottom center point
        glm::vec3 p1;  // top center point
        float radius;  // radius of the capsule
    };
    struct CollisionEntry {
        CollisionEntry(ECS& ecs, EntityID _id) {
            id = _id;
            collider_comp = ecs.get_component<ColliderComponent>(_id);
            transform_comp = ecs.get_component<TransformComponent>(_id);
            is_sphere = collider_comp.type == HitboxType::Sphere;

            if (is_sphere) {
                sphere = compute_world_sphere(transform_comp, collider_comp);
                collider_aabb = compute_world_aabb_from_sphere(sphere);
            } else {
                obb = compute_world_OBB(transform_comp, collider_comp);
                collider_aabb = compute_world_aabb_from_obb(obb);
            }
        }

        EntityID id;
        ColliderComponent collider_comp;
        TransformComponent transform_comp;
        bool is_sphere;
        Sphere sphere;
        OBB obb;
        AABB collider_aabb;
    };

    // Compute world shapes
    static OBB compute_world_OBB(TransformComponent& tr, ColliderComponent& c);
    static Sphere compute_world_sphere(TransformComponent& tr, ColliderComponent& c);
    static Capsule compute_world_capsule(TransformComponent& tr, ColliderComponent& c);
    static AABB compute_world_aabb_from_obb(const OBB& obb);
    static AABB compute_world_aabb_from_sphere(const Sphere& s);

    // Broadphase
    static bool aabb_overlap(const AABB& A, const AABB& B);

    // Narrowphase tests
    static bool sphere_vs_sphere(EntityID a, const Sphere& A, EntityID b, const Sphere& B, Contact& out);
    static bool sphere_vs_obb(EntityID a, const Sphere& s, EntityID b, const OBB& obb, Contact& out);
    static bool obb_vs_obb(EntityID a, const OBB& A, EntityID b, const OBB& B, Contact& out);

    // TODO Implement capsule combinations

    // Resolve contacts
    void resolve_contacts(ECS& ecs, std::vector<Contact> contacts, int solver_iterations = 2);
    void resolve_phys_contact(ECS& ecs, const Contact& c);
    void positional_correction(ECS& ecs, const Contact& c);
    void resolve_trigger_contact(ECS& ecs, const Contact& c);
};
