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

#ifndef IQUEUE_H_
#define IQUEUE_H_

#include "IQueueBase.h"

#include <vector>

namespace BaseLib
{
class SharedObjects;

class IQueueEntry
{
public:
	IQueueEntry() {};
	virtual ~IQueueEntry() {};
};

class IQueue : public IQueueBase
{
public:
	IQueue(SharedObjects* baseLib, uint32_t queueCount, uint32_t bufferSize);
	virtual ~IQueue();
	void startQueue(int32_t index, uint32_t processingThreadCount, int32_t threadPriority, int32_t threadPolicy);
	void stopQueue(int32_t index);
	bool enqueue(int32_t index, std::shared_ptr<IQueueEntry>& entry);
	virtual void processQueueEntry(int32_t index, std::shared_ptr<IQueueEntry>& entry) = 0;
	bool queueEmpty(int32_t index);
private:
	int32_t _bufferSize = 1000;
	std::vector<int32_t> _bufferHead;
	std::vector<int32_t> _bufferTail;
	std::vector<int32_t> _bufferCount;
	std::unique_ptr<std::mutex[]> _bufferMutex = nullptr;
	std::vector<std::vector<std::shared_ptr<IQueueEntry>>> _buffer;
	std::unique_ptr<std::mutex[]> _queueMutex = nullptr;
	std::vector<std::vector<std::shared_ptr<std::thread>>> _processingThread;
	std::unique_ptr<std::condition_variable[]> _produceConditionVariable = nullptr;
	std::unique_ptr<std::condition_variable[]> _processingConditionVariable = nullptr;

	void process(int32_t index);
};

}
#endif
