#pragma once

#include "components/icomponent.h"
#include "core/log.h"

#include <cstdint>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/epsilon.hpp>

#define CAM_WPOS_EPS 1e-2f
#define CAM_PARAM_EPS 1e-6f

enum class ProjectionType { Perspective, Orthographic };

struct Frustum {
    glm::vec4 planes[6];  // store planes as coefficients for the cartesian form

    Frustum() = default;

    void compute(const glm::mat4& view_proj) {
        glm::vec4 row_x = glm::row(view_proj, 0);
        glm::vec4 row_y = glm::row(view_proj, 1);
        glm::vec4 row_z = glm::row(view_proj, 2);
        glm::vec4 row_w = glm::row(view_proj, 3);

        // Left   = w + x
        planes[0] = row_w + row_x;
        // Right  = w - x
        planes[1] = row_w - row_x;
        // Bottom = w + y
        planes[2] = row_w + row_y;
        // Top    = w - y
        planes[3] = row_w - row_y;
        // Near   = w + z
        planes[4] = row_w + row_z;
        // Far    = w - z
        planes[5] = row_w - row_z;

        // Normalize planes
        for (int32_t i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(planes[i]));
            planes[i] /= length;
        }
    }

    bool is_sphere_visible(const glm::vec3& center, float radius) {
        for (int32_t i = 0; i < 6; i++) {
            float distance = glm::dot(glm::vec3(planes[i]), center) + planes[i].w;
            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }

    bool is_AABB_visible(const glm::vec3& min, const glm::vec3& max) {
        for (int32_t i = 0; i < 6; i++) {
            glm::vec3 positive = min;
            if (planes[i].x >= 0) {
                positive.x = max.x;
            }
            if (planes[i].y >= 0) {
                positive.y = max.y;
            }
            if (planes[i].z >= 0) {
                positive.z = max.z;
            }

            if (glm::dot(glm::vec3(planes[i]), positive) + planes[i].w < 0) {
                return false;
            }
        }

        return true;
    }
};

struct CameraComponent : public IComponent {
    CameraComponent(glm::vec3 offset = glm::vec3(0.0f), ProjectionType proj_type = ProjectionType::Perspective,
                    bool active = false, int32_t priority = 0)
        : offset(offset), is_active(active), priority(priority), proj_type(proj_type) {
    }

    glm::vec3 offset{0.0f};  // Local offset relative to its owner's position
    bool is_active = true;
    int32_t priority = 0;

    // GETTERS

    const glm::vec3& world_position() const {
        return m_world_pos;
    }

    // Projection - Perspective

    float fov_y() const {
        return m_fov_y;
    }
    float aspect() const {
        return m_aspect;
    }
    float near_clip() const {
        return m_near_clip;
    }
    float far_clip() const {
        return m_far_clip;
    }

    // Projection - Orthographic

    float ortho_size() const {
        return m_ortho_size;
    }
    float ortho_near() const {
        return m_ortho_near;
    }
    float ortho_far() const {
        return m_ortho_far;
    }

    // View

    float yaw() const {
        return m_yaw;
    }
    float pitch() const {
        return m_pitch;
    }
    const glm::vec3& front() const {
        return m_front;
    }
    const glm::vec3& up() const {
        return m_up;
    }

    const glm::mat4& proj_matrix() {
        if (proj_type == ProjectionType::Perspective) {
            compute_proj_matrix_perspective();
        } else {
            compute_proj_matrix_ortographic();
        }
        return m_proj_matrix;
    }

    const glm::mat4& view_matrix() {
        compute_view_matrix();
        return m_view_matrix;
    }

    Frustum& frustum() {
        compute_frustum();
        return m_frustum;
    }

    // SETTERS

    void set_world_position(const glm::vec3& world_position) {
        if (glm::all(glm::epsilonEqual(m_world_pos, world_position, CAM_WPOS_EPS))) {
            return;
        }

        m_world_pos = world_position;
        view_dirty = true;
        frustum_dirty = true;
    }

    // Projection - Perspective

    void set_fov_y(float v) {
        if (std::fabs(m_fov_y - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_fov_y = v;
        proj_perspective_dirty = true;
        frustum_dirty = true;
    }
    void set_aspect(float v) {
        if (std::fabs(m_aspect - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_aspect = v;
        proj_perspective_dirty = true;
    }
    void set_near_clip(float v) {
        if (std::fabs(m_near_clip - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_near_clip = v;
        proj_perspective_dirty = true;
        frustum_dirty = true;
    }
    void set_far_clip(float v) {
        if (std::fabs(m_far_clip - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_far_clip = v;
        proj_perspective_dirty = true;
        frustum_dirty = true;
    }

    // Projection - Orthographic

    void set_ortho_size(float v) {
        if (std::fabs(m_ortho_size - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_ortho_size = v;
        proj_ortographic_dirty = true;
        frustum_dirty = true;
    }
    void set_ortho_near(float v) {
        if (std::fabs(m_ortho_near - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_ortho_near = v;
        proj_ortographic_dirty = true;
        frustum_dirty = true;
    }
    void set_ortho_far(float v) {
        if (std::fabs(m_ortho_far - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_ortho_far = v;
        proj_ortographic_dirty = true;
        frustum_dirty = true;
    }

    // View

    void set_yaw(float v) {
        if (std::fabs(m_yaw - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_yaw = v;
        view_dirty = true;
        frustum_dirty = true;
    }
    void set_pitch(float v) {
        if (std::fabs(m_pitch - v) <= CAM_PARAM_EPS) {
            return;
        }

        m_pitch = glm::clamp(v, -89.0f, 89.0f);
        view_dirty = true;
        frustum_dirty = true;
    }

    // Others

    void set_proj_type(ProjectionType v) {
        proj_type = v;
        if (v == ProjectionType::Perspective) {
            proj_perspective_dirty = true;
        } else {
            proj_ortographic_dirty = true;
        }
        frustum_dirty = true;
    }

    // UPDATERS

    void update_offset(const glm::vec3& delta) {
        offset += delta;
    }

    // Projection - Perspective

    void update_fov_y(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_fov_y += delta;
        proj_perspective_dirty = true;
        frustum_dirty = true;
    }
    void update_aspect(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_aspect += delta;
        proj_perspective_dirty = true;
        frustum_dirty = true;
    }
    void update_near_clip(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_near_clip += delta;
        proj_perspective_dirty = true;
        frustum_dirty = true;
    }
    void update_far_clip(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_far_clip += delta;
        proj_perspective_dirty = true;
        frustum_dirty = true;
    }

    // Projection - Orthographic

    void update_ortho_size(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_ortho_size += delta;
        proj_ortographic_dirty = true;
        frustum_dirty = true;
    }
    void update_ortho_near(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_ortho_near += delta;
        proj_ortographic_dirty = true;
        frustum_dirty = true;
    }
    void update_ortho_far(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_ortho_far += delta;
        proj_ortographic_dirty = true;
        frustum_dirty = true;
    }

    // View

    void update_yaw(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_yaw += delta;
        view_dirty = true;
        frustum_dirty = true;
    }
    void update_pitch(float delta) {
        if (std::fabs(delta) <= CAM_PARAM_EPS) {
            return;
        }

        m_pitch = glm::clamp(m_pitch + delta, -89.0f, 89.0f);
        view_dirty = true;
        frustum_dirty = true;
    }

private:
    glm::vec3 m_world_pos{0.0f};

    // Matrices
    glm::mat4 m_proj_matrix{1.0f};
    glm::mat4 m_view_matrix{1.0f};

    // Projection - Perspective
    float m_fov_y = glm::radians(60.0f);
    float m_aspect = 16.0f / 9.0f;
    float m_near_clip = 0.1f;
    float m_far_clip = 1000.0f;
    // Projection - Orthographic
    float m_ortho_size = 10.0f;     // half-height of view
    float m_ortho_near = -1000.0f;  // near plane
    float m_ortho_far = 1000.0f;    // far plane

    // View
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    glm::vec3 m_front{0.0f};
    glm::vec3 m_up{0.0f};

    // Other
    ProjectionType proj_type = ProjectionType::Perspective;
    Frustum m_frustum;

    // Dirty flags
    bool proj_perspective_dirty = true;
    bool proj_ortographic_dirty = true;
    bool view_dirty = true;
    bool frustum_dirty = true;

    void compute_proj_matrix_perspective() {
        if (proj_perspective_dirty) {
            m_proj_matrix = glm::perspective(m_fov_y, m_aspect, m_near_clip, m_far_clip);

            proj_perspective_dirty = false;
        }
    }

    void compute_proj_matrix_ortographic() {
        if (proj_ortographic_dirty) {
            float hh = m_ortho_size;
            float hw = hh * m_aspect;

            m_proj_matrix = glm::ortho(-hw, hw, -hh, hh, m_ortho_near, m_ortho_far);

            proj_ortographic_dirty = false;
        }
    }

    void compute_view_matrix() {
        if (view_dirty) {
            // View vectors
            m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_front.y = sin(glm::radians(m_pitch));
            m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_front = glm::normalize(m_front);

            glm::vec3 right = glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));  // Use world up vector
            m_up = glm::normalize(glm::cross(right, m_front));

            m_view_matrix = glm::lookAt(m_world_pos, m_world_pos + m_front, m_up);
            view_dirty = false;
        }
    }

    void compute_frustum() {
        if (frustum_dirty) {
            m_frustum.compute(proj_matrix() * view_matrix());
            frustum_dirty = false;
        }
    }
};
