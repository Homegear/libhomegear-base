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

#include "ITimedQueue.h"
#include "BaseLib.h"

namespace BaseLib
{

ITimedQueue::ITimedQueue(SharedObjects* baseLib, uint32_t queueCount) : IQueueBase(baseLib, queueCount)
{
	_firstPositionChanged.resize(queueCount);
	_bufferMutex.reset(new std::mutex[queueCount]);
	_buffer.resize(queueCount);
	_processingThreadMutex.reset(new std::mutex[queueCount]);
	_processingThread.resize(queueCount);
	_processingConditionVariable.reset(new std::condition_variable[queueCount]);

	for(int32_t i = 0; i < _queueCount; i++)
	{
		_stopProcessingThread[i] = true;
		_firstPositionChanged[i] = false;
	}
}

ITimedQueue::~ITimedQueue()
{
	for(int32_t i = 0; i < _queueCount; i++)
	{
		stopQueue(i);
	}
}

void ITimedQueue::startQueue(int32_t index, int32_t threadPriority, int32_t threadPolicy)
{
	if(index < 0 || index >= _queueCount) return;
	_stopProcessingThread[index] = false;
	_bl->threadManager.start(_processingThread[index], true, threadPriority, threadPolicy, &ITimedQueue::process, this, index);
}

void ITimedQueue::stopQueue(int32_t index)
{
	if(index < 0 || index >= _queueCount) return;
	if(_stopProcessingThread[index]) return;
	_stopProcessingThread[index] = true;
	_processingConditionVariable[index].notify_one();
	_bl->threadManager.join(_processingThread[index]);
}

bool ITimedQueue::enqueue(int32_t index, std::shared_ptr<ITimedQueueEntry>& entry, int64_t& id)
{
	try
	{
		if(index < 0 || index >= _queueCount || !entry) return false;
		{
			std::lock_guard<std::mutex> bufferGuard(_bufferMutex[index]);
			if(_buffer[index].size() >= (unsigned)_bufferSize) return false;

			id = entry->getTime();
			while(_buffer[index].find(id) != _buffer[index].end()) id++;

			if(!_buffer[index].empty() && _buffer[index].begin()->first > id) _firstPositionChanged[index] = true;
			_buffer[index].insert(std::pair<int64_t, std::shared_ptr<ITimedQueueEntry>>(id, entry));
		}

		_processingConditionVariable[index].notify_one();
		return true;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

void ITimedQueue::removeQueueEntry(int32_t index, int64_t id)
{
	try
	{
		std::lock_guard<std::mutex> bufferGuard(_bufferMutex[index]);
		_buffer[index].erase(id);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void ITimedQueue::process(int32_t index)
{
	if(index < 0 || index >= _queueCount) return;
	int64_t now;
	int64_t next;
	while(!_stopProcessingThread[index])
	{
		std::unique_lock<std::mutex> lock(_processingThreadMutex[index]);
		try
		{
			_bufferMutex[index].lock();
			now = _bl->hf.getTime();
			if(_buffer[index].empty() || _buffer[index].begin()->first > now) //Only lock, when there is really no packet to process. This check is necessary, because the check of the while loop condition is outside of the mutex
			{
				next = _buffer[index].empty() ? 0 : _buffer[index].begin()->first;
				_bufferMutex[index].unlock();
				if(next > 0) _processingConditionVariable[index].wait_until(lock, std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::milliseconds(next)), [&]{ std::lock_guard<std::mutex> bufferGuard(_bufferMutex[index]); return _buffer[index].empty() || _buffer[index].begin()->first <= _bl->hf.getTime() || _firstPositionChanged[index] || _stopProcessingThread[index]; });
				else {
                  while (!_processingConditionVariable[index].wait_for(lock, std::chrono::milliseconds(1000), [&]{
                    std::lock_guard<std::mutex> bufferGuard(_bufferMutex[index]); return !_buffer[index].empty() || _stopProcessingThread[index];
                  }));
				}
				if(_firstPositionChanged[index]) _firstPositionChanged[index] = false;
			}
			else _bufferMutex[index].unlock();
			if(_stopProcessingThread[index]) return;

			now = _bl->hf.getTime();

			int64_t id = 0;
			std::shared_ptr<ITimedQueueEntry> entry;
			{
				std::lock_guard<std::mutex> bufferGuard(_bufferMutex[index]);
				if(_buffer[index].empty() || _buffer[index].begin()->first > now) continue;
				id = _buffer[index].begin()->first;
				entry = _buffer[index].begin()->second;
				_buffer[index].erase(_buffer[index].begin());
			}
			if(entry) processQueueEntry(index, id, entry);
		}
		catch(const std::exception& ex)
		{
			_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		}
		catch(...)
		{
			_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		}
		lock.unlock();
	}
}

}
