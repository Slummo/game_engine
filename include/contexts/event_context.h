#pragma once

#include "contexts/icontext.h"
#include "events/events.h"

#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <typeindex>
#include <queue>

class EventContext : public IContext {
public:
    template <typename T>
        requires std::is_base_of_v<IEvent, T>
    using Callback = std::function<void(const T&)>;

    template <typename T>
        requires std::is_base_of_v<IEvent, T>
    void emit(const T& event) {
        m_queue.push(std::make_shared<T>(event));
    }

    template <typename T>
        requires std::is_base_of_v<IEvent, T>
    void subscribe(Callback<T> cb) {
        auto& callbacks = m_subscribers[typeid(T)];
        callbacks.push_back([cb](const std::shared_ptr<IEvent>& base_event) {
            auto* event = static_cast<T*>(base_event.get());
            cb(*event);
        });
    }

    void dispatch() {
        while (!m_queue.empty()) {
            std::shared_ptr<IEvent> event = m_queue.front();
            m_queue.pop();

            auto it = m_subscribers.find(typeid(*event));
            if (it != m_subscribers.end()) {
                for (auto& cb : it->second) {
                    cb(event);
                }
            }
        }
    }

private:
    std::queue<std::shared_ptr<IEvent>> m_queue;
    std::unordered_map<std::type_index, std::vector<std::function<void(std::shared_ptr<IEvent>)>>> m_subscribers;
};