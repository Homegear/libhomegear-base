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

#include "../BaseLib.h"
#include "ThreadManager.h"

namespace BaseLib
{

bool _stopThreadCountTest = false;

void* threadCountTest(void*)
{
    while(!_stopThreadCountTest)
    {
    	std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return nullptr;
}

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::init(BaseLib::SharedObjects* baseLib, bool testMaxThreadCount)
{
	_bl = baseLib;
	if(testMaxThreadCount) this->testMaxThreadCount();
}

void ThreadManager::testMaxThreadCount()
{
	std::vector<pthread_t> threads;
	threads.reserve(1000);
	while(true)
	{
		pthread_t thread;
		if(pthread_create(&thread, nullptr, threadCountTest, nullptr) != 0 || _maxThreadCount > 10000)
		{
			_stopThreadCountTest = true;
			for(auto i : threads)
			{
				pthread_join(i, nullptr);
			}
			_maxThreadCount = _maxThreadCount * 90 / 100;
			return;
		}
		threads.push_back(thread);
		if(threads.size() > threads.capacity() - 10) threads.reserve(threads.size() * 2);
		_maxThreadCount++;
	}
}

uint32_t ThreadManager::getMaxRegisteredThreadCount()
{
	return _maxRegisteredThreadCount;
}

void ThreadManager::setMaxThreadCount(uint32_t value)
{
	_maxThreadCount = value;
}

uint32_t ThreadManager::getMaxThreadCount()
{
	return _maxThreadCount;
}

int32_t ThreadManager::getCurrentThreadCount()
{
	std::lock_guard<std::mutex> threadCountGuard(_threadCountMutex);
	int32_t count = _currentThreadCount;
	return count;
}

bool ThreadManager::checkThreadCount(bool highPriority)
{
	if(_maxThreadCount == 0) return true;
	if(highPriority && _currentThreadCount < (signed)_maxThreadCount) return true;
	else if(_currentThreadCount < (signed)_maxThreadCount * 90 / 100) return true;
	if(highPriority) _bl->out.printCritical("Critical: Can't start more threads. Thread limit reached (" + std::to_string(_maxThreadCount) + " threads).");
	else _bl->out.printCritical("Critical: Can't start more low priority threads. 90% of thread limit reached (" + std::to_string(_currentThreadCount) + " of " + std::to_string(_maxThreadCount) + ").");
	return false;
}

int32_t ThreadManager::getThreadPolicyFromString(std::string policy)
{
	HelperFunctions::toLower(policy);
	if(policy == "sched_other") return SCHED_OTHER;
	else if(policy == "sched_rr") return SCHED_RR;
	else if(policy == "sched_fifo") return SCHED_FIFO;
#ifdef SCHED_IDLE
	else if(policy == "sched_idle") return SCHED_IDLE;
#endif
#ifdef SCHED_BATCH
	else if(policy == "sched_batch") return SCHED_BATCH;
#endif
    return 0;
}

int32_t ThreadManager::parseThreadPriority(int32_t priority, int32_t policy)
{
	if(policy == SCHED_FIFO || policy == SCHED_OTHER)
	{
		if(priority > 99) return 99;
		else if(priority < 1) return 1;
		else return priority;
	}
	else return 0;
}

void ThreadManager::setThreadPriority(pthread_t thread, int32_t priority, int32_t policy)
{
	try
	{
		if(!_bl->settings.prioritizeThreads()) return;
		if(priority == -1)
		{
			_bl->out.printWarning("Warning: Priority of -1 was passed to setThreadPriority.");
			priority = 0;
			policy = SCHED_OTHER;
		}
		if(policy == SCHED_OTHER) return;
		if((policy == SCHED_FIFO || policy == SCHED_RR) && (priority < 1 || priority > 99)) throw Exception("Invalid thread priority for SCHED_FIFO or SCHED_RR: " + std::to_string(priority));
#ifdef SCHED_IDLE
#ifdef SCHED_BATCH
		else if((policy == SCHED_IDLE || policy == SCHED_BATCH) && priority != 0) throw Exception("Invalid thread priority for SCHED_IDLE: " + std::to_string(priority));
#endif
#endif
		sched_param schedParam;
		schedParam.sched_priority = priority;
		int32_t error;
		//Only use SCHED_FIFO or SCHED_RR
		if((error = pthread_setschedparam(thread, policy, &schedParam)) != 0)
		{
			if(error == EPERM)
			{
				_bl->out.printInfo("Info: Could not set thread priority. The executing user does not have enough privileges. Please run \"ulimit -r 100\" before executing Homegear.");
			}
			else if(error == ESRCH) _bl->out.printError("Could not set thread priority. Thread could not be found.");
			else if(error == EINVAL) _bl->out.printError("Could not set thread priority: policy is not a recognized policy, or param does not make sense for the policy.");
			else _bl->out.printError("Error: Could not set thread priority to " + std::to_string(priority) + " Error: " + std::to_string(error));
			_bl->settings.setPrioritizeThreads(false);
		}
		else _bl->out.printDebug("Debug: Thread priority successfully set to: " + std::to_string(priority), 7);
	}
	catch(const std::exception& ex)
    {
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ThreadManager::join(std::thread& thread)
{
	if(thread.joinable())
	{
		thread.join();
		unregisterThread();
	}
}

void ThreadManager::registerThread()
{
	std::lock_guard<std::mutex> threadCountGuard(_threadCountMutex);
	_currentThreadCount++;
	if(_currentThreadCount > (signed)_maxRegisteredThreadCount) _maxRegisteredThreadCount = _currentThreadCount;
}

void ThreadManager::unregisterThread()
{
	std::lock_guard<std::mutex> threadCountGuard(_threadCountMutex);
	_currentThreadCount--;
}

}
