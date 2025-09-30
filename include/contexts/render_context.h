#pragma once

#include "contexts/icontext.h"
#include "core/types/id.h"

struct RenderContext : public IContext {
    AssetID environment_id;
};