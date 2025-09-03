#pragma once

#include "contexts/icontext.h"

#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <bitset>
#include <unordered_map>
#include <functional>

using Key = int32_t;
using MouseButton = int32_t;

enum struct InputType { Key, MouseButton };

// What input triggers an Action
struct Binding {
    Binding(InputType type, int32_t code, int32_t mods = 0, float scale = 1.0f, bool invert = false);

    InputType type;      // type of input
    int32_t code;        // keycode / btn or axis id
    int32_t mods = 0;    // modifier
    float scale = 1.0f;  // for axis
    bool invert = false;
};

// Named mapping to a set of binding
struct Action {
    Action(std::string name, float threshold = 0.5f);
    void add_binding(Binding binding);

    std::string name;
    std::vector<Binding> bindings;
    float threshold = 0.5f;
};

using ActionCallback = std::function<void()>;

class InputContext : public IContext {
public:
    InputContext();

    // GLFW callbacks
    void on_key(int32_t key, int32_t /*scancode*/, int32_t action, int32_t mods);
    void on_mouse_button(int32_t button, int32_t action, int32_t /*mods*/);
    void on_cursor_pos(double xpos, double ypos);
    void on_scroll(double /*xoffset*/, double yoffset);
    void on_char(uint32_t codepoint);

    // Call each frame to update internal state
    void begin_frame();
    void end_frame();

    // Actions registration

    void register_action(const Action& a);
    void register_action(const std::string& name, std::vector<Binding> bindings);
    void register_action(const std::string& name, InputType type, int32_t key, int32_t mod = 0);
    void unregister_action(const std::string& name);

    // --- Polling API (is_*_down = held, was_*_pressed = pressed this frame, was_*_released = released this frame) ---

    bool is_key_down(Key k) const;
    bool was_key_pressed(Key k) const;
    bool was_key_released(Key k) const;
    bool is_mouse_button_down(MouseButton b) const;
    bool was_mouse_button_pressed(MouseButton b) const;
    bool was_mouse_button_released(MouseButton b) const;
    bool is_action_down(const std::string& name) const;
    bool was_action_pressed(const std::string& name) const;
    bool was_action_released(const std::string& name) const;

    // --- Callback API ---

    void on_action_pressed(const std::string& name, ActionCallback cb);
    void on_action_released(const std::string& name, ActionCallback cb);

    // Getters and setters

    glm::dvec2 mouse_pos() const;
    glm::dvec2 mouse_delta() const;
    double scroll_delta() const;  // since last frame
    void set_mouse_pos(glm::dvec2 pos);
    void set_mouse_delta(glm::dvec2 delta);
    float axis(const std::string& axis_name) const;  // -1..1

private:
    // Keys
    static constexpr size_t MAX_KEYS = 1024;
    std::bitset<MAX_KEYS> m_curr_keys;
    std::bitset<MAX_KEYS> m_prev_keys;
    int32_t m_curr_mods = 0;

    // Mouse buttons
    std::bitset<32> m_curr_mouse_btns;
    std::bitset<32> m_prev_mouse_btns;

    // Mouse position and scroll
    glm::dvec2 m_mouse_pos{0.0};
    glm::dvec2 m_prev_mouse_pos{0.0};
    glm::dvec2 m_mouse_delta{0.0};
    double m_scroll_delta = 0.0;
    double m_scroll_accum = 0.0;

    // Actions
    std::unordered_map<std::string, Action> m_actions;
    std::unordered_map<std::string, float> m_axis_cache;
    std::unordered_map<std::string, bool> m_action_down_cache;
    std::unordered_map<std::string, bool> m_action_pressed_cache;
    std::unordered_map<std::string, bool> m_action_released_cache;

    // Callbacks
    std::unordered_map<std::string, std::vector<ActionCallback>> m_action_pressed_callbacks;
    std::unordered_map<std::string, std::vector<ActionCallback>> m_action_released_callbacks;

    // Helper
    float eval_binding(const Binding& b) const;
};