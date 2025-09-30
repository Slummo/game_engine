#include "assets/interfaces.h"

#include "assets/sound_asset.h"
#include "assets/shader_asset.h"
#include "assets/texture_asset.h"
#include "assets/model_asset.h"

template <typename T, typename Slot, typename Derived>
AssetID AssetCreatorBase<T, Slot, Derived>::resolve_dep(const Dep& dep, bool required) {
    switch (dep.index()) {
        case 0: {  // monostate
            if (required) {
                throw std::runtime_error("Asset has uninitialized dep in slot!");
            }
            return INVALID_ASSET;
        }
        case 1: {  // CreateDep
            auto& d = std::get<CreateDep>(dep);
            if (required && !am.exists(d.id)) {
                throw std::runtime_error("Missing dependency");
            }
            return d.id;
        }
        case 2: {  // LoadDep
            auto d = std::get<LoadDep>(dep);
            switch (d.type) {
                case LoadableAssetType::Sound: {
                    return am.template load<SoundAsset>(std::move(d.name), std::move(d.path)).finish();
                }
                case LoadableAssetType::Shader: {
                    return am.template load<ShaderAsset>(std::move(d.name), std::move(d.path)).finish();
                }
                case LoadableAssetType::Texture: {
                    return am.template load<TextureAsset>(std::move(d.name), std::move(d.path), d.is_path_relative)
                        .set_type(to_mat_texture_type(d.tex_type))
                        .set_params(TextureParams::default_material_params())
                        .finish();
                }
                case LoadableAssetType::Model: {
                    return am.template load<ModelAsset>(std::move(d.name), std::move(d.path)).finish();
                }
                default: {
                    throw std::runtime_error("Unhandled LoadableAssetType");
                }
            }
        }
        default:
            throw std::runtime_error("Unknown dependency variant index");
    }
}