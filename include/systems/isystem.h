#pragma once

#include <string>
#include <cstdint>

struct Engine;

class ISystem {
public:
    virtual ~ISystem() = default;

    virtual void init(Engine&) {
    }
    virtual void update(Engine&) = 0;
    virtual void shutdown(Engine&) {
    }
};