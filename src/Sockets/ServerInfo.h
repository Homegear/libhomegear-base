/* Copyright 2013-2019 Homegear GmbH
 *
 * libhomegear-base is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libhomegear-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libhomegear-base.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#ifndef SERVERINFO_H_
#define SERVERINFO_H_

#include "../Variable.h"

#include <memory>
#include <string>
#include <map>
#include <cstring>
#include <vector>
#include <unordered_set>

namespace BaseLib {

class SharedObjects;
class FileDescriptor;
typedef std::shared_ptr<FileDescriptor> PFileDescriptor;

namespace Rpc {

class ServerInfo {
 public:
  class Info {
   public:
    enum AuthType { undefined = 0, none = 1, basic = 2, cert = 4, session = 8, oauth2Local = 16 };

    Info() {
      interface = "::";
      contentPath = "/var/lib/homegear/www/";
    }
    virtual ~Info() {}
    int32_t index = -1;
    std::string name;
    std::string interface;
    int32_t port = -1;
    bool ssl = true;
    std::string caPath;
    std::string certPath;
    std::string keyPath;
    AuthType authType = AuthType::cert;
    std::unordered_set<uint64_t> validGroups;
    std::string contentPath;
    uint32_t contentPathPermissions = 360;
    std::string contentPathUser;
    std::string contentPathGroup;
    bool webServer = false;
    bool webSocket = false;
    AuthType websocketAuthType = AuthType::session;
    bool rpcServer = false;
    bool restServer = false;
    bool familyServer = false;
    int32_t cacheAssets = 2592000;
    std::string redirectTo;

    // Helpers
    PFileDescriptor socketDescriptor;

    // Mods
    std::map<std::string, std::vector<std::string>> modSettings;

    //Not settable
    std::string address;
    PVariable serializedInfo;

    /**
     * Serializes the whole object except modSettings.
     */
    PVariable serialize();
    void unserialize(PVariable data);
  };

  ServerInfo();
  explicit ServerInfo(BaseLib::SharedObjects *baseLib);
  ~ServerInfo() = default;
  void init(BaseLib::SharedObjects *baseLib);
  void load(const std::string &filename);

  int32_t count() { return _servers.size(); }
  std::shared_ptr<Info> get(int32_t index) { if (_servers.find(index) != _servers.end()) return _servers[index]; else return std::shared_ptr<Info>(); }
 private:
  BaseLib::SharedObjects *_bl = nullptr;
  std::map<int32_t, std::shared_ptr<Info>> _servers;

  void reset();
};

typedef std::shared_ptr<ServerInfo::Info> PServerInfo;

}
}
#endif
