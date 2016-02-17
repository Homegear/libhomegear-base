/* Copyright 2013-2016 Sathya Laufer
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

IQueue::IQueue(Obj* baseLib, int32_t bufferSize)
{
	_bl = baseLib;
	if(bufferSize > 0) _bufferSize = bufferSize;
	for(int32_t i = 0; i < _queueCount; i++)
	{
		_buffer[i] = nullptr;
		_bufferHead[i] = 0;
		_bufferTail[i] = 0;
		_bufferCount[i] = 0;
		_stopProcessingThread[i] = true;
	}
}

IQueue::~IQueue()
{
	for(int32_t i = 0; i < _queueCount; i++)
	{
		stopQueue(i);
		if(_buffer[i]) delete[] _buffer[i];
	}
}

bool IQueue::queueEmpty(int32_t index)
{
	return _bufferCount[index] > 0;
}

void IQueue::startQueue(int32_t index, uint32_t processingThreadCount, int32_t threadPriority, int32_t threadPolicy)
{
	if(index < 0 || index >= _queueCount) return;
	_stopProcessingThread[index] = false;
	_bufferHead[index] = 0;
	_bufferTail[index] = 0;
	_bufferCount[index] = 0;
	for(uint32_t i = 0; i < processingThreadCount; i++)
	{
		std::shared_ptr<std::thread> thread(new std::thread());
		_bl->threadManager.start(*thread, true, threadPriority, threadPolicy, &IQueue::process, this, index);
		_processingThread[index].push_back(thread);
	}
	if(!_buffer[index]) _buffer[index] = new std::shared_ptr<IQueueEntry>[_bufferSize];
}

void IQueue::stopQueue(int32_t index)
{
	if(index < 0 || index >= _queueCount) return;
	if(_stopProcessingThread[index]) return;
	_stopProcessingThread[index] = true;
	_processingConditionVariable[index].notify_all();
	_produceConditionVariable[index].notify_all();
	for(uint32_t i = 0; i < _processingThread[index].size(); i++)
	{
		_bl->threadManager.join(*(_processingThread[index][i]));
	}
	_processingThread[index].clear();
	delete[] _buffer[index];
	_buffer[index] = nullptr;

}

bool IQueue::enqueue(int32_t index, std::shared_ptr<IQueueEntry>& entry)
{
	try
	{
		if(index < 0 || index >= _queueCount || !entry || !_buffer[index] || _stopProcessingThread[index]) return false;
		std::unique_lock<std::mutex> lock(_queueMutex[index]);
		_produceConditionVariable[index].wait(lock, [&]{ return _bufferCount[index] < _bufferSize || _stopProcessingThread[index]; });
		if(_stopProcessingThread[index]) return true;

        _buffer[index][_bufferTail[index]] = entry;
        _bufferTail[index] = (_bufferTail[index] + 1) % _bufferSize;
        ++(_bufferCount[index]);

		_processingConditionVariable[index].notify_one();
		return true;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

void IQueue::process(int32_t index)
{
	if(index < 0 || index >= _queueCount) return;
	while(!_stopProcessingThread[index])
	{
		try
		{
			std::shared_ptr<IQueueEntry> entry;

			{
				std::unique_lock<std::mutex> lock(_queueMutex[index]);

				_processingConditionVariable[index].wait(lock, [&]{ return _bufferCount[index] > 0 || _stopProcessingThread[index]; });
				if(_stopProcessingThread[index]) return;

				entry = _buffer[index][_bufferHead[index]];
				_bufferHead[index] = (_bufferHead[index] + 1) % _bufferSize;
				--(_bufferCount[index]);

				_produceConditionVariable[index].notify_one();
			}

			if(entry) processQueueEntry(index, entry);
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
	}
}

}
