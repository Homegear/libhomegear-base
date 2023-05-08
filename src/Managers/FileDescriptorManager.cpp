#include <memory>

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

#include "FileDescriptorManager.h"

#include <string>
#include <unordered_map>
#include <atomic>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/resource.h>

namespace BaseLib {

FileDescriptor::~FileDescriptor() {
  if (tlsSession) gnutls_deinit(tlsSession);

  if (epoll_descriptor != -1) {
    epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, closed_descriptor_, nullptr);
  }

  tlsSession = nullptr;
}

void FileDescriptor::Invalidate() {
  closed_descriptor_ = descriptor.load();
  descriptor = -1;
}

void FileDescriptor::Close() {
  if (tlsSession) gnutls_bye(tlsSession, GNUTLS_SHUT_WR);
  ::close(descriptor);
  closed_descriptor_ = descriptor.load();
  descriptor = -1;
}

void FileDescriptor::Shutdown() {
  if (tlsSession) gnutls_bye(tlsSession, GNUTLS_SHUT_WR);
  //On SSL connections shutdown is not necessary and might even cause segfaults
  if (!tlsSession) ::shutdown(descriptor, SHUT_WR);
  ::close(descriptor);
  closed_descriptor_ = descriptor.load();
  descriptor = -1;
}

typedef std::unordered_map<int32_t, PFileDescriptor> FileDescriptors;

struct FileDescriptorManager::OpaquePointer {
  uint32_t _currentId = 0;
  std::atomic_int _maxFd{0};
  std::mutex _descriptorsMutex;
  FileDescriptors _descriptors;
};

FileDescriptorManager::FileDescriptorManager() : opaque_pointer_(new OpaquePointer()) {

}

FileDescriptorManager::FileDescriptorManager(FileDescriptorManager &&other) noexcept = default;

FileDescriptorManager::~FileDescriptorManager() {
  dispose();
}

void FileDescriptorManager::dispose() {
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  for (auto &descriptor: opaque_pointer_->_descriptors) {
    if (!descriptor.second) continue;
    descriptor.second->Close();
  }
  opaque_pointer_->_descriptors.clear();
}

PFileDescriptor FileDescriptorManager::add(int fileDescriptor) {
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  if (fileDescriptor < 0) return std::make_shared<FileDescriptor>();
  auto descriptorIterator = opaque_pointer_->_descriptors.find(fileDescriptor);
  if (descriptorIterator != opaque_pointer_->_descriptors.end()) {
    PFileDescriptor oldDescriptor = descriptorIterator->second;
    if (oldDescriptor->tlsSession) {
      gnutls_deinit(oldDescriptor->tlsSession);
      oldDescriptor->tlsSession = nullptr;
    }
    oldDescriptor->descriptor = -1;
  }
  PFileDescriptor descriptor = std::make_shared<FileDescriptor>();
  descriptor->id = opaque_pointer_->_currentId++;
  descriptor->descriptor = fileDescriptor;
  opaque_pointer_->_descriptors[fileDescriptor] = descriptor;
  if (fileDescriptor > opaque_pointer_->_maxFd) opaque_pointer_->_maxFd.store(fileDescriptor, std::memory_order_relaxed);
  fcntl(fileDescriptor, F_SETFD, fcntl(fileDescriptor, F_GETFD) | FD_CLOEXEC);
  return descriptor;
}

PFileDescriptor FileDescriptorManager::add(int file_descriptor, int epoll_file_descriptor, uint32_t epoll_events) {
  std::lock_guard<std::mutex> descriptors_guard(opaque_pointer_->_descriptorsMutex);
  if (file_descriptor < 0) return std::make_shared<FileDescriptor>();
  auto descriptor_iterator = opaque_pointer_->_descriptors.find(file_descriptor);
  if (descriptor_iterator != opaque_pointer_->_descriptors.end()) {
    PFileDescriptor old_descriptor = descriptor_iterator->second;
    old_descriptor->Invalidate();
  }
  PFileDescriptor descriptor = std::make_shared<FileDescriptor>();
  while (descriptor->id == 0) {
    descriptor->id = opaque_pointer_->_currentId++;
  }
  descriptor->descriptor = file_descriptor;
  opaque_pointer_->_descriptors[file_descriptor] = descriptor;
  if (file_descriptor > opaque_pointer_->_maxFd) opaque_pointer_->_maxFd.store(file_descriptor, std::memory_order_relaxed);
  fcntl(file_descriptor, F_SETFD, fcntl(file_descriptor, F_GETFD) | FD_CLOEXEC);

  if (epoll_file_descriptor != -1) {
    descriptor->epoll_descriptor = epoll_file_descriptor;
    struct epoll_event event{};
    event.events = epoll_events;
    event.data.fd = file_descriptor;
    epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, file_descriptor, &event);
  }

  return descriptor;
}

void FileDescriptorManager::remove(PFileDescriptor &descriptor) {
  if (!descriptor || descriptor->descriptor == -1) return;
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  auto descriptorIterator = opaque_pointer_->_descriptors.find(descriptor->descriptor);
  if (descriptorIterator != opaque_pointer_->_descriptors.end() && descriptorIterator->second->id == descriptor->id) {
    descriptor->Close();
    opaque_pointer_->_descriptors.erase(descriptor->descriptor);
  }
}

void FileDescriptorManager::close(PFileDescriptor &descriptor) {
  if (!descriptor || descriptor->descriptor == -1) return;
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  auto descriptorIterator = opaque_pointer_->_descriptors.find(descriptor->descriptor);
  if (descriptorIterator != opaque_pointer_->_descriptors.end() && descriptorIterator->second->id == descriptor->id) {
    opaque_pointer_->_descriptors.erase(descriptor->descriptor);
    descriptor->Close();
  }
}

void FileDescriptorManager::shutdown(PFileDescriptor &descriptor) {
  if (!descriptor || descriptor->descriptor == -1) return;
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  auto descriptorIterator = opaque_pointer_->_descriptors.find(descriptor->descriptor);
  if (descriptorIterator != opaque_pointer_->_descriptors.end() && descriptorIterator->second && descriptorIterator->second->id == descriptor->id) {
    opaque_pointer_->_descriptors.erase(descriptor->descriptor);
    descriptor->Shutdown();
  }
}

std::unique_lock<std::mutex> FileDescriptorManager::getLock() {
  return {opaque_pointer_->_descriptorsMutex, std::defer_lock};
}

PFileDescriptor FileDescriptorManager::get(int fileDescriptor) {
  if (fileDescriptor < 0) return {};
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  auto descriptorIterator = opaque_pointer_->_descriptors.find(fileDescriptor);
  if (descriptorIterator != opaque_pointer_->_descriptors.end()) return descriptorIterator->second;
  return {};
}

bool FileDescriptorManager::isValid(int fileDescriptor, int32_t id) {
  if (fileDescriptor < 0) return false;
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  auto descriptorIterator = opaque_pointer_->_descriptors.find(fileDescriptor);
  if (descriptorIterator != opaque_pointer_->_descriptors.end() && descriptorIterator->second->id == id) return true;
  return false;
}

bool FileDescriptorManager::isValid(const PFileDescriptor &descriptor) {
  if (!descriptor || descriptor->descriptor < 0) return false;
  std::lock_guard<std::mutex> descriptorsGuard(opaque_pointer_->_descriptorsMutex);
  auto descriptorIterator = opaque_pointer_->_descriptors.find(descriptor->descriptor);
  if (descriptorIterator != opaque_pointer_->_descriptors.end() && descriptorIterator->second->id == descriptor->id) return true;
  return false;
}

int32_t FileDescriptorManager::getMax() {
  //Child process
  struct rlimit limits{};
  if (getrlimit(RLIMIT_NOFILE, &limits) == -1 || limits.rlim_cur >= INT32_MAX) {
    return opaque_pointer_->_maxFd.load(std::memory_order_relaxed) + 1024;
  }
  return limits.rlim_cur;
}

}
