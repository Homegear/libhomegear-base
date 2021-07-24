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

#include "ServiceMessage.h"

namespace BaseLib {

PVariable BaseLib::ServiceMessage::serialize() {
  auto serviceMessage = std::make_shared<Variable>(VariableType::tStruct);
  serviceMessage->structValue->emplace("TYPE", std::make_shared<Variable>((int32_t)type));
  serviceMessage->structValue->emplace("MESSAGE", TranslationManager::getTranslations(message, variables));
  serviceMessage->structValue->emplace("VALUE", std::make_shared<Variable>(value));
  serviceMessage->structValue->emplace("TIMESTAMP", std::make_shared<Variable>(timestamp));
  if (type == ServiceMessageType::kGlobal) {
    serviceMessage->structValue->emplace("MESSAGE_ID", std::make_shared<Variable>(messageId));
    serviceMessage->structValue->emplace("MESSAGE_SUBID", std::make_shared<Variable>(messageSubId));
    if (data) serviceMessage->structValue->emplace("DATA", data);
  } else if (type == ServiceMessageType::kFamily) {
    serviceMessage->structValue->emplace("FAMILY_ID", std::make_shared<Variable>(familyId));
    if (!interface.empty()) serviceMessage->structValue->emplace("INTERFACE", std::make_shared<Variable>(interface));
    serviceMessage->structValue->emplace("MESSAGE_ID", std::make_shared<Variable>(messageId));
    serviceMessage->structValue->emplace("MESSAGE_SUBID", std::make_shared<Variable>(messageSubId));
    if (data) serviceMessage->structValue->emplace("DATA", data);
  } else if (type == ServiceMessageType::kDevice) {
    serviceMessage->structValue->emplace("PEER_ID", std::make_shared<Variable>(peerId));
    serviceMessage->structValue->emplace("CHANNEL", std::make_shared<Variable>(channel));
    serviceMessage->structValue->emplace("VARIABLE", std::make_shared<Variable>(variable));
  }

  return serviceMessage;
}

}