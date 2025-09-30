#pragma once

#include "components/icomponent.h"
#include "core/types/aabb.h"

#include <glm/glm.hpp>

struct Model : public IComponent {
    Model(AssetID model_id = 0) : asset_id(model_id) {
    }

    AssetID asset_id = 0;
    AABB local_aabb;
    bool visible = false;
    int32_t material_override_index = -1;  // optional per-submesh override
    bool casts_shadows = true;
    int32_t layer = 0;  // rendering layer / culling mask
};
