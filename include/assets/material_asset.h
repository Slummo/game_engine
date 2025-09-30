#pragma once

#include "assets/interfaces.h"
#include "assets/texture_asset.h"

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>

struct TexData {
    MaterialTextureType type = MaterialTextureType::None;
    std::string path;
};

using ParamValue = std::variant<float, glm::vec3, std::string>;

class MaterialAsset : public IAsset {
public:
    MaterialAsset(std::string name, AssetID shader_id);

    void add_texture(MaterialTextureType type, AssetID texture_id);
    bool get_texture(MaterialTextureType type, AssetID& out) const;

    template <typename T>
    bool get_param(const std::string& name, T& out) const {
        auto it = m_params.find(name);
        if (it == m_params.end()) {
            return false;
        }

        auto val = std::get_if<T>(&it->second);
        if (val) {
            out = *val;
            return true;
        }

        return false;
    }

    template <typename T>
    T get_param_or_default(const std::string& name, const T& default_value) const {
        T val;
        if (get_param(name, val)) {
            return val;
        }

        return default_value;
    }

    AssetID shader_id() const;

    static std::string load_name(const aiMaterial* ai_mat);
    static std::vector<TexData> load_textures(const aiMaterial* ai_mat, const std::string& model_path,
                                              std::vector<MaterialTextureType> texture_types);

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    std::unordered_map<MaterialTextureType, AssetID> m_textures;
    std::unordered_map<std::string, ParamValue> m_params;
    AssetID m_shader_id = INVALID_ASSET;
    bool m_double_sided = false;
};

enum class MaterialDepSlot { Shader, Ambient, Diffuse, Specular };

template <>
class AssetCreator<MaterialAsset> : public AssetCreatorDep<MaterialAsset, MaterialDepSlot> {
public:
    using Base = AssetCreatorDep<MaterialAsset, MaterialDepSlot>;
    using Base::Base;

    static std::shared_ptr<MaterialAsset> create_fallback(AssetManager& am);
    AssetID finish() override;
};