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

#include <unistd.h>
#include <sys/socket.h>

namespace BaseLib
{

typedef std::unordered_map<int32_t, PFileDescriptor> FileDescriptors;

struct FileDescriptorManager::OpaquePointer
{
    uint32_t _currentID = 0;
    std::mutex _descriptorsMutex;
    FileDescriptors _descriptors;
};

FileDescriptorManager::FileDescriptorManager() : _opaquePointer(new OpaquePointer())
{

}

FileDescriptorManager::FileDescriptorManager(FileDescriptorManager&& other) noexcept = default;

FileDescriptorManager::~FileDescriptorManager()
{
    dispose();
}

void FileDescriptorManager::dispose()
{
	std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
	for(auto& descriptor : _opaquePointer->_descriptors)
	{
		if(!descriptor.second) continue;
		::close(descriptor.second->descriptor);
	}
    _opaquePointer->_descriptors.clear();
}

PFileDescriptor FileDescriptorManager::add(int32_t fileDescriptor)
{
    std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
    if(fileDescriptor < 0 ) return std::make_shared<FileDescriptor>();
    auto descriptorIterator = _opaquePointer->_descriptors.find(fileDescriptor);
    if(descriptorIterator != _opaquePointer->_descriptors.end())
    {
        PFileDescriptor oldDescriptor = descriptorIterator->second;
        if(oldDescriptor->tlsSession)
        {
            gnutls_deinit(oldDescriptor->tlsSession);
            oldDescriptor->tlsSession = nullptr;
        }
        oldDescriptor->descriptor = -1;
    }
    PFileDescriptor descriptor = std::make_shared<FileDescriptor>();
    descriptor->id = _opaquePointer->_currentID++;
    descriptor->descriptor = fileDescriptor;
    _opaquePointer->_descriptors[fileDescriptor] = descriptor;
    return descriptor;
}

void FileDescriptorManager::remove(PFileDescriptor& descriptor)
{
    if(!descriptor || descriptor->descriptor < 0) return;
    std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
    auto descriptorIterator = _opaquePointer->_descriptors.find(descriptor->descriptor);
    if(descriptorIterator != _opaquePointer->_descriptors.end() && descriptorIterator->second->id == descriptor->id)
    {
        descriptor->descriptor = -1;
        _opaquePointer->_descriptors.erase(descriptor->descriptor);
    }
}

void FileDescriptorManager::close(PFileDescriptor& descriptor)
{
    if(!descriptor || descriptor->descriptor < 0) return;
    std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
    auto descriptorIterator = _opaquePointer->_descriptors.find(descriptor->descriptor);
    if(descriptorIterator != _opaquePointer->_descriptors.end() && descriptorIterator->second->id == descriptor->id)
    {
        _opaquePointer->_descriptors.erase(descriptor->descriptor);
        if(descriptor->tlsSession) gnutls_bye(descriptor->tlsSession, GNUTLS_SHUT_WR);
        ::close(descriptor->descriptor);
        if(descriptor->tlsSession) gnutls_deinit(descriptor->tlsSession);
        descriptor->tlsSession = nullptr;
        descriptor->descriptor = -1;
    }
}

void FileDescriptorManager::shutdown(PFileDescriptor& descriptor)
{
    if(!descriptor || descriptor->descriptor < 0) return;
    std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
    auto descriptorIterator = _opaquePointer->_descriptors.find(descriptor->descriptor);
    if(descriptorIterator != _opaquePointer->_descriptors.end() && descriptorIterator->second && descriptorIterator->second->id == descriptor->id)
    {
        _opaquePointer->_descriptors.erase(descriptor->descriptor);
        if(descriptor->tlsSession) gnutls_bye(descriptor->tlsSession, GNUTLS_SHUT_WR);
        //On SSL connections shutdown is not necessary and might even cause segfaults
        if(!descriptor->tlsSession) ::shutdown(descriptor->descriptor, 0);
        ::close(descriptor->descriptor);
        if(descriptor->tlsSession) gnutls_deinit(descriptor->tlsSession);
        descriptor->tlsSession = nullptr;
        descriptor->descriptor = -1;
    }
}

std::unique_lock<std::mutex> FileDescriptorManager::getLock()
{
	return std::unique_lock<std::mutex>(_opaquePointer->_descriptorsMutex, std::defer_lock);
}

PFileDescriptor FileDescriptorManager::get(int32_t fileDescriptor)
{
    if(fileDescriptor < 0) return PFileDescriptor();
    std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
    auto descriptorIterator = _opaquePointer->_descriptors.find(fileDescriptor);
    if(descriptorIterator != _opaquePointer->_descriptors.end()) return descriptorIterator->second;
}

bool FileDescriptorManager::isValid(int32_t fileDescriptor, int32_t id)
{
    if(fileDescriptor < 0) return false;
    std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
    auto descriptorIterator = _opaquePointer->_descriptors.find(fileDescriptor);
    if(descriptorIterator != _opaquePointer->_descriptors.end() && descriptorIterator->second->id == id) return true;
    return false;
}

bool FileDescriptorManager::isValid(const PFileDescriptor& descriptor)
{
    if(!descriptor || descriptor->descriptor < 0) return false;
    std::lock_guard<std::mutex> descriptorsGuard(_opaquePointer->_descriptorsMutex);
    FileDescriptors::iterator descriptorIterator = _opaquePointer->_descriptors.find(descriptor->descriptor);
    if(descriptorIterator != _opaquePointer->_descriptors.end() && descriptorIterator->second->id == descriptor->id) return true;
	return false;
}

}
