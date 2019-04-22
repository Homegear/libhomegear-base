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

#include "Gcrypt.h"
#include "SecureVector.h"

namespace BaseLib
{
namespace Security
{

Gcrypt::Gcrypt(int algorithm, int mode, unsigned int flags) : _algorithm(algorithm), _mode(mode), _flags(flags)
{
	gcry_error_t result = gcry_cipher_open(&_handle, algorithm, mode, flags);
	if(result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
	if(!_handle) throw GcryptException("Could not get handle.");
}

Gcrypt::~Gcrypt()
{
	if(_handle) gcry_cipher_close(_handle);
}

void Gcrypt::reset()
{
    if(_handle) gcry_cipher_close(_handle);
    _handle = nullptr;
    gcry_error_t result = gcry_cipher_open(&_handle, _algorithm, _mode, _flags);
    if(result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
    if(!_handle) throw GcryptException("Could not get handle.");
}

std::string Gcrypt::getError(int32_t errorCode)
{
	std::array<char, 512> result{};
	gpg_strerror_r(errorCode, result.data(), result.size());
	result.back() = 0;
	std::string resultString(result.data());
	return resultString;
}

size_t Gcrypt::getBlockSize()
{
	size_t result = gcry_cipher_get_algo_blklen(_algorithm);
	if (result == 0) throw GcryptException("Could not get block size.");
	return result;
}

template<typename Data> void Gcrypt::setIv(const Data& iv)
{
    if(!_keySet) throw GcryptException("Please set the key first");
	if(iv.empty()) throw GcryptException("iv is empty.");
	setIv(iv.data(), iv.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::setIv<std::vector<char>>(const std::vector<char>& iv);
template void Gcrypt::setIv<std::vector<uint8_t>>(const std::vector<uint8_t>& iv);
template void Gcrypt::setIv<SecureVector<uint8_t>>(const SecureVector<uint8_t>& iv);
template void Gcrypt::setIv<std::array<uint8_t, 16>>(const std::array<uint8_t, 16>& counter);
#endif

void Gcrypt::setIv(const void* iv, const size_t length)
{
	gcry_error_t result = gcry_cipher_setiv(_handle, iv, length);
	if (result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
}

template<typename Data> void Gcrypt::setCounter(const Data& counter)
{
    if(!_keySet) throw GcryptException("Please set the key first");
	if(counter.empty()) throw GcryptException("counter is empty.");
	setCounter(counter.data(), counter.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::setCounter<std::vector<char>>(const std::vector<char>& counter);
template void Gcrypt::setCounter<std::vector<uint8_t>>(const std::vector<uint8_t>& counter);
template void Gcrypt::setCounter<SecureVector<uint8_t>>(const SecureVector<uint8_t>& counter);
template void Gcrypt::setCounter<std::array<uint8_t, 16>>(const std::array<uint8_t, 16>& counter);
#endif

void Gcrypt::setCounter(const void* counter, const size_t length)
{
	gcry_error_t result = gcry_cipher_setctr(_handle, counter, length);
	if (result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
}

template<typename Data> void Gcrypt::setKey(const Data& key)
{
	if(key.empty()) throw GcryptException("key is empty.");
	setKey(key.data(), key.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::setKey<std::vector<char>>(const std::vector<char>& key);
template void Gcrypt::setKey<std::vector<uint8_t>>(const std::vector<uint8_t>& key);
template void Gcrypt::setKey<SecureVector<uint8_t>>(const SecureVector<uint8_t>& key);
template void Gcrypt::setKey<std::array<uint8_t, 32>>(const std::array<uint8_t, 32>& key);
#endif

void Gcrypt::setKey(const void* key, const size_t length)
{
	gcry_error_t result = gcry_cipher_setkey(_handle, key, length);
	if (result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
	_keySet = true;
}

void Gcrypt::encrypt(void* out, const size_t outLength, const void* in, const size_t inLength)
{
	gcry_error_t result = gcry_cipher_encrypt(_handle, out, outLength, in, inLength);
	if (result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
}

template<typename DataOut, typename DataIn> void Gcrypt::encrypt(DataOut& out, const DataIn& in)
{
    if(!_keySet) throw GcryptException("No key set.");
	out.clear();
	if(in.empty()) return;
	out.resize(in.size());
	encrypt((void*)out.data(), out.size(), (void*)in.data(), in.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::encrypt<std::vector<char>, std::vector<char>>(std::vector<char>& out, const std::vector<char>& in);
template void Gcrypt::encrypt<std::vector<uint8_t>, std::vector<uint8_t>>(std::vector<uint8_t>& out, const std::vector<uint8_t>& in);
template void Gcrypt::encrypt<std::vector<char>, std::vector<uint8_t>>(std::vector<char>& out, const std::vector<uint8_t>& in);
template void Gcrypt::encrypt<std::vector<uint8_t>, std::vector<char>>(std::vector<uint8_t>& out, const std::vector<char>& in);
template void Gcrypt::encrypt<std::vector<char>, SecureVector<char>>(std::vector<char>& out, const SecureVector<char>& in);
template void Gcrypt::encrypt<std::vector<uint8_t>, SecureVector<uint8_t>>(std::vector<uint8_t>& out, const SecureVector<uint8_t>& in);
#endif

void Gcrypt::decrypt(void* out, const size_t outLength, const void* in, const size_t inLength)
{
	gcry_error_t result = gcry_cipher_decrypt(_handle, out, outLength, in, inLength);
	if (result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
}

template<typename DataOut, typename DataIn> void Gcrypt::decrypt(DataOut& out, const DataIn& in)
{
    if(!_keySet) throw GcryptException("No key set.");
	out.clear();
	if(in.empty()) return;
	out.resize(in.size());
	decrypt((void*)out.data(), out.size(), (void*)in.data(), in.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::decrypt<std::vector<char>, std::vector<char>>(std::vector<char>& out, const std::vector<char>& in);
template void Gcrypt::decrypt<std::vector<uint8_t>, std::vector<uint8_t>>(std::vector<uint8_t>& out, const std::vector<uint8_t>& in);
template void Gcrypt::decrypt<std::vector<char>, std::vector<uint8_t>>(std::vector<char>& out, const std::vector<uint8_t>& in);
template void Gcrypt::decrypt<std::vector<uint8_t>, std::vector<char>>(std::vector<uint8_t>& out, const std::vector<char>& in);
template void Gcrypt::decrypt<SecureVector<char>, std::vector<char>>(SecureVector<char>& out, const std::vector<char>& in);
template void Gcrypt::decrypt<SecureVector<uint8_t>, std::vector<uint8_t>>(SecureVector<uint8_t>& out, const std::vector<uint8_t>& in);
#endif

bool Gcrypt::authenticate(const void* in, const size_t inLength)
{
    if(!_keySet) throw GcryptException("No key set.");
	gcry_error_t result = gcry_cipher_authenticate(_handle, in, inLength);
	return result == GPG_ERR_NO_ERROR;
}

template<typename DataIn> bool Gcrypt::authenticate(const DataIn& in)
{
	if(in.empty()) return false;
	return authenticate((void*)in.data(), in.size());
}

#ifndef DOXYGEN_SKIP
template bool Gcrypt::authenticate<std::vector<char>>(const std::vector<char>& in);
template bool Gcrypt::authenticate<std::vector<uint8_t>>(const std::vector<uint8_t>& in);
#endif

}
}
