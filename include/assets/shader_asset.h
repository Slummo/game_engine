#pragma once

#include "assets/interfaces.h"

#include <string>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <unordered_map>
#include <glad/glad.h>

class ShaderAsset : public IAsset {
public:
    ShaderAsset(std::string name);

    void use() const;
    void get_active_uniforms();
    int32_t get_uniform_location(const std::string& name) const;
    void set_bool(const std::string& name, bool value) const;
    void set_int(const std::string& name, int32_t value) const;
    void set_float(const std::string& name, float value) const;
    void set_vec_3f(const std::string& name, const GLfloat* ptr) const;
    void set_vec_3f(const std::string& name, const glm::vec3& vec) const;
    void set_matrix_3f(const std::string& name, const GLfloat* ptr) const;
    void set_matrix_3f(const std::string& name, const glm::mat3& mat) const;
    void set_matrix_4f(const std::string& name, const GLfloat* ptr) const;
    void set_matrix_4f(const std::string& name, const glm::mat4& mat) const;

    ~ShaderAsset();

    uint32_t program_id = 0;
    std::unordered_map<std::string, int32_t> uniform_locations;

protected:
    std::ostream& print(std::ostream& os) const override;
};

template <>
class AssetCreator<ShaderAsset> : public AssetCreatorNoDep<ShaderAsset> {
public:
    using Base = AssetCreatorNoDep<ShaderAsset>;
    using Base::Base;

    static std::shared_ptr<ShaderAsset> create_fallback(AssetManager& am);
};

template <>
class AssetLoader<ShaderAsset> : public AssetLoaderNoDep<ShaderAsset> {
public:
    using Base = AssetLoaderNoDep<ShaderAsset>;
    using Base::Base;

    static const char* base_path();
    AssetID finish() override;
};