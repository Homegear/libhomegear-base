//
// Created by sathya on 17.09.19.
//

#ifndef LIBHOMEGEAR_BASE_ROLE_H
#define LIBHOMEGEAR_BASE_ROLE_H

#include <cstdint>

namespace BaseLib
{

enum class RoleDirection
{
    undefined = -1,
    input = 0,
    output = 1,
    both = 2
};

struct Role
{
    Role() = default;

    Role(uint64_t id, RoleDirection direction, bool invert) : id(id), direction(direction), invert(invert) {}

    uint64_t id;
    RoleDirection direction = RoleDirection::both;
    bool invert = false;
};

}
#endif //LIBHOMEGEAR_BASE_ROLE_H
