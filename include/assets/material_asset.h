#pragma once

#include "core/types.h"
#include "assets/iasset.h"
#include "assets/texture_asset.h"
#include "assets/shader_asset.h"

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <assimp/scene.h>

struct TexData {
    TextureType type;
    std::string path;
    bool is_path_absolute = false;
};

using ParamValue = std::variant<float, glm::vec3, std::string>;

class Material : public IAsset {
public:
    // Loads texture assets and the shader asset
    Material(std::string name, std::vector<TexData> textures_data, const std::string& shader_name,
             bool double_sided = false);

    // For material with a single texture
    Material(std::string name, TextureType texture_type, std::string texture_name, const std::string& shader_name,
             bool double_sided = false);

    // Loads texture assets and the shader asset
    Material(const aiMaterial* ai_mat, const std::string& model_path, const std::string& shader_name);

    // Used to create fallback material
    Material(std::string name);

    static std::shared_ptr<Material> create_fallback();

    void set_uniforms();

    const std::string& name() const;

    bool get_texture(TextureType type, AssetID& out) const;

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

    // Binds the shader and returns it
    const ShaderAsset& bound_shader() const;

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    std::string m_name;
    std::unordered_map<TextureType, AssetID> m_textures;
    std::unordered_map<std::string, ParamValue> m_params;
    AssetID m_shader_id;
    bool m_double_sided;

    Material() = default;

    static std::string load_name(const aiMaterial* ai_mat);
    static std::vector<TexData> load_textures(const aiMaterial* ai_mat, const std::string& model_path,
                                              std::vector<TextureType> texture_types);
};
