/* Copyright 2013-2017 Sathya Laufer
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

#ifndef IQUEUEBASE_H_
#define IQUEUEBASE_H_

#include "Output/Output.h"

#include <atomic>
#include <memory>
#include <condition_variable>
#include <thread>
#include <vector>

namespace BaseLib
{
class SharedObjects;

class IQueueBase
{
public:
	IQueueBase(SharedObjects* baseLib, uint32_t queueCount);
	virtual ~IQueueBase() {}

	void printQueueFullError(BaseLib::Output& out, std::string message);
protected:
	SharedObjects* _bl = nullptr;
	int32_t _queueCount = 2;
	std::unique_ptr<std::atomic_bool[]> _stopProcessingThread;

	std::atomic<uint32_t> _droppedEntries{0};
	std::atomic<int64_t> _lastQueueFullError{0};
};

}
#endif
