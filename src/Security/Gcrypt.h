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

#ifndef BASELIB_SECURITY_GCRYPT_H_
#define BASELIB_SECURITY_GCRYPT_H_

#include "../Exception.h"
#include <gcrypt.h>

#include <memory>
#include <vector>
#include <cstdint>

namespace BaseLib
{
namespace Security
{

/**
 * Exception class for Gcrypt.
 *
 * @see Gcrypt
 */
class GcryptException : public Exception
{
public:
	GcryptException(std::string message) : Exception(message) {}
};

class Gcrypt
{
private:
	/**
	 * Set to true if key is set.
	 */
	bool _keySet = false;

	/**
	 * The algorithm used.
	 */
	int _algorithm = 0;

	/**
	 * The Gcrypt handle
	 */
	gcry_cipher_hd_t _handle = nullptr;
public:
	/**
	 * Constructor. Takes the same arguments as gcry_cipher_open().
	 */
	Gcrypt(int algorithm, int mode, unsigned int flags);
	virtual ~Gcrypt();

	/**
	 * Gets the error message to a GCRYPT error code.
	 *
	 * @param errorCode The GCRYPT error code.
	 * @return Returns the error message for the provided error code.
	 */
	static std::string getError(int32_t errorCode);

	/**
	 * Returns the block size of the algorithm used in bytes.
	 */
	size_t getBlockSize();

	/**
	 * Sets the IV to use.
	 */
	template<typename Data> void setIv(const Data& iv);

	/**
	 * Takes same parameters as gcry_cipher_setiv() except for the handle.
	 */
	void setIv(const void* iv, const size_t length);

	/**
	 * Sets the key to use.
	 */
	template<typename Data> void setKey(const Data& key);

	/**
	 * Takes same parameters as gcry_cipher_setkey() except for the handle.
	 */
	void setKey(const void* key, const size_t length);

	/**
	 * Encrypts data. Takes same parameters as gcry_cipher_encrypt() except for the handle.
	 */
	void encrypt(void* out, const size_t outLength, const void* in, const size_t inLength);

	/**
	 * Encrypt data.
	 */
	template<typename DataOut, typename DataIn> void encrypt(DataOut& out, const DataIn& in);

	/**
	 * Encrypts data. Takes same parameters as gcry_cipher_decrypt() except for the handle.
	 */
	void decrypt(void* out, const size_t outLength, const void* in, const size_t inLength);

	/**
	 * Encrypt data.
	 */
	template<typename DataOut, typename DataIn> void decrypt(DataOut& out, const DataIn& in);
};

typedef std::shared_ptr<Gcrypt> PGcrypt;

}
}
#endif
