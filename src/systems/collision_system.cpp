#include "systems/collision_system.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <limits>

#define COL_EPS 1e-6f
#define COR_PER 0.1f  // positional correction percentage
#define SLOP 0.01f    // penetration allowance

void CollisionSystem::init(ECS& ecs) {
    for (auto [e, col, m] : ecs.entities_with<ColliderComponent, ModelComponent>()) {
        // Update colliders with model's local aabb
        const AABB& local_aabb = m.local_aabb;
        col.size = local_aabb.max - local_aabb.min;
        col.offset = (local_aabb.min + local_aabb.max) * 0.5f;
    }
}

void CollisionSystem::update(ECS& ecs, float /*dt*/) {
    std::vector<Contact> contacts;
    std::vector<CollisionEntry> entries;
    for (auto [e, tr, col] : ecs.entities_with<TransformComponent, ColliderComponent>()) {
        if (!col.is_enabled) {
            continue;
        }

        // TODO: filter by layer or mask
        entries.push_back(CollisionEntry(ecs, e));
    }

    // Broadphase + narrowphase
    for (size_t i = 0; i < entries.size(); ++i) {
        for (size_t j = i + 1; j < entries.size(); ++j) {
            CollisionEntry& A = entries[i];
            CollisionEntry& B = entries[j];

            // Layer / mask filtering
            if ((A.collider_comp.collides_with & A.collider_comp.layer) == 0) {
                // TODO: user masking logic
            }

            if (!aabb_overlap(A.collider_aabb, B.collider_aabb)) {
                continue;
            }

            // Gather contacts
            Contact c;
            bool hit = false;
            if (A.is_sphere && B.is_sphere) {
                hit = sphere_vs_sphere(A.id, A.sphere, B.id, B.sphere, c);
            } else if (A.is_sphere && !B.is_sphere) {
                hit = sphere_vs_obb(A.id, A.sphere, B.id, B.obb, c);
            } else if (!A.is_sphere && B.is_sphere) {
                hit = sphere_vs_obb(B.id, B.sphere, A.id, A.obb, c);
                if (hit) {
                    // Flip so A->B ordering is consistent
                    std::swap(c.a, c.b);
                    c.normal = -c.normal;
                }
            } else {
                hit = obb_vs_obb(A.id, A.obb, B.id, B.obb, c);
            }

            if (hit) {
                c.is_trigger = (A.collider_comp.is_trigger || B.collider_comp.is_trigger);
                contacts.push_back(c);
            }
        }
    }

    resolve_contacts(ecs, std::move(contacts));
}

CollisionSystem::OBB CollisionSystem::compute_world_OBB(TransformComponent& tr, ColliderComponent& c) {
    const glm::mat4& M = tr.model_matrix();

    glm::vec4 local_center(c.offset, 1.0f);
    glm::vec3 center(M * local_center);

    glm::vec3 x(M[0]);
    glm::vec3 y(M[1]);
    glm::vec3 z(M[2]);

    glm::vec3 half_extents_local = c.size * 0.5f;

    OBB obb;
    obb.center = center;
    obb.axes[0] = glm::normalize(x);
    obb.axes[1] = glm::normalize(y);
    obb.axes[2] = glm::normalize(z);

    obb.half_extents.x = glm::length(x) * half_extents_local.x;
    obb.half_extents.y = glm::length(y) * half_extents_local.y;
    obb.half_extents.z = glm::length(z) * half_extents_local.z;

    return obb;
}

CollisionSystem::Sphere CollisionSystem::compute_world_sphere(TransformComponent& tr, ColliderComponent& c) {
    const glm::mat4& M = tr.model_matrix();

    glm::vec4 local_center(c.offset, 1.0f);
    glm::vec3 center(M * local_center);

    float sx = glm::length(glm::vec3(M[0]));
    float sy = glm::length(glm::vec3(M[1]));
    float sz = glm::length(glm::vec3(M[2]));
    float max_scale = glm::max(sx, glm::max(sy, sz));

    float radius = c.size.x * max_scale;

    return {center, radius};
}

CollisionSystem::Capsule CollisionSystem::compute_world_capsule(TransformComponent& tr, ColliderComponent& c) {
    const glm::mat4& M = tr.model_matrix();

    glm::vec4 local_center(c.offset, 1.0f);
    glm::vec3 center(M * local_center);

    glm::vec3 up = glm::normalize(glm::vec3(M[1]));  // Y axis in world space
    float sx = glm::length(glm::vec3(M[0]));
    float sy = glm::length(glm::vec3(M[1]));

    float hh = (c.size.y * 0.5f) * sy;  // half height
    float radius = c.size.x * sx;

    return {center - up * hh, center + up * hh, radius};
}

AABB CollisionSystem::compute_world_aabb_from_obb(const OBB& obb) {
    glm::vec3 e0 = glm::abs(obb.axes[0]) * obb.half_extents.x;
    glm::vec3 e1 = glm::abs(obb.axes[1]) * obb.half_extents.y;
    glm::vec3 e2 = glm::abs(obb.axes[2]) * obb.half_extents.z;

    glm::vec3 extents = e0 + e1 + e2;

    return AABB{.min = obb.center - extents, .max = obb.center + extents};
}

AABB CollisionSystem::compute_world_aabb_from_sphere(const Sphere& s) {
    glm::vec3 radius_vec(s.radius);
    return AABB{.min = s.center - radius_vec, .max = s.center + radius_vec};
}

bool CollisionSystem::aabb_overlap(const AABB& A, const AABB& B) {
    return (A.min.x <= B.max.x && A.max.x >= B.min.x) && (A.min.y <= B.max.y && A.max.y >= B.min.y) &&
           (A.min.z <= B.max.z && A.max.z >= B.min.z);
}

bool CollisionSystem::sphere_vs_sphere(EntityID a, const Sphere& A, EntityID b, const Sphere& B, Contact& out) {
    glm::vec3 d = B.center - A.center;
    float dist2 = dot(d, d);
    float rsum = A.radius + B.radius;
    if (dist2 >= rsum * rsum) {
        return false;
    }
    // Use sqrt over dist2 since glm::lenght does the same
    float dist = sqrt(std::max(dist2, COL_EPS));

    out.a = a;
    out.b = b;
    out.normal = (dist > COL_EPS) ? (d / dist) : glm::vec3(1, 0, 0);
    out.penetration = rsum - dist;
    out.position = A.center + out.normal * (A.radius - out.penetration * 0.5f);
    out.is_trigger = false;
    return true;
}

bool CollisionSystem::sphere_vs_obb(EntityID a, const Sphere& s, EntityID b, const OBB& obb, Contact& out) {
    // Find closest point on OBB to sphere center
    // Transform sphere center into OBB local space: local = rot_mat^t * (point - center)
    glm::mat3 rot_mat(obb.axes[0], obb.axes[1], obb.axes[2]);
    glm::vec3 local = transpose(rot_mat) * (s.center - obb.center);
    glm::vec3 clamped = clamp(local, -obb.half_extents, obb.half_extents);
    glm::vec3 closest = rot_mat * clamped + obb.center;
    glm::vec3 d = s.center - closest;
    float dist2 = dot(d, d);
    if (dist2 > s.radius * s.radius) {
        return false;
    }

    float dist = sqrt(std::max(dist2, COL_EPS));

    out.a = a;
    out.b = b;
    out.normal = (dist > COL_EPS) ? (d / dist) : glm::vec3(1, 0, 0);
    out.penetration = s.radius - dist;
    out.position = closest;
    out.is_trigger = false;
    return true;
}

bool CollisionSystem::obb_vs_obb(EntityID a, const OBB& A, EntityID b, const OBB& B, Contact& out) {
    // Compute rotation matrix expressing B in A's local frame: rot_mat = rot_mat_A^t * rot_mat_B
    glm::mat3 rot_mat_A(A.axes[0], A.axes[1], A.axes[2]);
    glm::mat3 rot_mat_B(B.axes[0], B.axes[1], B.axes[2]);

    glm::mat3 rot_mat = glm::transpose(rot_mat_A) * rot_mat_B;

    // Translation from A to B in world space, then express in A's frame
    glm::vec3 t_world = B.center - A.center;
    glm::vec3 t = transpose(rot_mat_A) * t_world;  // t in A's local coords

    // Compute common subexpressions abs(rot_mat) + COL_EPS
    glm::mat3 abs_R;
    for (int32_t i = 0; i < 3; ++i) {
        for (int32_t j = 0; j < 3; ++j) {
            abs_R[i][j] = std::abs(rot_mat[i][j]) + COL_EPS;
        }
    }

    // half extents
    glm::vec3 a_he = A.half_extents;
    glm::vec3 b_he = B.half_extents;

    // Test the 15 axes and find the minimal penetration axis
    float min_overlap = std::numeric_limits<float>::infinity();
    glm::vec3 min_axis(1, 0, 0);
    bool separated = false;

    auto text_axis_fn = [&](const glm::vec3& axis_world, float proj_a, float proj_b, float dist_along_axis) -> bool {
        float overlap = proj_a + proj_b - std::abs(dist_along_axis);
        if (overlap < 0.0f) {
            separated = true;
            return false;
        }

        if (overlap < min_overlap) {
            min_overlap = overlap;
            min_axis = axis_world;
        }

        return true;
    };

    // Test axes L0, L1, L2 (A's local axes)
    // For A frame i: proj_a = a_he[i]
    // For B frame onto A axis: proj_b = sum_j (b_he[j] * abs_R[i][j])
    for (int32_t i = 0; i < 3 && !separated; ++i) {
        float proj_a = a_he[i];
        float proj_b = b_he[0] * abs_R[i][0] + b_he[1] * abs_R[i][1] + b_he[2] * abs_R[i][2];
        float dist = t[i];  // projection of t onto A's axis i (since t is in A frame)
        glm::vec3 axis_world = rot_mat_A * glm::vec3((i == 0), (i == 1), (i == 2));  // i-th world axis of A
        if (!text_axis_fn(axis_world, proj_a, proj_b, dist)) {
            return false;
        }
    }

    // Test axes M0, M1, M2 (B's local axes)
    // For A frame onto B axis: proj_a = sum_i (a_he[i] * abs_R[i][j])
    // For B frame j = proj_b = b_he[j]
    for (int32_t j = 0; j < 3 && !separated; ++j) {
        float proj_a = a_he[0] * abs_R[0][j] + a_he[1] * abs_R[1][j] + a_he[2] * abs_R[2][j];
        float proj_b = b_he[j];
        // distance = dot(t, rot_mat[:,j]) -> since t is A-frame coords, dot with column j of rot_mat
        float dist = t[0] * rot_mat[0][j] + t[1] * rot_mat[1][j] + t[2] * rot_mat[2][j];
        glm::vec3 axis_world = rot_mat_B * glm::vec3((j == 0), (j == 1), (j == 2));  // j-th world axis of B
        if (!text_axis_fn(axis_world, proj_a, proj_b, dist)) {
            return false;
        }
    }

    // Test cross product axes (9 tests)
    for (int32_t i = 0; i < 3 && !separated; ++i) {
        for (int32_t j = 0; j < 3 && !separated; ++j) {
            // axis = A_i x B_j (expressed in world)
            glm::vec3 Ai = rot_mat_A * glm::vec3((i == 0), (i == 1), (i == 2));
            glm::vec3 Bj = rot_mat_B * glm::vec3((j == 0), (j == 1), (j == 2));
            glm::vec3 axis_world = cross(Ai, Bj);
            float axis_len2 = dot(axis_world, axis_world);
            if (axis_len2 < COL_EPS) {
                continue;
            }

            float proj_a = 0.0f;
            float proj_b = 0.0f;

            // Projection calculation for A
            for (int32_t k = 0; k < 3; k++) {
                glm::vec3 Ak = rot_mat_A * glm::vec3((k == 0), (k == 1), (k == 2));
                float proj = dot(Ak, normalize(axis_world));
                proj_a += a_he[k] * std::abs(proj);
            }

            // Projection calculation for B
            for (int32_t k = 0; k < 3; k++) {
                glm::vec3 Bk = rot_mat_B * glm::vec3((k == 0), (k == 1), (k == 2));
                float proj = dot(Bk, normalize(axis_world));
                proj_b += b_he[k] * std::abs(proj);
            }

            float dist = dot(t_world, normalize(axis_world));
            if (!text_axis_fn(normalize(axis_world), proj_a, proj_b, dist)) {
                return false;
            }
        }
    }

    if (separated) {
        return false;
    }

    // Use min_axis (in world space) as approximate contact normal
    glm::vec3 center_dir = B.center - A.center;
    if (dot(center_dir, min_axis) < 0.0f) {
        min_axis = -min_axis;
    }

    out.a = a;
    out.b = b;
    out.normal = normalize(min_axis);
    out.penetration = min_overlap;
    // approximate contact position as midway between centers shifted by half penetration along normal
    out.position = (A.center + B.center) * 0.5f - out.normal * (out.penetration * 0.5f);
    out.is_trigger = false;
    return true;
}

void CollisionSystem::resolve_contacts(ECS& ecs, std::vector<Contact> contacts, int32_t solver_iterations) {
    for (auto [_e, pl, fpc] : ecs.entities_with<PlayerComponent, FPControllerComponent>()) {
        fpc.is_grounded = false;
    }

    for (int32_t it = 0; it < std::max(1, solver_iterations); ++it) {
        for (const Contact& c : contacts) {
            if (c.is_trigger) {
                resolve_trigger_contact(ecs, c);
            } else {
                resolve_phys_contact(ecs, c);
                positional_correction(ecs, c);
            }
        }
    }
}

void CollisionSystem::resolve_phys_contact(ECS& ecs, const Contact& c) {
    // Get components
    if (!ecs.has_component<RigidBodyComponent>(c.a) || !ecs.has_component<RigidBodyComponent>(c.b)) {
        return;
    }

    auto& a_rb = ecs.get_component<RigidBodyComponent>(c.a);
    auto& b_rb = ecs.get_component<RigidBodyComponent>(c.b);

    // Skip static/static
    if (a_rb.is_static && b_rb.is_static) {
        return;
    }

    // Determine if one of the two entities is the player
    bool is_a_player = ecs.has_component<PlayerComponent>(c.a);
    bool is_b_player = ecs.has_component<PlayerComponent>(c.b);

    float normal_y_for_player = is_a_player ? -c.normal.y : c.normal.y;  // flip if player is A
    if (is_a_player || is_b_player) {
        auto& fpc = ecs.get_component<FPControllerComponent>(is_a_player ? c.a : c.b);
        auto& rb = ecs.get_component<RigidBodyComponent>(is_a_player ? c.a : c.b);

        // Only set grounded if moving downward and hitting mostly horizontal surface
        if (normal_y_for_player > 0.75f && rb.velocity.y < 0.0f) {
            fpc.is_grounded = true;
            rb.velocity.y = 0.0f;
        }
    }

    // Relative velocity at contact
    glm::vec3 rv = b_rb.velocity - a_rb.velocity;
    float vel_along_normal = glm::dot(rv, c.normal);

    // Do not resolve separating contacts
    if (vel_along_normal > 0.0f) {
        return;
    }

    // Restitution
    float e = std::min(a_rb.restitution, b_rb.restitution);
    float inv_mass_sum = a_rb.inv_mass + b_rb.inv_mass;
    if (inv_mass_sum <= 0.0f) {
        return;
    }

    // Compute impulse
    float j = -(1.0f + e) * vel_along_normal;
    j /= inv_mass_sum;
    glm::vec3 impulse = j * c.normal;

    if (!a_rb.is_static && !a_rb.is_kinematic) {
        a_rb.apply_impulse(-impulse);
    }
    if (!b_rb.is_static && !b_rb.is_kinematic) {
        b_rb.apply_impulse(impulse);
    }

    // Simple friction with approximated Coulomb model
    glm::vec3 tangent = rv - vel_along_normal * c.normal;
    float tlen2 = glm::dot(tangent, tangent);
    if (tlen2 > 1e-8f) {
        tangent = glm::normalize(tangent);
        float mu = std::sqrt(a_rb.friction * b_rb.friction);
        float jt = -glm::dot(rv, tangent);
        jt /= inv_mass_sum;

        // Clamp friction impulse
        float jt_max = j * mu;
        jt = std::clamp(jt, -jt_max, jt_max);
        glm::vec3 friction_impulse = jt * tangent;
        if (!a_rb.is_static && !a_rb.is_kinematic) {
            a_rb.apply_impulse(-friction_impulse);
        }
        if (!b_rb.is_static && !b_rb.is_kinematic) {
            b_rb.apply_impulse(friction_impulse);
        }
    }
}

void CollisionSystem::positional_correction(ECS& ecs, const Contact& c) {
    if (!ecs.has_component<RigidBodyComponent>(c.a) || !ecs.has_component<RigidBodyComponent>(c.b) ||
        !ecs.has_component<TransformComponent>(c.a) || !ecs.has_component<TransformComponent>(c.b)) {
        return;
    }

    auto& a_rb = ecs.get_component<RigidBodyComponent>(c.a);
    auto& b_rb = ecs.get_component<RigidBodyComponent>(c.b);
    if (a_rb.is_static && b_rb.is_static) {
        return;
    }

    auto& a_tr = ecs.get_component<TransformComponent>(c.a);
    auto& b_tr = ecs.get_component<TransformComponent>(c.b);

    float inv_mass_sum = a_rb.inv_mass + b_rb.inv_mass;
    if (inv_mass_sum == 0.0f) {
        return;
    }

    float correction_mag = std::max(c.penetration - SLOP, 0.0f) / inv_mass_sum * COR_PER;
    glm::vec3 correction = correction_mag * c.normal;

    // move A opposite to the normal
    if (!a_rb.is_static && !a_rb.is_kinematic) {
        a_tr.update_position(-correction * a_rb.inv_mass);
    }

    // move B along the normal
    if (!b_rb.is_static && !b_rb.is_kinematic) {
        b_tr.update_position(correction * b_rb.inv_mass);
    }
}

void CollisionSystem::resolve_trigger_contact(ECS& /*ecs*/, const Contact& /*c*/) {
    // TODO: Implement
}