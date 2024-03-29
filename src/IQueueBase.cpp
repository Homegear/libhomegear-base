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

#include "IQueueBase.h"
#include "BaseLib.h"

namespace BaseLib {

IQueueBase::IQueueBase(SharedObjects *baseLib, uint32_t queueCount) {
  _bl = baseLib;
  if (queueCount < 1000000) _queueCount = queueCount;
  _stopProcessingThread.reset(new std::atomic_bool[queueCount]);
}

void IQueueBase::printQueueFullError(BaseLib::Output &out, const std::string &message) {
  uint32_t droppedEntries = ++_droppedEntries;
  if (BaseLib::HelperFunctions::getTime() - _lastQueueFullError > 10000) {
    _lastQueueFullError = BaseLib::HelperFunctions::getTime();
    _droppedEntries = 0;
    out.printError(message + " This message won't repeat for 10 seconds. Dropped outputs since last message: " + std::to_string(droppedEntries));
  }
}

}
