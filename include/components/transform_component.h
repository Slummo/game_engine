#pragma once

#include "core/types.h"
#include "components/icomponent.h"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/epsilon.hpp>

#define TR_POS_EPS 1e-6f
#define TR_ROT_EPS 1e-6f
#define TR_SCALE_EPS 1e-6f

struct TransformComponent : public IComponent {
public:
    TransformComponent(glm::vec3 pos = glm::vec3(0.0f), glm::quat rot = glm::quat(1, 0, 0, 0),
                       glm::vec3 scale = glm::vec3(1.0f), EntityID parent = 0)
        : parent(parent), m_position(pos), m_rotation(rot), m_scale(scale) {
    }

    EntityID parent = 0;

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

    const glm::mat4& model_matrix() {
        compute_model_matrix();
        return m_model_matrix;
    }

    // SETTERS

    void set_position(const glm::vec3& pos) {
        if (glm::all(glm::epsilonEqual(m_position, pos, TR_POS_EPS))) {
            return;
        }

        m_position = pos;
        dirty = true;
    }
    void set_rotation(const glm::quat& rot) {
        glm::quat rot_norm = glm::normalize(rot);
        if (glm::all(glm::epsilonEqual(m_rotation, rot_norm, TR_ROT_EPS)) ||
            glm::all(glm::epsilonEqual(m_rotation, -rot_norm, TR_ROT_EPS))) {
            return;
        }

        m_rotation = rot_norm;
        dirty = true;
    }
    void set_scale(const glm::vec3& sc) {
        if (glm::all(glm::epsilonEqual(m_scale, sc, TR_SCALE_EPS))) {
            return;
        }

        m_scale = sc;
        dirty = true;
    }

    // UPDATERS

    void update_position(const glm::vec3& delta) {
        if (glm::all(glm::epsilonEqual(delta, glm::vec3(0.0f), TR_POS_EPS))) {
            return;
        }

        m_position += delta;
        dirty = true;
    }
    void update_rotation(const glm::quat& delta) {
        glm::quat delta_nor = glm::normalize(delta);

        // Compute rotation angle from quaternion
        float angle = 2.0f * std::acos(glm::clamp(delta_nor.w, -1.0f, 1.0f));
        if (std::fabs(angle) <= TR_ROT_EPS) {
            return;
        }

        // Quaternion multiplication to rotate incrementally
        m_rotation = glm::normalize(delta_nor * m_rotation);
        dirty = true;
    }
    void update_scale(const glm::vec3& delta) {
        if (glm::all(glm::epsilonEqual(delta, glm::vec3(0.0f), TR_SCALE_EPS))) {
            return;
        }

        m_scale += delta;
        dirty = true;
    }

private:
    glm::vec3 m_position{0.0f};
    glm::quat m_rotation{1, 0, 0, 0};
    glm::vec3 m_scale{1.0f};
    glm::mat4 m_model_matrix{1.0f};

    bool dirty = true;

    void compute_model_matrix() {
        if (!dirty) {
            return;
        }

        dirty = false;

        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix *= glm::translate(glm::mat4(1.0f), m_position);
        m_model_matrix *= glm::mat4_cast(m_rotation);
        m_model_matrix *= glm::scale(glm::mat4(1.0f), m_scale);
    }
};
