#pragma once

#include "contexts/icontext.h"
#include "core/types/contact.h"

#include <vector>

struct CollisionContext : public IContext {
    std::vector<Contact> contacts;
};