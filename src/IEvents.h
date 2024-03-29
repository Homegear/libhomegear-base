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

#ifndef IEVENTS_H_
#define IEVENTS_H_

#include "Output/Output.h"

#include <atomic>
#include <forward_list>
#include <mutex>
#include <unordered_map>

namespace BaseLib {

class IEventSinkBase;

class EventHandler {
 public:
  EventHandler(int32_t id);
  EventHandler(int32_t id, IEventSinkBase *handler);
  virtual ~EventHandler();

  int32_t id();
  int32_t useCount();
  IEventSinkBase *handler();
  void lock();
  void unlock();
  void invalidate();
 private:
  int32_t _id;
  std::atomic<int32_t> _useCount;
  IEventSinkBase *_handler = nullptr;
};

typedef std::shared_ptr<EventHandler> PEventHandler;
typedef std::unordered_map<IEventSinkBase *, PEventHandler> EventHandlers;

class IEventSinkBase {
 public:
  IEventSinkBase() {}
  virtual ~IEventSinkBase() {};
};

class IEvents {
 public:
  IEvents();
  virtual ~IEvents();

  virtual void setEventHandler(IEventSinkBase *eventHandler);
  virtual void resetEventHandler();
  virtual IEventSinkBase *getEventHandler();
 protected:
  IEventSinkBase *_eventHandler = nullptr;
 private:
  IEvents(const IEvents &);
  IEvents &operator=(const IEvents &);
};

class IEventsEx {
 public:
  IEventsEx();
  virtual ~IEventsEx();

  virtual PEventHandler addEventHandler(IEventSinkBase *eventHandler);
  virtual std::vector<PEventHandler> addEventHandlers(EventHandlers eventHandlers);
  virtual void removeEventHandler(PEventHandler eventHandler);
  virtual EventHandlers getEventHandlers();
 protected:
  int32_t _currentId = 0;
  std::mutex _eventHandlerMutex;
  EventHandlers _eventHandlers;
 private:
  IEventsEx(const IEventsEx &);
  IEventsEx &operator=(const IEventsEx &);
};
}
#endif
