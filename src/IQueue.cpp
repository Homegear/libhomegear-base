/* Copyright 2013-2015 Sathya Laufer
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

#include "IQueue.h"
#include "BaseLib.h"

namespace BaseLib
{

IQueue::IQueue(Obj* baseLib)
{
	_bl = baseLib;
	for(int32_t i = 0; i < _queueCount; i++)
	{
		_bufferHead[i] = 0;
		_bufferTail[i] = 0;
		_processingEntryAvailable[i] = false;
		_stopProcessingThread[i] = true;
	}
}

IQueue::~IQueue()
{
	for(int32_t i = 0; i < _queueCount; i++)
	{
		stopQueue(i);
	}
}

void IQueue::startQueue(int32_t index, int32_t threadPriority, int32_t threadPolicy)
{
	if(index < 0 || index >= _queueCount) return;
	_stopProcessingThread[index] = false;
	_processingEntryAvailable[index] = false;
	_bufferHead[index] = 0;
	_bufferTail[index] = 0;
	_processingThread[index] = std::thread(&IQueue::process, this, index);
	Threads::setThreadPriority(_bl, _processingThread[index].native_handle(), threadPriority, threadPolicy);
}

void IQueue::stopQueue(int32_t index)
{
	if(index < 0 || index >= _queueCount) return;
	if(_stopProcessingThread[index]) return;
	_stopProcessingThread[index] = true;
	_processingEntryAvailable[index] = true;
	_processingConditionVariable[index].notify_one();
	if(_processingThread[index].joinable()) _processingThread[index].join();
}

bool IQueue::enqueue(int32_t index, std::shared_ptr<IQueueEntry>& entry)
{
	try
	{
		if(index < 0 || index >= _queueCount) return false;
		_bufferMutex[index].lock();
		int32_t tempHead = _bufferHead[index] + 1;
		if(tempHead >= _bufferSize) tempHead = 0;
		if(tempHead == _bufferTail[index])
		{
			_bufferMutex[index].unlock();
			return false;
		}

		_buffer[index][_bufferHead[index]] = entry;
		_bufferHead[index]++;
		if(_bufferHead[index] >= _bufferSize)
		{
			_bufferHead[index] = 0;
		}
		_processingEntryAvailable[index] = true;
		_bufferMutex[index].unlock();

		_processingConditionVariable[index].notify_one();
		return true;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		_bufferMutex[index].unlock();
	}
	catch(BaseLib::Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		_bufferMutex[index].unlock();
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		_bufferMutex[index].unlock();
	}
	return false;
}

void IQueue::process(int32_t index)
{
	if(index < 0 || index >= _queueCount) return;
	while(!_stopProcessingThread[index])
	{
		std::unique_lock<std::mutex> lock(_processingThreadMutex[index]);
		try
		{
			_bufferMutex[index].lock();
			if(_bufferHead[index] == _bufferTail[index]) //Only lock, when there is really no packet to process. This check is necessary, because the check of the while loop condition is outside of the mutex
			{
				_bufferMutex[index].unlock();
				_processingConditionVariable[index].wait(lock, [&]{ return _processingEntryAvailable[index]; });
			}
			else _bufferMutex[index].unlock();
			if(_stopProcessingThread[index])
			{
				lock.unlock();
				return;
			}

			while(_bufferHead[index] != _bufferTail[index])
			{
				_bufferMutex[index].lock();
				std::shared_ptr<IQueueEntry> entry = _buffer[index][_bufferTail[index]];
				_buffer[index][_bufferTail[index]].reset();
				_bufferTail[index]++;
				if(_bufferTail[index] >= _bufferSize) _bufferTail[index] = 0;
				if(_bufferHead[index] == _bufferTail[index]) _processingEntryAvailable[index] = false; //Set here, because otherwise it might be set to "true" in publish and then set to false again after the while loop
				_bufferMutex[index].unlock();
				if(entry) processQueueEntry(index, entry);
			}
		}
		catch(const std::exception& ex)
		{
			_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		}
		catch(const BaseLib::Exception& ex)
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
