#pragma once

#include <string>
#include <cxxabi.h>
#include <typeinfo>

template <typename T>
std::string readable_type_name() {
    int32_t status;
    char* demangled = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string result = (status == 0) ? demangled : typeid(T).name();
    free(demangled);
    return result;
}