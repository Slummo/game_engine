#pragma once

#include "core/types.h"
#include "components/icomponent.h"
#include "managers/asset_manager.h"

#include <glm/glm.hpp>

struct ModelComponent : public IComponent {
    ModelComponent(AssetID model_id = 0) : model_id(model_id) {
        AssetManager& am = AssetManager::instance();
        auto& mesh_ids = am.get_asset<Model>(model_id).meshes();

        // Compute model's AABB
        glm::vec3 min(FLT_MAX);
        glm::vec3 max(-FLT_MAX);
        for (AssetID mesh_id : mesh_ids) {
            Mesh& mesh = am.get_asset<Mesh>(mesh_id);
            min = glm::min(min, mesh.aabb().min);
            max = glm::max(max, mesh.aabb().max);
        }
        model_aabb = {min, max};
    }

    AssetID model_id = 0;
    AABB model_aabb;
    int material_override_index = -1;  // optional per-submesh override
    bool casts_shadows = true;
    int layer = 0;  // rendering layer / culling mask
};
