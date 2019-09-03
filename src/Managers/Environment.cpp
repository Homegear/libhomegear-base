//
// Created by sathya on 03.09.19.
//

#include "Environment.h"

std::string Environment::get(const std::string& name)
{
    std::lock_guard<std::mutex> environmentGuard(_environmentMutex);
    auto result = getenv(name.c_str());
    if(result) return std::string(result);

    return "";
}

void Environment::set(const std::string& name, const std::string& value)
{
    std::lock_guard<std::mutex> environmentGuard(_environmentMutex);
    setenv(name.c_str(), value.c_str(), 1);
}
