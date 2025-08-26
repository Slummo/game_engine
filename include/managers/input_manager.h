#pragma once

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

enum class EventType { KeyDown, KeyUp, MouseDown, MouseUp, MouseMove, Scroll, ActionTriggered };

struct KeyPayload {
    Key key;
    int mods;
};

struct MouseButtonPayload {
    MouseButton btn;
};

struct MouseMovePayload {
    double dx;
    double dy;
};

struct MouseScrollPayload {
    double offset;
};

struct ActionPayload {
    std::string action;
    float value;
};

using EventPayload =
    std::variant<std::monostate, KeyPayload, MouseButtonPayload, MouseMovePayload, MouseScrollPayload, ActionPayload>;

struct Event {
    EventType type;
    double timestamp;
    EventPayload payload;
};

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

class InputManager {
public:
    InputManager();

    // Called by GLFW callbacks
    void on_key(int key, int /*scancode*/, int action, int mods);
    void on_mouse_button(int button, int action, int /*mods*/);
    void on_cursor_pos(double xpos, double ypos);
    void on_scroll(double /*xoffset*/, double yoffset);
    void on_char(unsigned int codepoint);

    // To use a callbacks system
    void on_action_pressed(const std::string& name, ActionCallback cb);

    // Call each frame to update internal state
    void begin_frame(double current_time);
    void end_frame();

    // Polling API
    bool is_key_down(Key k) const;
    bool was_key_pressed(Key k) const;  // pressed this frame
    bool was_key_released(Key k) const;

    bool is_mouse_button_down(MouseButton b) const;
    glm::dvec2 mouse_pos() const;
    glm::dvec2 mouse_delta() const;
    double scroll_delta() const;  // since last frame

    void set_mouse_pos(glm::dvec2 pos);
    void set_mouse_delta(glm::dvec2 delta);

    // Action/axis interface
    void register_action(const Action& a);
    void register_action(const std::string& name, std::vector<Binding> bindings);
    void register_action(const std::string& name, InputType type, int key, int mod = 0);
    void unregister_action(const std::string& name);
    float get_axis(const std::string& axis_name) const;      // -1..1
    bool get_action_down(const std::string& name) const;     // held
    bool get_action_pressed(const std::string& name) const;  // pressed this frame

private:
    static constexpr size_t MAX_KEYS = 1024;
    std::bitset<MAX_KEYS> m_curr_keys;
    std::bitset<MAX_KEYS> m_prev_keys;
    int32_t m_curr_mods = 0;

    std::bitset<32> m_curr_mouse_btns;
    std::bitset<32> m_prev_mouse_btns;

    glm::dvec2 m_mouse_pos{0.0};
    glm::dvec2 m_prev_mouse_pos{0.0};
    glm::dvec2 m_mouse_delta{0.0};
    double m_scroll_delta = 0.0;
    double m_scroll_accum = 0.0;

    double m_time = 0.0;

    std::unordered_map<std::string, Action> m_actions;
    std::unordered_map<std::string, std::vector<ActionCallback>> m_action_pressed_callbacks;

    // Cached evaluation results
    std::unordered_map<std::string, float> m_axis_cache;
    std::unordered_map<std::string, bool> m_action_down_cache;
    std::unordered_map<std::string, bool> m_action_pressed_cache;

    // per-frame event queue
    std::vector<Event> m_events;

    // helpers
    void emit_event(const Event& ev);
    float eval_binding(const Binding& b) const;
};