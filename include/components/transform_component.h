#pragma once

#include "core/types.h"
#include "components/icomponent.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct TransformComponent : public IComponent {
public:
    TransformComponent(glm::vec3 pos = glm::vec3(0.0f), glm::quat rot = glm::quat(1, 0, 0, 0),
                       glm::vec3 scale = glm::vec3(1.0f), EntityID parent = 0)
        : m_position(pos), m_rotation(rot), m_scale(scale), m_parent(parent) {
    }

    void compute_model_matrix() {
        if (!m_dirty) {
            return;
        }

        m_dirty = false;

        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix *= glm::translate(glm::mat4(1.0f), m_position);
        m_model_matrix *= glm::mat4_cast(m_rotation);
        m_model_matrix *= glm::scale(glm::mat4(1.0f), m_scale);
    }

    // GETTERS

    const glm::vec3& position() const {
        return m_position;
    }
    const glm::quat& rotation() const {
        return m_rotation;
    }
    const glm::vec3& scale() const {
        return m_scale;
    }
    const glm::mat4& model_matrix() const {
        return m_model_matrix;
    }
    EntityID parent() const {
        return m_parent;
    }

    // SETTERS

    void set_position(glm::vec3 position) {
        m_position = position;
        m_dirty = true;
    }
    void set_rotation(glm::quat rotation) {
        m_rotation = rotation;
        m_dirty = true;
    }
    void set_scale(glm::vec3 visual_scale) {
        m_scale = visual_scale;
        m_dirty = true;
    }
    void set_parent(EntityID p) {
        m_parent = p;
    }

    // UPDATERS

    void update_position(const glm::vec3& delta) {
        m_position += delta;
        m_dirty = true;
    }
    void update_rotation(const glm::quat& delta) {
        // Quaternion multiplication to rotate incrementally
        m_rotation = glm::normalize(delta * m_rotation);
        m_dirty = true;
    }
    void update_scale(const glm::vec3& delta) {
        m_scale += delta;
        m_dirty = true;
    }

private:
    glm::vec3 m_position{0.0f};
    glm::quat m_rotation{1, 0, 0, 0};
    glm::vec3 m_scale{1.0f};

    glm::mat4 m_model_matrix{1.0f};

    bool m_dirty = true;
    EntityID m_parent = 0;
};
