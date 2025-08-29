#pragma once

#include <cstdint>
#include <cxxabi.h>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <glm/glm.hpp>

using EntityID = uint32_t;
using AssetID = uint64_t;

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

template <typename T>
std::string readable_type_name() {
    int32_t status;
    char* demangled = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string result = (status == 0) ? demangled : typeid(T).name();
    free(demangled);
    return result;
}