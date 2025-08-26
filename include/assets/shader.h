#pragma once

#include "assets/iasset.h"

#include <string>
#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <glad/glad.h>

class Shader : public IAsset {
public:
    Shader() = default;

    static std::shared_ptr<Shader> create_fallback();

    // This function assumes the provided path contains all the
    // shader files in the form s.vert, s.frag etc
    static std::optional<std::shared_ptr<Shader>> load_from_file(const std::string& path);
    static const char* base_path() {
        return "assets/shaders/";
    }
    const std::string& full_path();

    void use() const;
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

    void destroy();

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    uint32_t m_program_id;
    std::string m_full_path;
    std::unordered_map<std::string, int32_t> m_uniform_locations;

    static bool check_compile_errors(uint32_t shader, const std::string& type);
    void get_active_uniforms();
};
