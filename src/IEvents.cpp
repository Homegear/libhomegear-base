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

#include "IEvents.h"
#include <thread>

namespace BaseLib {

EventHandler::EventHandler(int32_t id) {
  _id = id;
  _useCount = 0;
}

EventHandler::EventHandler(int32_t id, IEventSinkBase *handler) {
  _id = id;
  _handler = handler;
  _useCount = 0;
}

EventHandler::~EventHandler() {
}

int32_t EventHandler::id() {
  return _id;
}

int32_t EventHandler::useCount() {
  return _useCount;
}

IEventSinkBase *EventHandler::handler() {
  return _handler;
}

void EventHandler::lock() {
  _useCount++;
}

void EventHandler::unlock() {
  _useCount--;
}

void EventHandler::invalidate() {
  //We don't lock mutex here. Make sure it is locked before calling this method.
  _handler = nullptr;
}

IEvents::IEvents() {

}

IEvents::~IEvents() {

}

void IEvents::setEventHandler(IEventSinkBase *eventHandler) {
  if (!eventHandler) return;
  _eventHandler = eventHandler;
}

void IEvents::resetEventHandler() {
  _eventHandler = nullptr;
}

IEventSinkBase *IEvents::getEventHandler() {
  return _eventHandler;
}

IEventsEx::IEventsEx() {

}

IEventsEx::~IEventsEx() {

}

PEventHandler IEventsEx::addEventHandler(IEventSinkBase *eventHandler) {
  PEventHandler handler;
  if (!eventHandler) return handler;
  std::lock_guard<std::mutex> eventHandlerGuard(_eventHandlerMutex);
  for (EventHandlers::iterator i = _eventHandlers.begin(); i != _eventHandlers.end(); ++i) {
    if (i->first == eventHandler) {
      handler = i->second;
      return handler;
    }
  }
  handler.reset(new EventHandler(_currentId++, eventHandler));
  _eventHandlers[eventHandler] = handler;
  return handler;
}

std::vector<PEventHandler> IEventsEx::addEventHandlers(EventHandlers eventHandlers) {
  std::vector<PEventHandler> newHandlers;
  if (eventHandlers.empty()) return newHandlers;
  std::lock_guard<std::mutex> eventHandlerGuard(_eventHandlerMutex);
  for (EventHandlers::iterator i = eventHandlers.begin(); i != eventHandlers.end(); ++i) {
    EventHandlers::iterator handlerIterator = _eventHandlers.find(i->first);
    if (handlerIterator == _eventHandlers.end()) {
      _eventHandlers[i->first] = i->second;
      newHandlers.push_back(i->second);
    } else newHandlers.push_back(handlerIterator->second);
  }
  return newHandlers;
}

void IEventsEx::removeEventHandler(PEventHandler eventHandler) {
  if (!eventHandler) return;

  std::unique_ptr<std::lock_guard<std::mutex>> eventHandlerGuard(new std::lock_guard<std::mutex>(_eventHandlerMutex));
  while (eventHandler->useCount() != 0) {
    eventHandlerGuard.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    eventHandlerGuard.reset(new std::lock_guard<std::mutex>(_eventHandlerMutex));
  }
  EventHandlers::iterator handlerIterator = _eventHandlers.find(eventHandler->handler());
  if (handlerIterator != _eventHandlers.end()) {
    _eventHandlers.erase(eventHandler->handler());
    eventHandler->invalidate();
  }
}

EventHandlers IEventsEx::getEventHandlers() {
  EventHandlers eventHandlers;
  std::lock_guard<std::mutex> eventHandlerGuard(_eventHandlerMutex);
  eventHandlers = _eventHandlers;
  return eventHandlers;
}

}
