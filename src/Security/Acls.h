//
// Created by sathya on 25.02.18.
//

#ifndef LIBHOMEGEAR_BASE_ACLS_H
#define LIBHOMEGEAR_BASE_ACLS_H

#include "Acl.h"
#include "../BaseLib.h"

namespace BaseLib
{

class SharedObjects;

namespace Security
{

class Acls
{
private:
    BaseLib::SharedObjects* _bl = nullptr;
    std::mutex _aclsMutex;
    std::vector<PAcl> _acls;
public:
    Acls(BaseLib::SharedObjects* bl);
    ~Acls();

    void clear();
    bool fromUser(std::string& userName);
    bool fromGroups(std::vector<uint64_t>& groupIds);
};
typedef std::shared_ptr<Acls> PAcls;

}
}

#endif
