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

#include "IQueue.h"

#include <memory>
#include "BaseLib.h"

namespace BaseLib {

IQueue::IQueue(SharedObjects *baseLib, uint32_t queueCount, uint32_t bufferSize) : IQueueBase(baseLib, queueCount) {
  if (bufferSize < 2000000000) _bufferSize = (int32_t)bufferSize;

  _bufferHead.resize(queueCount);
  _bufferTail.resize(queueCount);
  _bufferCount.resize(queueCount, 0);
  _waitWhenFull.resize(queueCount);
  _buffer.resize(queueCount);
  _queueMutex = std::make_unique<std::mutex[]>(queueCount);
  _processingThread.resize(queueCount);
  _produceConditionVariable = std::make_unique<std::condition_variable[]>(queueCount);
  _processingConditionVariable = std::make_unique<std::condition_variable[]>(queueCount);

  _threadsInUse = std::make_unique<std::atomic<uint32_t>[]>(queueCount);

  _maxThreadLoad1m = std::make_unique<std::atomic<double>[]>(queueCount);
  _maxThreadLoad1mCurrent = std::make_unique<std::atomic<double>[]>(queueCount);
  _maxWait1m = std::make_unique<std::atomic<int64_t>[]>(queueCount);
  _maxWait1mCurrent = std::make_unique<std::atomic<int64_t>[]>(queueCount);
  _last1mCycle = std::make_unique<std::atomic<int64_t>[]>(queueCount);

  _maxThreadLoad10m = std::make_unique<std::atomic<double>[]>(queueCount);
  _maxThreadLoad10mCurrent = std::make_unique<std::atomic<double>[]>(queueCount);
  _maxWait10m = std::make_unique<std::atomic<int64_t>[]>(queueCount);
  _maxWait10mCurrent = std::make_unique<std::atomic<int64_t>[]>(queueCount);
  _last10mCycle = std::make_unique<std::atomic<int64_t>[]>(queueCount);

  _maxThreadLoad1h = std::make_unique<std::atomic<double>[]>(queueCount);
  _maxThreadLoad1hCurrent = std::make_unique<std::atomic<double>[]>(queueCount);
  _maxWait1h = std::make_unique<std::atomic<int64_t>[]>(queueCount);
  _maxWait1hCurrent = std::make_unique<std::atomic<int64_t>[]>(queueCount);
  _last1hCycle = std::make_unique<std::atomic<int64_t>[]>(queueCount);

  _maxThreadLoad = std::make_unique<std::atomic<double>[]>(queueCount);
  _maxWait = std::make_unique<std::atomic<int64_t>[]>(queueCount);

  for (int32_t i = 0; i < _queueCount; i++) {
    _bufferHead[i] = 0;
    _bufferTail[i] = 0;
    _bufferCount[i] = 0;
    _stopProcessingThread[i] = true;

    _maxThreadLoad1m[i] = 0;
    _maxThreadLoad1mCurrent[i] = 0;
    _maxWait1m[i] = 0;
    _maxWait1mCurrent[i] = 0;
    _last1mCycle[i] = 0;

    _maxThreadLoad10m[i] = 0;
    _maxThreadLoad10mCurrent[i] = 0;
    _maxWait10m[i] = 0;
    _maxWait10mCurrent[i] = 0;
    _last10mCycle[i] = 0;

    _maxThreadLoad1h[i] = 0;
    _maxThreadLoad1hCurrent[i] = 0;
    _maxWait1h[i] = 0;
    _maxWait1hCurrent[i] = 0;
    _last1hCycle[i] = 0;

    _maxThreadLoad[i] = 0;
    _maxWait[i] = 0;
  }
}

IQueue::~IQueue() {
  for (int32_t i = 0; i < _queueCount; i++) {
    stopQueue(i);
    _buffer[i].clear();
  }
}

int32_t IQueue::queueSize(int32_t index) {
  return _bufferCount[index];
}

bool IQueue::queueEmpty(int32_t index) {
  return _bufferCount[index] > 0;
}

double IQueue::maxThreadLoad(int32_t index) {
  return _maxThreadLoad[index];
}

double IQueue::maxThreadLoad1m(int32_t index) {
  return _maxThreadLoad1m[index];
}

double IQueue::maxThreadLoad10m(int32_t index) {
  return _maxThreadLoad10m[index];
}

double IQueue::maxThreadLoad1h(int32_t index) {
  return _maxThreadLoad1h[index];
}

int64_t IQueue::maxWait(int32_t index) {
  return _maxWait[index];
}

int64_t IQueue::maxWait1m(int32_t index) {
  return _maxWait1m[index];
}

int64_t IQueue::maxWait10m(int32_t index) {
  return _maxWait10m[index];
}

int64_t IQueue::maxWait1h(int32_t index) {
  return _maxWait1h[index];
}

void IQueue::startQueue(int32_t index, bool waitWhenFull, uint32_t processingThreadCount, int32_t threadPriority, int32_t threadPolicy) {
  if (index < 0 || index >= _queueCount) return;
  _stopProcessingThread[index] = false;
  _bufferHead[index] = 0;
  _bufferTail[index] = 0;
  _bufferCount[index] = 0;
  _waitWhenFull[index] = waitWhenFull;
  for (uint32_t i = 0; i < processingThreadCount; i++) {
    std::shared_ptr<std::thread> thread = std::make_shared<std::thread>();
    _bl->threadManager.start(*thread, true, threadPriority, threadPolicy, &IQueue::process, this, index);
    _processingThread[index].push_back(thread);
  }
  _buffer.at(index).resize(_bufferSize);
}

void IQueue::stopQueue(int32_t index) {
  if (index < 0 || index >= _queueCount) return;
  if (_stopProcessingThread[index]) return;
  _stopProcessingThread[index] = true;
  std::unique_lock<std::mutex> lock(_queueMutex[index]);
  lock.unlock();
  _processingConditionVariable[index].notify_all();
  _produceConditionVariable[index].notify_all();
  for (uint32_t i = 0; i < _processingThread[index].size(); i++) {
    _bl->threadManager.join(*(_processingThread[index][i]));
  }
  _processingThread[index].clear();
  _buffer[index].clear();

}

bool IQueue::queueIsStarted(int32_t index) {
  return _stopProcessingThread[index] == false;
}

bool IQueue::enqueue(int32_t index, std::shared_ptr<IQueueEntry> &entry, bool waitWhenFull) {
  try {
    if (index < 0 || index >= _queueCount || !entry || _stopProcessingThread[index]) return true;
    entry->time = HelperFunctions::getTime();
    std::unique_lock<std::mutex> lock(_queueMutex[index]);
    if (_waitWhenFull[index] || waitWhenFull) {
      while (!_produceConditionVariable[index].wait_for(lock, std::chrono::milliseconds(1000), [&] {
        return _bufferCount[index] < _bufferSize || _stopProcessingThread[index];
      }));
      if (_stopProcessingThread[index]) return true;
    } else if (_bufferCount[index] >= _bufferSize) return false;

    _buffer[index][_bufferTail[index]] = entry;
    _bufferTail[index] = (_bufferTail[index] + 1) % _bufferSize;
    ++(_bufferCount[index]);

    lock.unlock();
    _processingConditionVariable[index].notify_one();
    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return false;
}

void IQueue::process(int32_t index) {
  if (index < 0 || index >= _queueCount) return;
  while (!_stopProcessingThread[index]) {
    try {
      std::unique_lock<std::mutex> lock(_queueMutex[index]);

      while (!_processingConditionVariable[index].wait_for(lock, std::chrono::milliseconds(1000), [&] {
        return _bufferCount[index] > 0 || _stopProcessingThread[index];
      }));
      if (_stopProcessingThread[index]) return;

      { //Metrics
        _threadsInUse[index]++;
        auto time = BaseLib::HelperFunctions::getTime();
        double backlog = _bufferCount[index];
        double threadLoad = ((double)_threadsInUse[index] / (double)_processingThread[index].size()) + (backlog / (double)_processingThread[index].size());

        if (time - _last1mCycle[index] >= 60000) {
          _last1mCycle[index] = time;
          _maxThreadLoad1m[index] = _maxThreadLoad1mCurrent[index].load();
          _maxThreadLoad1mCurrent[index] = 0;
          _maxWait1m[index] = _maxWait1mCurrent[index].load();
          _maxWait1mCurrent[index] = 0;
        }

        if (time - _last10mCycle[index] >= 600000) {
          _last10mCycle[index] = time;
          _maxThreadLoad10m[index] = _maxThreadLoad10mCurrent[index].load();
          _maxThreadLoad10mCurrent[index] = 0;
          _maxWait10m[index] = _maxWait10mCurrent[index].load();
          _maxWait10mCurrent[index] = 0;
        }

        if (time - _last1hCycle[index] >= 3600000) {
          _last1hCycle[index] = time;
          _maxThreadLoad1h[index] = _maxThreadLoad1hCurrent[index].load();
          _maxThreadLoad1hCurrent[index] = 0;
          _maxWait1h[index] = _maxWait1hCurrent[index].load();
          _maxWait1hCurrent[index] = 0;
        }

        if (threadLoad > _maxThreadLoad[index]) _maxThreadLoad[index] = threadLoad;
        if (threadLoad > _maxThreadLoad1mCurrent[index]) _maxThreadLoad1mCurrent[index] = threadLoad;
        if (threadLoad > _maxThreadLoad10mCurrent[index]) _maxThreadLoad10mCurrent[index] = threadLoad;
        if (threadLoad > _maxThreadLoad1hCurrent[index]) _maxThreadLoad1hCurrent[index] = threadLoad;
      }

      do {
        std::shared_ptr<IQueueEntry> entry;

        entry = _buffer[index][_bufferHead[index]];
        _buffer[index][_bufferHead[index]].reset();
        _bufferHead[index] = (_bufferHead[index] + 1) % _bufferSize;
        --_bufferCount[index];

        lock.unlock();

        _produceConditionVariable[index].notify_one();

        try {
          if (entry) {
            { //Metrics
              auto time = BaseLib::HelperFunctions::getTime();
              int64_t latency = time - entry->time;
              if (latency > _maxWait[index]) _maxWait[index] = latency;
              if (latency > _maxWait1mCurrent[index]) _maxWait1mCurrent[index] = latency;
              if (latency > _maxWait10mCurrent[index]) _maxWait10mCurrent[index] = latency;
              if (latency > _maxWait1hCurrent[index]) _maxWait1hCurrent[index] = latency;
            }

            processQueueEntry(index, entry);
          }
        } catch (const std::exception &ex) {
          _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
        }

        lock.lock();
      } while (_bufferCount[index] > 0 && !_stopProcessingThread[index]);
      _threadsInUse[index]--;
    }
    catch (const std::exception &ex) {
      _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
      _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
  }
}

}
