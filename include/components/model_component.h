#pragma once

#include "core/types.h"
#include "components/icomponent.h"
#include "managers/asset_manager.h"

#include <glm/glm.hpp>
#include <limits>
#include <cstdint>

struct ModelComponent : public IComponent {
    ModelComponent(AssetID model_id = 0) : asset_id(model_id) {
        AssetManager& am = AssetManager::instance();
        auto& mesh_ids = am.get_asset<Model>(model_id).meshes();

        // Compute model's local AABB
        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(-std::numeric_limits<float>::max());
        for (AssetID mesh_id : mesh_ids) {
            Mesh& mesh = am.get_asset<Mesh>(mesh_id);
            const AABB& local_aabb = mesh.local_aabb();
            min = glm::min(min, local_aabb.min);
            max = glm::max(max, local_aabb.max);
        }
        local_aabb = {min, max};
    }

    AssetID asset_id = 0;
    AABB local_aabb;
    bool visible = false;
    int32_t material_override_index = -1;  // optional per-submesh override
    bool casts_shadows = true;
    int32_t layer = 0;  // rendering layer / culling mask
};
