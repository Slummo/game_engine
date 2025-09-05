#include "contexts/input_context.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <cstring>
#include <cassert>

Binding::Binding(InputType type, int32_t code, int32_t mods, float scale, bool invert)
    : type(type), code(code), mods(mods), scale(scale), invert(invert) {
}

Action::Action(std::string name, float threshold) : name(name), threshold(threshold) {
}

void Action::add_binding(Binding binding) {
    bindings.push_back(std::move(binding));
}

InputContext::InputContext() {
    m_axis_cache.reserve(64);
    m_action_down_cache.reserve(64);
    m_action_pressed_cache.reserve(64);
}

void InputContext::link_callbacks(Window& win) {
    win.set_key_callback(
        [&](int32_t key, int32_t scancode, int32_t action, int32_t mods) { on_key(key, scancode, action, mods); });
    win.set_mouse_button_callback(
        [&](int32_t button, int32_t action, int32_t mods) { on_mouse_button(button, action, mods); });
    win.set_cursor_pos_callback([&](double xpos, double ypos) { on_cursor_pos(xpos, ypos); });
    win.set_scroll_callback([&](double xoffset, double yoffset) { on_scroll(xoffset, yoffset); });
    win.set_char_callback([&](uint32_t codepoint) { on_char(codepoint); });
}

void InputContext::on_key(int32_t key, int32_t /*scancode*/, int32_t action, int32_t mods) {
    if (key < 0 || key >= static_cast<int32_t>(MAX_KEYS)) {
        return;
    }

    bool is_down = (action != 0);  // GLFW_PRESS or GLFW_REPEAT = non-zero, GLFW_RELEASE = 0
    if (is_down) {
        m_curr_keys.set(static_cast<size_t>(key));
    } else {
        m_curr_keys.reset(static_cast<size_t>(key));
    }

    m_curr_mods = mods;
}

void InputContext::on_mouse_button(int32_t button, int32_t action, int32_t /*mods*/) {
    if (button < 0 || button >= 32) {
        return;
    }
    bool is_down = (action != 0);
    if (is_down) {
        m_curr_mouse_btns.set(static_cast<size_t>(button));
    } else {
        m_curr_mouse_btns.reset(static_cast<size_t>(button));
    }
}

void InputContext::on_cursor_pos(double xpos, double ypos) {
    m_mouse_pos = glm::dvec2(xpos, ypos);
}

void InputContext::on_scroll(double /*xoffset*/, double yoffset) {
    m_scroll_accum += yoffset;
}

void InputContext::on_char(uint32_t /*codepoint*/) {
    // TODO
}

void InputContext::begin_frame() {
    // Mouse
    m_mouse_delta = m_mouse_pos - m_prev_mouse_pos;
    m_prev_mouse_pos = m_mouse_pos;
    m_scroll_delta = m_scroll_accum;
    m_scroll_accum = 0.0;

    // Actions
    for (auto& [name, action] : m_actions) {
        // Evaluate bindings
        float axis_accum = 0.0f;
        bool any_down = false;

        for (const Binding& b : action.bindings) {
            float contrib = eval_binding(b);
            axis_accum += contrib;

            if (std::abs(contrib) >= action.threshold) {
                any_down = true;
            }
        }

        // Clamp axis to [-1,1]
        axis_accum = std::max(-1.0f, std::min(1.0f, axis_accum));
        m_axis_cache[name] = axis_accum;

        // Determine down state for this action
        bool down = any_down || (std::abs(axis_accum) >= action.threshold);

        // Edge detection
        bool prev_down = m_action_down_cache[name];
        bool pressed = down && !prev_down;
        bool released = !down && prev_down;

        m_action_down_cache[name] = down;
        m_action_pressed_cache[name] = pressed;
        m_action_released_cache[name] = released;

        // Trigger callbacks
        if (pressed) {
            auto it = m_action_pressed_callbacks.find(name);
            if (it != m_action_pressed_callbacks.end()) {
                for (auto& cb : it->second) {
                    cb();
                }
            }
        }
        if (released) {
            auto it = m_action_released_callbacks.find(name);
            if (it != m_action_released_callbacks.end()) {
                for (auto& cb : it->second) {
                    cb();
                }
            }
        }
    }

    // Advance previous state for edge detection
    m_prev_keys = m_curr_keys;
    m_prev_mouse_btns = m_curr_mouse_btns;
}

void InputContext::end_frame() {
}

void InputContext::register_action(const Action& a) {
    m_actions.insert_or_assign(a.name, a);
}

void InputContext::register_action(const std::string& name, std::vector<Binding> bindings) {
    Action action(name);
    for (auto& b : bindings) {
        action.add_binding(b);
    }
    register_action(action);
}

void InputContext::register_action(const std::string& name, InputType type, int32_t key, int32_t mod) {
    register_action(name, {Binding(type, key, mod)});
}

void InputContext::unregister_action(const std::string& name) {
    m_actions.erase(name);
    m_axis_cache.erase(name);
    m_action_down_cache.erase(name);
    m_action_pressed_cache.erase(name);
}

bool InputContext::is_key_down(Key k) const {
    if (k < 0 || k >= static_cast<int32_t>(MAX_KEYS)) {
        return false;
    }
    return m_curr_keys.test(static_cast<size_t>(k));
}

bool InputContext::was_key_pressed(Key k) const {
    if (k < 0 || k >= static_cast<int32_t>(MAX_KEYS)) {
        return false;
    }
    size_t n = static_cast<size_t>(k);
    return m_curr_keys.test(n) && !m_prev_keys.test(n);
}

bool InputContext::was_key_released(Key k) const {
    if (k < 0 || k >= static_cast<int32_t>(MAX_KEYS)) {
        return false;
    }
    size_t n = static_cast<size_t>(k);
    return !m_curr_keys.test(n) && m_prev_keys.test(n);
}

bool InputContext::is_mouse_button_down(MouseButton b) const {
    if (b < 0 || b >= 32) {
        return false;
    }
    return m_curr_mouse_btns.test(static_cast<size_t>(b));
}

bool InputContext::was_mouse_button_pressed(MouseButton b) const {
    if (b < 0 || b >= 32) {
        return false;
    }
    size_t n = static_cast<size_t>(b);
    return m_curr_mouse_btns.test(n) && !m_prev_mouse_btns.test(n);
}

bool InputContext::was_mouse_button_released(MouseButton b) const {
    if (b < 0 || b >= 32) {
        return false;
    }
    size_t n = static_cast<size_t>(b);
    return !m_curr_mouse_btns.test(n) && m_prev_mouse_btns.test(n);
}

bool InputContext::is_action_down(const std::string& name) const {
    auto it = m_action_down_cache.find(name);
    return it != m_action_down_cache.end() ? it->second : false;
}

bool InputContext::was_action_pressed(const std::string& name) const {
    auto it = m_action_pressed_cache.find(name);
    return it != m_action_pressed_cache.end() ? it->second : false;
}

bool InputContext::was_action_released(const std::string& name) const {
    auto it = m_action_released_cache.find(name);
    return it != m_action_released_cache.end() ? it->second : false;
}

void InputContext::on_action_pressed(const std::string& name, ActionCallback cb) {
    m_action_pressed_callbacks[name].push_back(std::move(cb));
}

void InputContext::on_action_released(const std::string& name, ActionCallback cb) {
    m_action_released_callbacks[name].push_back(std::move(cb));
}

glm::dvec2 InputContext::mouse_pos() const {
    return m_mouse_pos;
}

glm::dvec2 InputContext::mouse_delta() const {
    return m_mouse_delta;
}

double InputContext::scroll_delta() const {
    return m_scroll_delta;
}

void InputContext::set_mouse_pos(glm::dvec2 pos) {
    m_mouse_pos = pos;
}

void InputContext::set_mouse_delta(glm::dvec2 delta) {
    m_mouse_delta = delta;
}

float InputContext::axis(const std::string& axis_name) const {
    auto it = m_axis_cache.find(axis_name);
    if (it == m_axis_cache.end()) {
        return 0.0f;
    }
    return it->second;
}

float InputContext::eval_binding(const Binding& b) const {
    switch (b.type) {
        case InputType::Key: {
            bool key_down = is_key_down(b.code);
            bool mods_match = b.mods == 0 || ((m_curr_mods & b.mods) == b.mods);

            if (key_down && mods_match) {
                return (b.invert ? -b.scale : b.scale);
            }
            return 0.0f;
        }
        case InputType::MouseButton: {
            bool btn_down = is_mouse_button_down(b.code);
            if (btn_down) {
                return (b.invert ? -b.scale : b.scale);
            }
            return 0.0f;
        }
        default: {
            return 0.0f;
        }
    }
}
