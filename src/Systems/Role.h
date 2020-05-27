//
// Created by sathya on 17.09.19.
//

#ifndef LIBHOMEGEAR_BASE_ROLE_H
#define LIBHOMEGEAR_BASE_ROLE_H

#include <cstdint>

namespace BaseLib
{

enum class RoleLevel
{
    undefined = -1,
    mainCategory = 0,
    subCategory = 1,
    role = 2
};

enum class RoleDirection
{
    undefined = -1,
    input = 0,
    output = 1,
    both = 2
};

struct RoleScaleInfo
{
    bool valueSet = false;
    double valueMin = 0;
    double valueMax = 0;
    double scaleMin = 0;
    double scaleMax = 0;
};

struct Role
{
    Role() = default;

    Role(uint64_t id, RoleDirection direction, bool invert, bool scale, RoleScaleInfo scaleInfo) : id(id), direction(direction), invert(invert), scale(scale), scaleInfo(scaleInfo)
    {
        if ((id / 10000) * 10000 == id) level = RoleLevel::mainCategory;
        else if ((id / 100) * 100 == id) level = RoleLevel::subCategory;
        else level = RoleLevel::role;
    }

    uint64_t id;
    RoleLevel level = RoleLevel::undefined;
    RoleDirection direction = RoleDirection::both;
    bool invert = false;
    bool scale = false;
    RoleScaleInfo scaleInfo;
};

}
#endif //LIBHOMEGEAR_BASE_ROLE_H
