#pragma once

#include <string>
#include <cstdint>
#include <functional>

#include <glad/glad.h>  // Import glad first
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <AL/alc.h>

class Application;

class Window {
public:
    bool create(const std::string& title, int32_t width, int32_t height);
    void show() const;
    void poll_events() const;
    void swap_buffers() const;
    bool should_close() const;
    const glm::ivec2& size() const;
    double time() const;
    void set_input_mode(int32_t mode, int32_t value) const;
    void set_key_callback(std::function<void(int32_t key, int32_t scancode, int32_t action, int32_t mods)> cb);
    void set_mouse_button_callback(std::function<void(int32_t button, int32_t action, int32_t mods)> cb);
    void set_cursor_pos_callback(std::function<void(double xpos, double ypos)> cb);
    void set_scroll_callback(std::function<void(double xoffset, double yoffset)> cb);
    void set_char_callback(std::function<void(uint32_t codepoint)> cb);
    void close() const;
    void destroy();

    std::function<void(int32_t, int32_t, int32_t, int32_t)> m_key_callback;
    std::function<void(int32_t, int32_t, int32_t)> m_mouse_button_callback;
    std::function<void(double, double)> m_cursor_pos_callback;
    std::function<void(double, double)> m_scroll_callback;
    std::function<void(uint32_t)> m_char_callback;

private:
    GLFWwindow* m_handle = nullptr;
    glm::ivec2 m_size{0};

    static void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height);
    static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
    static void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void char_callback(GLFWwindow* window, uint32_t codepoint);
};
