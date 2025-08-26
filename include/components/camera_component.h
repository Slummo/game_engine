#pragma once

#include "components/icomponent.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <cstdint>
#include <array>

enum class ProjectionType { Perspective, Orthographic };

struct CameraComponent : public IComponent {
    CameraComponent(glm::vec3 position = glm::vec3(0.0f), ProjectionType proj_type = ProjectionType::Perspective,
                    bool active = false, int32_t priority = 0)
        : m_position(position), m_proj_type(proj_type), m_active(active), m_priority(priority) {
    }

    void update_vectors() {
        if (!m_dirty.vectors_dirty) {
            return;
        }

        m_dirty.vectors_dirty = false;

        m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front.y = sin(glm::radians(m_pitch));
        m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(m_front);

        m_right = glm::normalize(glm::cross(m_front, m_world_up));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

    bool is_sphere_visible(const glm::vec3& center, float radius) {
        compute_frustum();

        for (int i = 0; i < 6; i++) {
            float distance = glm::dot(glm::vec3(m_frustum.planes[i]), center) + m_frustum.planes[i].w;
            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }

    bool is_AABB_visible(const glm::vec3& min, const glm::vec3& max) {
        compute_frustum();

        for (int i = 0; i < 6; i++) {
            glm::vec3 positive = min;
            if (m_frustum.planes[i].x >= 0) {
                positive.x = max.x;
            }
            if (m_frustum.planes[i].y >= 0) {
                positive.y = max.y;
            }
            if (m_frustum.planes[i].z >= 0) {
                positive.z = max.z;
            }

            if (glm::dot(glm::vec3(m_frustum.planes[i]), positive) + m_frustum.planes[i].w < 0) {
                return false;
            }
        }

        return true;
    }

    // GETTERS

    const glm::vec3& front() const {
        return m_front;
    }
    const glm::vec3& up() const {
        return m_up;
    }
    const glm::vec3& right() const {
        return m_right;
    }
    const glm::vec3& world_up() const {
        return m_world_up;
    }

    const glm::vec3& position() const {
        return m_position;
    }

    const glm::mat4& proj_matrix() {
        compute_proj_matrix();
        return m_proj_matrix;
    }

    const glm::mat4& view_matrix() {
        compute_view_matrix();
        return m_view_matrix;
    }

    float yaw() const {
        return m_yaw;
    }
    float pitch() const {
        return m_pitch;
    }
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
    float ortho_size() const {
        return m_ortho_size;
    }
    float ortho_near() const {
        return m_ortho_near;
    }
    float ortho_far() const {
        return m_ortho_far;
    }

    ProjectionType proj_type() const {
        return m_proj_type;
    }
    bool is_active() const {
        return m_active;
    }
    int32_t priority() const {
        return m_priority;
    }

    // SETTERS
    void set_position(const glm::vec3& position) {
        m_position = position;
        m_dirty.view_dirty = true;
    }

    void set_yaw(float v) {
        m_yaw = v;
        m_dirty.vectors_dirty = true;
        m_dirty.view_dirty = true;
    }
    void set_pitch(float v) {
        m_pitch = v;
        m_dirty.vectors_dirty = true;
        m_dirty.view_dirty = true;
    }
    void set_fov_y(float v) {
        m_fov_y = v;
        m_dirty.proj_dirty = true;
    }
    void set_aspect(float v) {
        m_aspect = v;
        m_dirty.proj_dirty = true;
    }
    void set_near_clip(float v) {
        m_near_clip = v;
        m_dirty.proj_dirty = true;
    }
    void set_far_clip(float v) {
        m_far_clip = v;
        m_dirty.proj_dirty = true;
    }
    void set_ortho_size(float v) {
        m_ortho_size = v;
        m_dirty.proj_dirty = true;
    }
    void set_ortho_near(float v) {
        m_ortho_near = v;
        m_dirty.proj_dirty = true;
    }
    void set_ortho_far(float v) {
        m_ortho_far = v;
        m_dirty.proj_dirty = true;
    }
    void set_proj_type(ProjectionType v) {
        m_proj_type = v;
        m_dirty.proj_dirty = true;
    }
    void set_active(bool v) {
        m_active = v;
    }
    void set_priority(int32_t v) {
        m_priority = v;
    }

    // Updaters
    void update_position(const glm::vec3& delta) {
        m_position += delta;
        m_dirty.view_dirty = true;
    }

    void update_yaw(float delta) {
        m_yaw += delta;
        m_dirty.vectors_dirty = true;
        m_dirty.view_dirty = true;
    }
    void update_pitch(float delta) {
        m_pitch += delta;
        m_dirty.vectors_dirty = true;
        m_dirty.view_dirty = true;
    }
    void update_fov_y(float delta) {
        m_fov_y += delta;
        m_dirty.proj_dirty = true;
    }
    void update_aspect(float delta) {
        m_aspect += delta;
        m_dirty.proj_dirty = true;
    }
    void update_near_clip(float delta) {
        m_near_clip += delta;
        m_dirty.proj_dirty = true;
    }
    void update_far_clip(float delta) {
        m_far_clip += delta;
        m_dirty.proj_dirty = true;
    }
    void update_ortho_size(float delta) {
        m_ortho_size += delta;
        m_dirty.proj_dirty = true;
    }
    void update_ortho_near(float delta) {
        m_ortho_near += delta;
        m_dirty.proj_dirty = true;
    }
    void update_ortho_far(float delta) {
        m_ortho_far += delta;
        m_dirty.proj_dirty = true;
    }

private:
    // Vectors
    glm::vec3 m_front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_world_up{0.0f, 1.0f, 0.0f};

    glm::vec3 m_position{0.0f};

    // Matrices
    glm::mat4 m_view_matrix{1.0f};
    glm::mat4 m_proj_matrix{1.0f};

    // View
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    // Perspective
    float m_fov_y = glm::radians(60.0f);
    float m_aspect = 16.0f / 9.0f;
    float m_near_clip = 0.1f;
    float m_far_clip = 1000.0f;

    // Orthographic
    float m_ortho_size = 10.0f;  // half-height of view
    float m_ortho_near = -1.0f;  // near plane
    float m_ortho_far = 1.0f;    // far plane

    // Other
    struct CameraDirty {
        bool vectors_dirty = true;
        bool view_dirty = true;
        bool proj_dirty = true;
    };

    ProjectionType m_proj_type = ProjectionType::Perspective;
    bool m_active = true;
    int32_t m_priority = 0;
    CameraDirty m_dirty;

    struct Frustum {
        glm::vec4 planes[6];  // store planes as coefficients for the cartesian form
    };
    Frustum m_frustum;

    void compute_proj_matrix() {
        if (!m_dirty.proj_dirty) {
            return;
        }

        m_dirty.proj_dirty = false;

        if (m_proj_type == ProjectionType::Perspective) {
            m_proj_matrix = glm::perspective(m_fov_y, m_aspect, m_near_clip, m_far_clip);
        } else {
            float hh = m_ortho_size;
            float hw = hh * m_aspect;

            m_proj_matrix = glm::ortho(-hw, hw, -hh, hh, m_ortho_near, m_ortho_far);
        }
    }

    void compute_view_matrix() {
        if (!m_dirty.view_dirty && !m_dirty.vectors_dirty) {
            return;
        }

        update_vectors();

        m_dirty.view_dirty = false;

        m_view_matrix = glm::lookAt(m_position, m_position + m_front, m_up);
    }

    void compute_frustum() {
        if (!m_dirty.proj_dirty && !m_dirty.view_dirty) {
            return;
        }

        glm::mat4 view_proj = proj_matrix() * view_matrix();

        glm::vec4 row_x = glm::row(view_proj, 0);
        glm::vec4 row_y = glm::row(view_proj, 1);
        glm::vec4 row_z = glm::row(view_proj, 2);
        glm::vec4 row_w = glm::row(view_proj, 3);

        // Left   = w + x
        m_frustum.planes[0] = row_w + row_x;
        // Right  = w - x
        m_frustum.planes[1] = row_w - row_x;
        // Bottom = w + y
        m_frustum.planes[2] = row_w + row_y;
        // Top    = w - y
        m_frustum.planes[3] = row_w - row_y;
        // Near   = w + z
        m_frustum.planes[4] = row_w + row_z;
        // Far    = w - z
        m_frustum.planes[5] = row_w - row_z;

        // Normalize planes
        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(m_frustum.planes[i]));
            m_frustum.planes[i] /= length;
        }

        m_dirty.proj_dirty = false;
        m_dirty.view_dirty = false;
    }
};
