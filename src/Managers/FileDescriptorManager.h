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

#ifndef FILEDESCRIPTORMANAGER_H_
#define FILEDESCRIPTORMANAGER_H_

#include <memory>
#include <mutex>
#include <atomic>

#include <sys/epoll.h>
#include <gnutls/gnutls.h>

namespace BaseLib {

class SharedObjects;

struct FileDescriptor {
  int32_t id = 0;
  std::atomic_int descriptor{-1};
  std::atomic_int epoll_descriptor{-1};
  gnutls_session_t tlsSession = nullptr;
} __attribute__((aligned(16)));

typedef std::shared_ptr<FileDescriptor> PFileDescriptor;

class FileDescriptorManager {
 public:
  FileDescriptorManager();
  FileDescriptorManager(const FileDescriptorManager &) = delete; //Copy constructor
  FileDescriptorManager(FileDescriptorManager &&) noexcept; //Move constructor
  FileDescriptorManager &operator=(const FileDescriptorManager &) = delete; //Copy assignment operator
  ~FileDescriptorManager();
  void dispose();

  PFileDescriptor add(int fileDescriptor);
  PFileDescriptor add(int file_descriptor, int epoll_file_descriptor, uint32_t epoll_events);
  void remove(PFileDescriptor &descriptor);
  void close(PFileDescriptor &descriptor);
  void shutdown(PFileDescriptor &descriptor);
  PFileDescriptor get(int fileDescriptor);
  bool isValid(int fileDescriptor, int32_t id);
  bool isValid(const PFileDescriptor &descriptor);
  int32_t getMax();
  std::unique_lock<std::mutex> getLock();
 private:
  struct OpaquePointer;
  std::unique_ptr<OpaquePointer> opaque_pointer_;

};
}
#endif
