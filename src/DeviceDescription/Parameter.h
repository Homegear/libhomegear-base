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

#ifndef DEVICEPARAMETER_H_
#define DEVICEPARAMETER_H_

#include "ParameterCast.h"
#include "Logical.h"
#include "Physical.h"
#include "../Systems/Role.h"
#include "UnitCode.h"
#include <string>
#include <set>

using namespace rapidxml;
using namespace BaseLib::DeviceDescription::ParameterCast;

namespace BaseLib {

class SharedObjects;

namespace DeviceDescription {

class Parameter;
class ParameterGroup;
typedef std::shared_ptr<ParameterGroup> PParameterGroup;

typedef std::shared_ptr<Parameter> PParameter;
typedef std::map<std::string, PParameter> Parameters;
typedef std::string ParameterRole;
typedef std::unordered_map<uint64_t, Role> ParameterRoles;

class Parameter : public std::enable_shared_from_this<Parameter> {
 public:
  class Packet {
   public:
    struct ConditionOperator {
      enum Enum { none, e, g, l, ge, le };
    };

    struct Type {
      enum Enum { none = 0, get = 1, set = 2, event = 3 };
    };

    std::string id;
    Type::Enum type = Type::none;
    std::vector<std::string> autoReset;
    std::pair<std::string, int32_t> delayedAutoReset;
    std::string responseId;
    ConditionOperator::Enum conditionOperator = ConditionOperator::none;
    int32_t conditionValue = -1;
    int32_t delay = -1;

    Packet() = default;
    virtual ~Packet() = default;

    bool checkCondition(int32_t value);
  };

  //Attributes
  std::string id;

  //Properties
  bool readable = true;
  bool writeable = true;
  bool readOnInit = false;
  bool transmitted = true;

  /**
   * Makes readonly variables writeable for addons (i. e. from device source code)
   */
  bool addonWriteable = true;

  bool password = false;
  bool visible = true;
  bool internal = false;
  bool parameterGroupSelector = false;
  bool service = false;
  bool serviceInverted = false;
  bool sticky = false;
  bool transform = false;
  bool isSignedSet = false;
  bool isSigned = false;

  /**
   * UI control to use for displaying this parameter.
   */
  std::string control;

  /**
   * The unit of the variable like "°C" or "km/h".
   */
  std::string unit;
  /**
   * The unit code (we use the BACnet codes)
   */
  UnitCode unit_code = UnitCode::kUndefined;
  bool mandatory = false;
  std::string formFieldType;
  int32_t formPosition = -1;
  std::string metadata;
  bool resetAfterRestart = false;
  int32_t priority = -1;

  /**
   * Deprecated. Remove beginning of 2021.
   * Visible on the HomeMatic CCU2.
   */
  bool ccu2Visible = true;

  /**
   * When parameters are linked together, logical changes of one parameter change the other to the same value.
   */
  std::string linkedParameter;
  Casts casts;
  ParameterRoles roles;

  //Elements
  std::shared_ptr<ILogical> logical;
  std::shared_ptr<IPhysical> physical;
  std::vector<std::shared_ptr<Packet>> getPackets;
  std::vector<std::shared_ptr<Packet>> setPackets;
  std::vector<std::shared_ptr<Packet>> eventPackets;

  //Helpers
  bool hasDelayedAutoResetParameters = false;

  explicit Parameter(BaseLib::SharedObjects *baseLib, const PParameterGroup &parent);
  virtual ~Parameter();

  void parseXml(xml_node *node);

  //Helpers
  /**
   * Converts binary data of a packet received by a physical interface to a RPC variable.
   *
   * @param data The data to convert.
   * @param role The associated role of the value.
   * @param isEvent Set to "true" if packet is an event packet. Necessary to set value of "Action" correctly.
   * @return Returns the RPC variable.
   */
  PVariable convertFromPacket(const std::vector<uint8_t> &data, const Role &role, bool isEvent);

  /**
   * Converts a RPC variable to binary data to send it over a physical interface.
   *
   * @param[in] value The value to convert.
   * @param[in] role The associated role of the value.
   * @param[out] convertedValue The converted binary data.
   */
  void convertToPacket(const PVariable &value, const Role &role, std::vector<uint8_t> &convertedValue);

  /**
   * Tries to convert a string value to a binary data to send it over a physical interface.
   *
   * @param[in] value The value to convert.
   * @param[in] role The associated role of the value.
   * @param[out] convertedValue The converted binary data.
   */
  void convertToPacket(const std::string &value, const Role &role, std::vector<uint8_t> &convertedValue);

  void adjustBitPosition(std::vector<uint8_t> &data);

  PParameterGroup parent();
 protected:
  BaseLib::SharedObjects *_bl = nullptr;

  //Helpers
  std::weak_ptr<ParameterGroup> _parent;

  /**
   * Reverses a binary array.
   *
   * @param[in] data The array to reverse.
   * @param[out] reversedData The reversed array.
   */
  void reverseData(const std::vector<uint8_t> &data, std::vector<uint8_t> &reversedData);
};

}
}

#endif
