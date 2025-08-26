#pragma once
#include <string>
#include <cstdint>
#include <glad/glad.h>  // Import glad first
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Application;

class Window {
public:
    Window() = default;

    bool create(Application* app_ptr, int32_t width, int32_t height, const std::string& title);
    void show() const;
    void poll_events() const;
    void swap_buffers() const;
    bool should_close() const;
    glm::ivec2 get_size() const;
    void get_framebuffer_size(int32_t* x, int32_t* y) const;
    double get_time() const;
    void set_input_mode(int32_t mode, int32_t value) const;
    void set_viewport(int32_t x, int32_t y) const;
    void set_wiremode(bool value) const;
    void close() const;
    void destroy();

private:
    GLFWwindow* m_handle = nullptr;
    int32_t m_width = 0;
    int32_t m_height = 0;
};
