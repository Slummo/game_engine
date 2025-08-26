#include "assets/material.h"
#include "managers/asset_manager.h"

#include <filesystem>

Material::Material(std::string name, std::vector<TexData> textures_data, const std::string& shader_name,
                   bool double_sided)
    : m_name(name.empty() ? "unnamed_mat" : std::move(name)), m_double_sided(double_sided) {
    AssetManager& am = AssetManager::instance();

    for (TexData& data : textures_data) {
        AssetID tex_id = data.is_path_absolute ? am.load_asset_path<Texture>(data.path, data.type)
                                               : am.load_asset<Texture>(data.path, data.type);
        m_textures[data.type] = tex_id;
    }
    m_shader_id = am.load_asset<Shader>(shader_name + "/");
}

Material::Material(std::string name, TextureType texture_type, std::string texture_name, const std::string& shader_name,
                   bool double_sided)
    : Material(name, {{texture_type, std::move(texture_name)}}, shader_name, double_sided) {
}

Material::Material(const aiMaterial* ai_mat, const std::string& model_path, const std::string& shader_name)
    : Material(Material::load_name(ai_mat),
               Material::load_textures(ai_mat, model_path,
                                       {TextureType::Diffuse, TextureType::Specular, TextureType::Ambient}),
               shader_name) {
}

Material::Material(std::string name) : m_name(std::move(name)), m_double_sided(false) {
    AssetManager& am = AssetManager::instance();
    AssetID tex_id = am.get_fallback_id<Texture>();
    m_textures[TextureType::Diffuse] = tex_id;
    m_shader_id = am.get_fallback_id<Shader>();
}

std::shared_ptr<Material> Material::create_fallback() {
    return std::make_shared<Material>("fallback_material");
}

void Material::set_uniforms() {
    AssetManager& am = AssetManager::instance();
    const Shader& shader = bound_shader();

    glm::vec3 base_color = get_param_or_default<glm::vec3>("base_color", glm::vec3(1.0f));  // default white
    shader.set_vec_3f("mat.base_color", base_color);

    shader.set_bool("mat.has_diffuse_map", false);
    shader.set_bool("mat.has_specular_map", false);
    shader.set_bool("mat.has_ambient_map", false);

    AssetID tex_id;
    if (get_texture(TextureType::Diffuse, tex_id)) {  // diffuse texture
        Texture& diffuse_tex = am.get_asset<Texture>(tex_id);
        diffuse_tex.bind(0);
        shader.set_int("mat.diffuse_map", 0);
        shader.set_bool("mat.has_diffuse_map", true);
    }
    if (get_texture(TextureType::Specular, tex_id)) {  // specular texture
        Texture& specular_tex = am.get_asset<Texture>(tex_id);
        specular_tex.bind(1);
        shader.set_int("mat.specular_map", 1);
        shader.set_bool("mat.has_specular_map", true);
    }
    if (get_texture(TextureType::Ambient, tex_id)) {  // ambient texture
        Texture& ambient_tex = am.get_asset<Texture>(tex_id);
        ambient_tex.bind(2);
        shader.set_int("mat.ambient_map", 2);
        shader.set_bool("mat.has_ambient_map", true);
    }

    float shininess = get_param_or_default<float>("shininess", 32.0f);
    shader.set_float("mat.shininess", shininess);
}

bool Material::get_texture(TextureType type, AssetID& out) const {
    auto it = m_textures.find(type);
    if (it == m_textures.end()) {
        return false;
    }

    out = it->second;
    return true;
}

const Shader& Material::bound_shader() const {
    AssetManager& am = AssetManager::instance();
    Shader& shader = am.get_asset<Shader>(m_shader_id);
    if (am.last_used_shader() != m_shader_id) {
        shader.use();
        am.set_last_used_shader(m_shader_id);
    }
    return shader;
}

std::ostream& Material::print(std::ostream& os) const {
    return os << "Material(name: " << m_name << ", params_num: " << m_params.size()
              << ", textures_num: " << m_textures.size() << ", shader (asset_id = " << m_shader_id
              << "): " << AssetManager::instance().asset_to_string(m_shader_id) << ", double_sided: " << m_double_sided
              << ")";
}

std::string Material::load_name(const aiMaterial* ai_mat) {
    aiString name;
    return ai_mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS ? std::string(name.C_Str()) : "";
}

std::vector<TexData> Material::load_textures(const aiMaterial* ai_mat, const std::string& model_path,
                                             std::vector<TextureType> texture_types) {
    std::vector<TexData> textures_data;

    for (TextureType type : texture_types) {
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

            textures_data.push_back({type, std::string(model_path + raw_path), true});
        }
    }

    return textures_data;
}