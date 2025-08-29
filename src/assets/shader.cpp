#include "assets/shader.h"
#include "core/log.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdint>

std::shared_ptr<Shader> Shader::create_fallback() {
    return std::make_shared<Shader>();
}

std::optional<std::shared_ptr<Shader>> Shader::load_from_file(const std::string& path) {
    std::ifstream v_file, f_file;

    // Ensure ifstream objects can throw exceptions:
    v_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::string v_code, f_code;
    try {
        // Open files
        v_file.open(path + "s.vert");
        f_file.open(path + "s.frag");

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
        ERR("[Shader] Error while reading shader file: " << e.what());
        return std::nullopt;
    }

    // Convert into a c-style string
    const char* v_code_c = v_code.c_str();
    const char* f_code_c = f_code.c_str();

    uint32_t vertex, fragment;

    // Vertex
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_code_c, NULL);
    glCompileShader(vertex);
    if (!Shader::check_compile_errors(vertex, "Vertex shader")) {
        return std::nullopt;
    }

    // Fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_code_c, NULL);
    glCompileShader(fragment);
    if (!Shader::check_compile_errors(fragment, "Fragment shader")) {
        return std::nullopt;
    }

    // Create program and attach shaders
    auto shader = std::make_shared<Shader>();
    shader->m_program_id = glCreateProgram();
    glAttachShader(shader->m_program_id, vertex);
    glAttachShader(shader->m_program_id, fragment);
    glLinkProgram(shader->m_program_id);
    if (!Shader::check_compile_errors(shader->m_program_id, "Program")) {
        shader->destroy();
        return std::nullopt;
    }

    shader->m_full_path = std::string(path);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Cache uniform locations
    shader->get_active_uniforms();

    return shader;
}

const std::string& Shader::full_path() {
    return m_full_path;
}

void Shader::use() const {
    glUseProgram(m_program_id);
}

int32_t Shader::get_uniform_location(const std::string& name) const {
    auto it = m_uniform_locations.find(name);
    return it == m_uniform_locations.end() ? -1 : it->second;
}

void Shader::set_bool(const std::string& name, bool value) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform1i(loc, (int32_t)value);
}

void Shader::set_int(const std::string& name, int32_t value) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform1i(loc, value);
}

void Shader::set_float(const std::string& name, float value) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform1f(loc, value);
}

void Shader::set_vec_3f(const std::string& name, const GLfloat* ptr) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniform3fv(loc, 1, ptr);
}

void Shader::set_vec_3f(const std::string& name, const glm::vec3& vec) const {
    set_vec_3f(name, glm::value_ptr(vec));
}

void Shader::set_matrix_3f(const std::string& name, const GLfloat* ptr) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniformMatrix3fv(loc, 1, GL_FALSE, ptr);
}

void Shader::set_matrix_3f(const std::string& name, const glm::mat3& mat) const {
    set_matrix_3f(name, glm::value_ptr(mat));
}

void Shader::set_matrix_4f(const std::string& name, const GLfloat* ptr) const {
    int32_t loc = get_uniform_location(name);
    if (loc == -1) {
        return;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, ptr);
}

void Shader::set_matrix_4f(const std::string& name, const glm::mat4& mat) const {
    set_matrix_4f(name, glm::value_ptr(mat));
}

void Shader::destroy() {
    glDeleteProgram(m_program_id);
    m_program_id = 0;
    m_uniform_locations.clear();
}

std::ostream& Shader::print(std::ostream& os) const {
    return os << "Shader(program_id: " << m_program_id << ", path: " << m_full_path << ")";
}

bool Shader::check_compile_errors(uint32_t shader, const std::string& type) {
    int32_t success;
    char info_log[1024];
    if (type != "Program") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, info_log);
            ERR("[Shader] compilation error for " << type << ":\n\t" << info_log)
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, info_log);
            ERR("[Shader] linking error for " << type << ":\n\t" << info_log);
            return false;
        }
    }

    return true;
}

void Shader::get_active_uniforms() {
    GLint count;
    glGetProgramiv(m_program_id, GL_ACTIVE_UNIFORMS, &count);

    for (int32_t i = 0; i < count; i++) {
        char name[128];
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(m_program_id, (GLuint)i, sizeof(name), &length, &size, &type, name);

        GLint loc = glGetUniformLocation(m_program_id, name);
        m_uniform_locations[name] = loc;
    }
}