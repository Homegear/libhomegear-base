//
// Created by sathya on 03.09.19.
//

#ifndef LIBHOMEGEAR_BASE_ENVIRONMENT_H
#define LIBHOMEGEAR_BASE_ENVIRONMENT_H

#include <mutex>

class Environment
{
private:
    static std::mutex _environmentMutex;
public:
    Environment() = delete;

    static std::string get(const std::string& name);
    static void set(const std::string& name, const std::string& value);
};


#endif //LIBHOMEGEAR_BASE_ENVIRONMENT_H
