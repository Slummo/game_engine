#pragma once

#include <cstdint>

using LayerMask = uint32_t;
namespace Layers {
constexpr LayerMask Default = 1 << 0;
constexpr LayerMask Ground = 1 << 1;
constexpr LayerMask Player = 1 << 2;
}  // namespace Layers