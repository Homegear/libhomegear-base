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

#ifndef IQUEUE_H_
#define IQUEUE_H_

#include "IQueueBase.h"

#include <vector>

namespace BaseLib {
class SharedObjects;

class IQueueEntry {
 public:
  IQueueEntry() {};
  virtual ~IQueueEntry() {};
};

/**
 * This class implements a queue after the producer-consumer paradigma. It can manage one or more queues. Your class needs to be derived from @c IQueue to use it.
 */
class IQueue : public IQueueBase {
 public:
  /**
   * Constructor.
   *
   * @param baseLib A base library object.
   * @param queueCount The number of queues to initialize.
   * @param bufferSize The maximum number of items allowed to be queued.
   */
  IQueue(SharedObjects *baseLib, uint32_t queueCount, uint32_t bufferSize);
  virtual ~IQueue();

  /**
   * Starts the threads of a queue.
   *
   * @param index The index of the queue to start. The number of queues is defined by @c queueCount in the constructor.
   * @param waitWhenFull When set to @c true, @c enqueue() waits until the queue is empty enough to queue the provided item. This takes precedence over the argument @c waitWhenFull of @c enqueue().
   * @param processingThreadCount The number of processing threads to start.
   * @param threadPriority The thread priority to set. Default is @c 0 (= disabled). The thread priority values depends on @c threadPolicy. See <tt>man sched</tt> for more details.
   * @param threadPolicy The thread policy to use. Default is @c SCHED_OTHER (= disabled). See <tt>man sched</tt> for more details.
   */
  void startQueue(int32_t index, bool waitWhenFull, uint32_t processingThreadCount, int32_t threadPriority = 0, int32_t threadPolicy = SCHED_OTHER);

  /**
   * Stops the threads of a queue previously started with @c startQueue().
   *
   * @param index The index of the queue to stop.
   */
  void stopQueue(int32_t index);

  /**
   * Checks if the specified queue has been started.
   */
  bool queueIsStarted(int32_t index);

  /**
   * Enqueues an item.
   *
   * @param index The index of the queue to enqueue the item into.
   * @param entry The item to queue.
   * @param waitWhenFull When set to @c true the method waits until the queue is empty enough to queue the item. When @c waitWhenFull is set to @c true in @c startQueue(), this argument is ignored.
   * @return Returns @c true if the item was successfully queued and @c false otherwise.
   */
  bool enqueue(int32_t index, std::shared_ptr<IQueueEntry> &entry, bool waitWhenFull = false);

  /**
   * This method is called by the processing threads for each item to process. It needs to be implemented by the derived class.
   *
   * @param index The index of the queue.
   * @param entry The queued item to process.
   */
  virtual void processQueueEntry(int32_t index, std::shared_ptr<IQueueEntry> &entry) = 0;

  /**
   * Checks if a queue is empty.
   *
   * @param index The index of the queue to check.
   * @return Returns @c true if the queue is empty.
   */
  bool queueEmpty(int32_t index);

  /**
   * Returns the number of items queued in a queue.
   *
   * @param index The index of the queue to check.
   * @return Return the number of queued items.
   */
  int32_t queueSize(int32_t index);
 private:
  int32_t _bufferSize = 10000;
  std::vector<int32_t> _bufferHead;
  std::vector<int32_t> _bufferTail;
  std::vector<int32_t> _bufferCount;
  std::vector<bool> _waitWhenFull;
  std::vector<std::vector<std::shared_ptr<IQueueEntry>>> _buffer;
  std::unique_ptr<std::mutex[]> _queueMutex = nullptr;
  std::vector<std::vector<std::shared_ptr<std::thread>>> _processingThread;
  std::unique_ptr<std::condition_variable[]> _produceConditionVariable = nullptr;
  std::unique_ptr<std::condition_variable[]> _processingConditionVariable = nullptr;

  void process(int32_t index);
};

}
#endif
