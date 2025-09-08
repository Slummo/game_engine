#include "core/window.h"
#include "core/application.h"
#include "core/log.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

bool Window::create(const std::string& title, int32_t width, int32_t height) {
    // Init GLFW
    if (!glfwInit()) {
        ERR("[Window] Failed to init GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Create the actual window
    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        ERR("[Window] Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_handle);

    // Load OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ERR("[Application] Failed to init GLAD");
        return false;
    }

    // Set initial size
    glfwGetFramebufferSize(m_handle, &m_size.x, &m_size.y);
    glViewport(0, 0, m_size.x, m_size.y);

    // Store this instance
    glfwSetWindowUserPointer(m_handle, this);

    // Set resize callback
    glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);

    // Set all other callbacks
    glfwSetKeyCallback(m_handle, key_callback);
    glfwSetMouseButtonCallback(m_handle, mouse_button_callback);
    glfwSetCursorPosCallback(m_handle, cursor_pos_callback);
    glfwSetScrollCallback(m_handle, scroll_callback);
    glfwSetCharCallback(m_handle, char_callback);

    // Init OpenAL
    ALCdevice* device = alcOpenDevice(nullptr);
    if (device) {
        ALCcontext* context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // Init ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_handle, true);
    ImGui_ImplOpenGL3_Init("#version 330");  // your GLSL version

    return true;
}

void Window::show() const {
    glfwShowWindow(m_handle);
}

void Window::poll_events() const {
    glfwPollEvents();
}

void Window::swap_buffers() const {
    glfwSwapBuffers(m_handle);
}

bool Window::should_close() const {
    return glfwWindowShouldClose(m_handle);
}

const glm::ivec2& Window::size() const {
    return m_size;
}

double Window::time() const {
    return glfwGetTime();
}

void Window::set_input_mode(int32_t mode, int32_t value) const {
    glfwSetInputMode(m_handle, mode, value);
}

void Window::set_key_callback(std::function<void(int32_t key, int32_t scancode, int32_t action, int32_t mods)> cb) {
    m_key_callback = cb;
}

void Window::set_mouse_button_callback(std::function<void(int32_t button, int32_t action, int32_t mods)> cb) {
    m_mouse_button_callback = cb;
}

void Window::set_cursor_pos_callback(std::function<void(double xpos, double ypos)> cb) {
    m_cursor_pos_callback = cb;
}

void Window::set_scroll_callback(std::function<void(double xoffset, double yoffset)> cb) {
    m_scroll_callback = cb;
}

void Window::set_char_callback(std::function<void(uint32_t codepoint)> cb) {
    m_char_callback = cb;
}

void Window::close() const {
    glfwSetWindowShouldClose(m_handle, 1);
}

Window::~Window() {
    // ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // OpenAL
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    // GLFW
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
    glfwTerminate();
    m_handle = nullptr;
}

void Window::framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!win) {
        return;
    }

    win->m_size.x = width;
    win->m_size.y = height;
    glViewport(0, 0, width, height);
}

void Window::key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_key_callback) {
        win->m_key_callback(key, scancode, action, mods);
    }
}

void Window::mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_mouse_button_callback) {
        win->m_mouse_button_callback(button, action, mods);
    }
}

void Window::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_cursor_pos_callback) {
        win->m_cursor_pos_callback(xpos, ypos);
    }
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_scroll_callback) {
        win->m_scroll_callback(xoffset, yoffset);
    }
}

void Window::char_callback(GLFWwindow* window, uint32_t codepoint) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_char_callback) {
        win->m_char_callback(codepoint);
    }
}
