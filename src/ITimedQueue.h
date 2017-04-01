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

#ifndef ITIMEDQUEUE_H_
#define ITIMEDQUEUE_H_

#include "IQueueBase.h"

#include <map>

namespace BaseLib
{
class SharedObjects;

class ITimedQueueEntry
{
public:
	ITimedQueueEntry() {}
	ITimedQueueEntry(int64_t time) { _time = time; };
	virtual ~ITimedQueueEntry() {};

	int64_t getTime() { return _time; }
	void setTime(int64_t value) { _time = value; }
private:
	int64_t _time = 0;
};

class ITimedQueue : public IQueueBase
{
public:
	ITimedQueue(SharedObjects* baseLib, uint32_t queueCount);
	virtual ~ITimedQueue();
	void startQueue(int32_t index, int32_t threadPriority, int32_t threadPolicy);
	void stopQueue(int32_t index);
	bool enqueue(int32_t index, std::shared_ptr<ITimedQueueEntry>& entry, int64_t& id);
	void removeQueueEntry(int32_t index, int64_t id);
	virtual void processQueueEntry(int32_t index, int64_t id, std::shared_ptr<ITimedQueueEntry>& entry) = 0;
private:
	static const int32_t _bufferSize = 1000;
	std::vector<bool> _firstPositionChanged;
	std::unique_ptr<std::mutex[]> _bufferMutex = nullptr;
	std::vector<std::map<int64_t, std::shared_ptr<ITimedQueueEntry>>> _buffer;
	std::unique_ptr<std::mutex[]> _processingThreadMutex = nullptr;
	std::vector<std::thread> _processingThread;
	std::unique_ptr<std::condition_variable[]> _processingConditionVariable = nullptr;

	void process(int32_t index);
};

}
#endif
