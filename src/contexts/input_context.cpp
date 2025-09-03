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
    m_events.reserve(256);
    m_axis_cache.reserve(64);
    m_action_down_cache.reserve(64);
    m_action_pressed_cache.reserve(64);
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

    Event ev = {.type = (is_down ? EventType::KeyDown : EventType::KeyUp),
                .timestamp = m_time,
                .payload = KeyPayload{.key = key, .mods = mods}};
    emit_event(ev);
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

    Event ev = {.type = (is_down ? EventType::MouseDown : EventType::MouseUp),
                .timestamp = m_time,
                .payload = MouseButtonPayload{.btn = button}};
    emit_event(ev);
}

void InputContext::on_cursor_pos(double xpos, double ypos) {
    m_mouse_pos = glm::dvec2(xpos, ypos);

    // Systems can read mouse_delta() after
    Event ev = {.type = EventType::MouseMove, .timestamp = m_time, .payload = MouseMovePayload{.dx = 0.0, .dy = 0.0}};
    emit_event(ev);
}

void InputContext::on_scroll(double /*xoffset*/, double yoffset) {
    m_scroll_accum += yoffset;

    Event ev = {.type = EventType::Scroll, .timestamp = m_time, .payload = MouseScrollPayload{.offset = yoffset}};
    emit_event(ev);
}

void InputContext::on_char(uint32_t codepoint) {
    // emit a keydown-like character event
    Event ev = {.type = EventType::KeyDown,
                .timestamp = m_time,
                .payload = KeyPayload{.key = static_cast<Key>(codepoint), .mods = 0}};
    emit_event(ev);
}

void InputContext::on_action_pressed(const std::string& name, ActionCallback cb) {
    m_action_pressed_callbacks[name].push_back(std::move(cb));
}

void InputContext::begin_frame(double current_time) {
    m_time = current_time;

    // Compute mouse delta (based on last frame's prev pos)
    m_mouse_delta = m_mouse_pos - m_prev_mouse_pos;
    // Keep prev mouse for next frame
    m_prev_mouse_pos = m_mouse_pos;

    // Capture scroll for this frame and reset accumulator
    m_scroll_delta = m_scroll_accum;
    m_scroll_accum = 0.0;

    // Evaluate actions/axes based on current raw state
    std::unordered_map<std::string, bool> prev_down;
    prev_down.reserve(m_action_down_cache.size());
    for (auto const& kv : m_action_down_cache) {
        prev_down[kv.first] = kv.second;
    }

    // Evaluate each registered action
    m_axis_cache.clear();
    for (auto const& kv : m_actions) {
        const std::string& name = kv.first;
        const Action& a = kv.second;

        // Evaluate bindings
        float axis_accum = 0.0f;
        bool any_down = false;

        for (const Binding& b : a.bindings) {
            float contrib = eval_binding(b);
            axis_accum += contrib;

            if (std::abs(contrib) >= a.threshold) {
                any_down = true;
            }
        }

        // Clamp axis to [-1,1]
        axis_accum = std::max(-1.0f, std::min(1.0f, axis_accum));
        m_axis_cache[name] = axis_accum;

        // Determine boolean down for this action
        bool down = any_down || (std::abs(axis_accum) >= a.threshold);
        m_action_down_cache[name] = down;

        bool prev = false;
        auto itp = prev_down.find(name);
        if (itp != prev_down.end()) {
            prev = itp->second;
        }
        m_action_pressed_cache[name] = (down && !prev);
    }

    for (const auto& kv : m_action_pressed_cache) {
        const std::string& action_name = kv.first;
        bool pressed = kv.second;
        if (!pressed) {
            continue;  // only emit on press
        }

        // Invoke callbacks
        auto it = m_action_pressed_callbacks.find(action_name);
        if (it != m_action_pressed_callbacks.end()) {
            for (auto& cb : it->second) {
                cb();
            }
        }

        Event ev;
        ev.type = EventType::ActionTriggered;
        ev.timestamp = m_time;

        // Get axis value (if any) or 1.0 for a simple button press
        float value = 0.0f;
        auto ait = m_axis_cache.find(action_name);
        value = ait != m_axis_cache.end() ? ait->second : 1.0f;

        ev.payload = ActionPayload{.action = action_name, .value = value};
        emit_event(ev);
    }

    // Advance key/button previous state copies (used by was_key_pressed/released)
    m_prev_keys = m_curr_keys;
    m_prev_mouse_btns = m_curr_mouse_btns;
}

void InputContext::end_frame() {
    m_action_pressed_cache.clear();
    m_events.clear();
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
    return m_curr_keys.test(static_cast<size_t>(k)) && !m_prev_keys.test(static_cast<size_t>(k));
}

bool InputContext::was_key_released(Key k) const {
    if (k < 0 || k >= static_cast<int32_t>(MAX_KEYS)) {
        return false;
    }
    return !m_curr_keys.test(static_cast<size_t>(k)) && m_prev_keys.test(static_cast<size_t>(k));
}

bool InputContext::is_mouse_button_down(MouseButton b) const {
    if (b < 0 || b >= 32) {
        return false;
    }
    return m_curr_mouse_btns.test(static_cast<size_t>(b));
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

float InputContext::get_axis(const std::string& axis_name) const {
    auto it = m_axis_cache.find(axis_name);
    if (it == m_axis_cache.end()) {
        return 0.0f;
    }
    return it->second;
}

bool InputContext::get_action_down(const std::string& name) const {
    auto it = m_action_down_cache.find(name);
    if (it == m_action_down_cache.end()) {
        return false;
    }
    return it->second;
}

bool InputContext::get_action_pressed(const std::string& name) const {
    auto it = m_action_pressed_cache.find(name);
    if (it == m_action_pressed_cache.end()) {
        return false;
    }
    return it->second;
}

void InputContext::emit_event(const Event& ev) {
    m_events.push_back(ev);
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
