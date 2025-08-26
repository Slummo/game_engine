#pragma once

#include "core/log.h"

class IAsset : public Printable {
public:
    virtual ~IAsset() = default;
};