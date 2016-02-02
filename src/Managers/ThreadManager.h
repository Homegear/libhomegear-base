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

#ifndef THREADS_H_
#define THREADS_H_

#include "../Exception.h"
#include "../Output/Output.h"
#include <mutex>

namespace BaseLib
{

class Obj;

class ThreadManager
{
public:
	ThreadManager();
	virtual ~ThreadManager();
	void init(BaseLib::Obj* baseLib, bool testMaxThreadCount);

	static int32_t getThreadPolicyFromString(std::string policy);
	static int32_t parseThreadPriority(int32_t priority, int32_t policy);
	void setThreadPriority(pthread_t thread, int32_t priority, int32_t policy = SCHED_FIFO);

	template<typename Function, typename... Args>
	bool start(std::thread& thread, bool highPriority, Function&& function, Args&&... args)
	{
		if(!checkThreadCount(highPriority)) return false;
		thread = std::thread(function, args...);
		registerThread();
		return true;
	}

	template<typename Function, typename... Args>
	bool start(std::thread& thread, bool highPriority, int32_t priority, int32_t policy, Function&& function, Args&&... args)
	{
		if(!checkThreadCount(highPriority)) return false;
		thread = std::thread(function, args...);
		setThreadPriority(thread.native_handle(), priority, policy);
		registerThread();
		return true;
	}

	void join(std::thread& thread);

	void registerThread();
	void unregisterThread();
	void setMaxThreadCount(uint32_t value);
	uint32_t getMaxThreadCount();
	int32_t getCurrentThreadCount();
	uint32_t getMaxRegisteredThreadCount();
	void testMaxThreadCount();
protected:
	Obj* _bl = nullptr;
    std::mutex _threadCountMutex;
    uint32_t _maxRegisteredThreadCount = 0;
    uint32_t _maxThreadCount = 0;
    volatile int32_t _currentThreadCount = 0;

    bool checkThreadCount(bool highPriority);
private:
	ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
};

}
#endif
