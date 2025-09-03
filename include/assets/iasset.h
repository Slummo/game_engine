#pragma once

#include "core/log.h"
#include "core/types/id.h"

class IAsset : public Printable {
public:
    virtual ~IAsset() = default;
};