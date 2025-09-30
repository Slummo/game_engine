#include "assets/shader_asset.h"
#include "managers/asset_manager.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdint>

ShaderAsset::ShaderAsset(std::string name) : IAsset(name.empty() ? "unnamed_shader" : std::move(name)) {
}

void ShaderAsset::use() const {
    glUseProgram(program_id);
}

void ShaderAsset::set_bool(const std::string& name, bool value) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform1i(loc, (int32_t)value);
}

void ShaderAsset::set_int(const std::string& name, int32_t value) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform1i(loc, value);
}

void ShaderAsset::set_float(const std::string& name, float value) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform1f(loc, value);
}

void ShaderAsset::set_vec_3f(const std::string& name, const GLfloat* ptr) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform3fv(loc, 1, ptr);
}

void ShaderAsset::set_vec_3f(const std::string& name, const glm::vec3& vec) const {
    set_vec_3f(name, glm::value_ptr(vec));
}

void ShaderAsset::set_matrix_3f(const std::string& name, const GLfloat* ptr) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniformMatrix3fv(loc, 1, GL_FALSE, ptr);
}

void ShaderAsset::set_matrix_3f(const std::string& name, const glm::mat3& mat) const {
    set_matrix_3f(name, glm::value_ptr(mat));
}

void ShaderAsset::set_matrix_4f(const std::string& name, const GLfloat* ptr) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, ptr);
}

void ShaderAsset::set_matrix_4f(const std::string& name, const glm::mat4& mat) const {
    set_matrix_4f(name, glm::value_ptr(mat));
}

ShaderAsset::~ShaderAsset() {
    if (program_id) {
        glDeleteProgram(program_id);
        program_id = 0;
    }
}

std::ostream& ShaderAsset::print(std::ostream& os) const {
    return os << "ShaderAsset(program_id: " << program_id << ")";
}

void ShaderAsset::get_active_uniforms() {
    GLint count;
    glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &count);

    for (int32_t i = 0; i < count; i++) {
        char name[128];
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(program_id, (GLuint)i, sizeof(name), &length, &size, &type, name);

        GLint loc = glGetUniformLocation(program_id, name);
        uniform_locations[name] = loc;
    }
}

int32_t ShaderAsset::get_uniform_location(const std::string& name) const {
    auto it = uniform_locations.find(name);
    return it == uniform_locations.end() ? -1 : it->second;
}

std::shared_ptr<ShaderAsset> AssetCreator<ShaderAsset>::create_fallback(AssetManager& /*am*/) {
    return std::make_shared<ShaderAsset>("fallback_shader");
}

bool check_compile_errors(uint32_t shader, const std::string& type) {
    int32_t success;
    char info_log[1024];
    if (type != "Program") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, info_log);
            ERR("[AssetLoader<ShaderAsset>] compilation error for " << type << ":\n\t" << info_log)
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, info_log);
            ERR("[AssetLoader<ShaderAsset>] linking error for " << type << ":\n\t" << info_log);
            return false;
        }
    }

    return true;
}

const char* AssetLoader<ShaderAsset>::base_path() {
    return "assets/shaders/";
}

AssetID AssetLoader<ShaderAsset>::finish() {
    AssetID id = am.is_loaded(absolute_path);
    if (id != INVALID_ASSET) {
        return id;
    }

    std::ifstream v_file, f_file;

    // Ensure ifstream objects can throw exceptions:
    v_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::string v_code, f_code;
    try {
        // Open files
        v_file.open(absolute_path + "/s.vert");
        f_file.open(absolute_path + "/s.frag");

        std::stringstream v_buf, f_buf;

        // Read files content
        v_buf << v_file.rdbuf();
        f_buf << f_file.rdbuf();

        // Close files
        v_file.close();
        f_file.close();

        // Read buffers as strings
        v_code = v_buf.str();
        f_code = f_buf.str();

    } catch (std::ifstream::failure& e) {
        ERR("[AssetLoader<ShaderAsset>] Error while reading shader file: " << e.what());
        return INVALID_ASSET;
    }

    // Convert into a c-style string
    const char* v_code_c = v_code.c_str();
    const char* f_code_c = f_code.c_str();

    uint32_t vertex, fragment;

    // Vertex
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_code_c, nullptr);
    glCompileShader(vertex);
    if (!check_compile_errors(vertex, "Vertex shader")) {
        return INVALID_ASSET;
    }

    // Fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_code_c, nullptr);
    glCompileShader(fragment);
    if (!check_compile_errors(fragment, "Fragment shader")) {
        return INVALID_ASSET;
    }

    // Create program and attach shaders
    auto shader = std::make_shared<ShaderAsset>(std::move(name));
    shader->program_id = glCreateProgram();
    glAttachShader(shader->program_id, vertex);
    glAttachShader(shader->program_id, fragment);
    glLinkProgram(shader->program_id);
    if (!check_compile_errors(shader->program_id, "Program")) {
        shader->~ShaderAsset();
        return INVALID_ASSET;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Cache uniform locations
    shader->get_active_uniforms();

    id = am.add<ShaderAsset>(std::move(shader));
    am.add_loaded(std::move(absolute_path), id);
    return id;
}