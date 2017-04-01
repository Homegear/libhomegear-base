/* Copyright 2013-2017 Sathya Laufer
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

namespace BaseLib
{
namespace Security
{

Gcrypt::Gcrypt(int algorithm, int mode, unsigned int flags) : _algorithm(algorithm)
{
	gcry_error_t result = gcry_cipher_open(&_handle, algorithm, mode, flags);
	if(result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
	if(!_handle) throw GcryptException("Could not get handle.");
}

Gcrypt::~Gcrypt()
{
	if(_handle) gcry_cipher_close(_handle);
}

std::string Gcrypt::getError(int32_t errorCode)
{
	std::string result("", 512);
	gpg_strerror_r(errorCode, &result[0], result.size());
	return result;
}

size_t Gcrypt::getBlockSize()
{
	size_t result = gcry_cipher_get_algo_blklen(_algorithm);
	if (result == 0) throw GcryptException("Could not get block size.");
	return result;
}

template<typename Data> void Gcrypt::setIv(const Data& iv)
{
	if(iv.empty()) throw GcryptException("iv is empty.");
	setIv(&iv[0], iv.size());
}


#ifndef DOXYGEN_SKIP
template void Gcrypt::setIv<std::vector<char>>(const std::vector<char>& iv);
template void Gcrypt::setIv<std::vector<uint8_t>>(const std::vector<uint8_t>& iv);
#endif

void Gcrypt::setIv(const void* iv, const size_t length)
{
	gcry_error_t result = gcry_cipher_setiv(_handle, iv, length);
	if (result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
}

template<typename Data> void Gcrypt::setKey(const Data& key)
{
	if(key.empty()) throw GcryptException("key is empty.");
	setKey(&key[0], key.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::setKey<std::vector<char>>(const std::vector<char>& key);
template void Gcrypt::setKey<std::vector<uint8_t>>(const std::vector<uint8_t>& key);
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
	out.clear();
	if(in.empty()) return;
	out.resize(in.size());
	encrypt(&out[0], out.size(), &in[0], in.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::encrypt<std::vector<char>, std::vector<char>>(std::vector<char>& out, const std::vector<char>& in);
template void Gcrypt::encrypt<std::vector<uint8_t>, std::vector<uint8_t>>(std::vector<uint8_t>& out, const std::vector<uint8_t>& in);
template void Gcrypt::encrypt<std::vector<char>, std::vector<uint8_t>>(std::vector<char>& out, const std::vector<uint8_t>& in);
template void Gcrypt::encrypt<std::vector<uint8_t>, std::vector<char>>(std::vector<uint8_t>& out, const std::vector<char>& in);
#endif

void Gcrypt::decrypt(void* out, const size_t outLength, const void* in, const size_t inLength)
{
	gcry_error_t result = gcry_cipher_decrypt(_handle, out, outLength, in, inLength);
	if (result != GPG_ERR_NO_ERROR) throw GcryptException(getError(result));
}

template<typename DataOut, typename DataIn> void Gcrypt::decrypt(DataOut& out, const DataIn& in)
{
	out.clear();
	if(in.empty()) return;
	out.resize(in.size());
	decrypt(&out[0], out.size(), &in[0], in.size());
}

#ifndef DOXYGEN_SKIP
template void Gcrypt::decrypt<std::vector<char>, std::vector<char>>(std::vector<char>& out, const std::vector<char>& in);
template void Gcrypt::decrypt<std::vector<uint8_t>, std::vector<uint8_t>>(std::vector<uint8_t>& out, const std::vector<uint8_t>& in);
template void Gcrypt::decrypt<std::vector<char>, std::vector<uint8_t>>(std::vector<char>& out, const std::vector<uint8_t>& in);
template void Gcrypt::decrypt<std::vector<uint8_t>, std::vector<char>>(std::vector<uint8_t>& out, const std::vector<char>& in);
#endif

}
}
