#include "assets/material_asset.h"
#include "assets/shader_asset.h"
#include "managers/asset_manager.h"

#include <filesystem>

MaterialAsset::MaterialAsset(std::string name, AssetID shader_id) : IAsset(std::move(name)), m_shader_id(shader_id) {
}

void MaterialAsset::add_texture(MaterialTextureType type, AssetID texture_id) {
    if (texture_id == INVALID_ASSET) {
        return;
    }

    m_textures[type] = texture_id;
}

bool MaterialAsset::get_texture(MaterialTextureType type, AssetID& out) const {
    auto it = m_textures.find(type);
    if (it == m_textures.end()) {
        return false;
    }

    out = it->second;
    return true;
}

AssetID MaterialAsset::shader_id() const {
    return m_shader_id;
}

std::ostream& MaterialAsset::print(std::ostream& os) const {
    return os << "MaterialAsset(name: " << m_name << ", params_num: " << m_params.size()
              << ", textures_num: " << m_textures.size() << ", shader_id: " << m_shader_id
              << ", double_sided: " << m_double_sided << ")";
}

std::string MaterialAsset::load_name(const aiMaterial* ai_mat) {
    aiString name;
    return ai_mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS ? std::string(name.C_Str()) : "";
}

std::vector<TexData> MaterialAsset::load_textures(const aiMaterial* ai_mat, const std::string& model_path,
                                                  std::vector<MaterialTextureType> texture_types) {
    std::vector<TexData> textures_data;

    for (MaterialTextureType type : texture_types) {
        aiTextureType ai_type = static_cast<aiTextureType>(type);

        for (uint32_t i = 0; i < ai_mat->GetTextureCount(ai_type); i++) {
            aiString ai_texture_path;
            if (ai_mat->GetTexture(ai_type, i, &ai_texture_path) == AI_FAILURE) {
                continue;
            }

            std::string raw_path(ai_texture_path.C_Str());

            // Remove "./" or ".\" prefixes
            if (raw_path.size() >= 2 && ((raw_path[0] == '.' && (raw_path[1] == '/' || raw_path[1] == '\\')))) {
                raw_path = raw_path.substr(2);
            }

            textures_data.push_back({type, std::string(model_path + raw_path)});
        }
    }

    return textures_data;
}

std::shared_ptr<MaterialAsset> AssetCreator<MaterialAsset>::create_fallback(AssetManager& am) {
    AssetID fallback_shader = am.get_fallback_id<ShaderAsset>();
    auto mat = std::make_shared<MaterialAsset>("fallback_material", fallback_shader);

    AssetID fallback_tex = am.get_fallback_id<TextureAsset>();
    mat->add_texture(MaterialTextureType::Ambient, fallback_tex);
    mat->add_texture(MaterialTextureType::Diffuse, fallback_tex);
    mat->add_texture(MaterialTextureType::Specular, fallback_tex);

    return mat;
}

AssetID AssetCreator<MaterialAsset>::finish() {
    AssetID shader_id = resolve_slot(MaterialDepSlot::Shader, am.get_fallback_id<ShaderAsset>());
    auto mat = std::make_shared<MaterialAsset>(name, shader_id);

    AssetID fallback_tex = am.get_fallback_id<TextureAsset>();
    mat->add_texture(MaterialTextureType::Ambient, resolve_slot(MaterialDepSlot::Ambient, fallback_tex, false));
    mat->add_texture(MaterialTextureType::Diffuse, resolve_slot(MaterialDepSlot::Diffuse, fallback_tex));
    mat->add_texture(MaterialTextureType::Specular, resolve_slot(MaterialDepSlot::Specular, fallback_tex, false));

    return am.add<MaterialAsset>(std::move(mat));
}