#include "core/window.h"
#include "core/application.h"
#include "core/log.h"

bool Window::create(Application* app_ptr, int32_t width, int32_t height, const std::string& title) {
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
    glfwSetWindowUserPointer(m_handle, app_ptr);

    // Load OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ERR("[Application] Failed to init GLAD");
        return false;
    }

    // Set viewport
    int32_t fb_w, fb_h;
    get_framebuffer_size(&fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);

    // Set callbacks
    glfwSetFramebufferSizeCallback(
        m_handle, [](GLFWwindow* /*w*/, int32_t width, int32_t height) { glViewport(0, 0, width, height); });

    glfwSetKeyCallback(m_handle, [](GLFWwindow* w, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(w));
        app->get_input_context().on_key(key, scancode, action, mods);
    });

    glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* w, int32_t button, int32_t action, int32_t mods) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(w));
        app->get_input_context().on_mouse_button(button, action, mods);
    });

    glfwSetCursorPosCallback(m_handle, [](GLFWwindow* w, double xpos, double ypos) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(w));
        app->get_input_context().on_cursor_pos(xpos, ypos);
    });

    glfwSetScrollCallback(m_handle, [](GLFWwindow* w, double xoffset, double yoffset) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(w));
        app->get_input_context().on_scroll(xoffset, yoffset);
    });

    glfwSetCharCallback(m_handle, [](GLFWwindow* w, uint32_t codepoint) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(w));
        app->get_input_context().on_char(codepoint);
    });

    m_width = width;
    m_height = height;

    // OpenAL
    ALCdevice* device = alcOpenDevice(NULL);
    if (device) {
        ALCcontext* context = alcCreateContext(device, NULL);
        alcMakeContextCurrent(context);
    }

    return true;
}

void Window::show() const {
    if (m_handle) {
        glfwShowWindow(m_handle);
    }
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

glm::ivec2 Window::get_size() const {
    return glm::ivec2(m_width, m_height);
}

void Window::get_framebuffer_size(int32_t* x, int32_t* y) const {
    glfwGetFramebufferSize(m_handle, x, y);
}

double Window::get_time() const {
    return glfwGetTime();
}
void Window::set_input_mode(int32_t mode, int32_t value) const {
    glfwSetInputMode(m_handle, mode, value);
}

void Window::close() const {
    glfwSetWindowShouldClose(m_handle, 1);
}

void Window::destroy() {
    // glfw
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
    glfwTerminate();
    m_handle = nullptr;

    // OpenAL
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}
