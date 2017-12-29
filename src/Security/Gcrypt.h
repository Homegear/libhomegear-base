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
	 *
	 * @param algorithm See https://www.gnupg.org/documentation/manuals/gcrypt/Available-ciphers.html#Available-ciphers for a list of ciphers and the according constants.
	 * @param mode See https://www.gnupg.org/documentation/manuals/gcrypt/Available-cipher-modes.html#Available-cipher-modes. Note that the mode needs to be compatible to the algorithm used.
	 * @param flags 0 or the bit-wise OR of the following constants: GCRY_CIPHER_SECURE (allocate everything in secure memory), GCRY_CIPHER_ENABLE_SYNC (CFB sync mode for OpenPGP), GCRY_CIPHER_CBC_CTS (Enable cipher text stealing for CBC), GCRY_CIPHER_CBC_MAC (compute CBC-MAC checksums, same as CBC but only outputs the last block)
	 * @throws GcryptException On error.
	 */
	Gcrypt(int algorithm, int mode, unsigned int flags);

	/**
	 * Destructor.
	 */
	virtual ~Gcrypt();

	/**
	 * Returns the underlying gcry_cipher_hd_t.
	 *
	 * @return Returns the underlying gcry_cipher_hd_t.
	 */
	gcry_cipher_hd_t getHandle() { return _handle; };

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
	 *
	 * @throws GcryptException On error.
	 */
	template<typename Data> void setIv(const Data& iv);

	/**
	 * Takes same parameters as gcry_cipher_setiv() except for the handle.
	 *
	 * @throws GcryptException On error.
	 */
	void setIv(const void* iv, const size_t length);

	/**
	 * Sets the counter to use.
	 *
	 * @throws GcryptException On error.
	 */
	template<typename Data> void setCounter(const Data& counter);

	/**
	 * Takes same parameters as gcry_cipher_setctr() except for the handle.
	 *
	 * @throws GcryptException On error.
	 */
	void setCounter(const void* counter, const size_t length);

	/**
	 * Sets the key to use.
	 *
	 * @throws GcryptException On error.
	 */
	template<typename Data> void setKey(const Data& key);

	/**
	 * Takes same parameters as gcry_cipher_setkey() except for the handle.
	 *
	 * @throws GcryptException On error.
	 */
	void setKey(const void* key, const size_t length);

	/**
	 * Encrypts data. Takes same parameters as gcry_cipher_encrypt() except for the handle.
	 *
	 * @throws GcryptException On error.
	 */
	void encrypt(void* out, const size_t outLength, const void* in, const size_t inLength);

	/**
	 * Encrypt data.
	 *
	 * @throws GcryptException On error.
	 */
	template<typename DataOut, typename DataIn> void encrypt(DataOut& out, const DataIn& in);

	/**
	 * Decrypts data. Takes same parameters as gcry_cipher_decrypt() except for the handle.
	 *
	 * @throws GcryptException On error.
	 */
	void decrypt(void* out, const size_t outLength, const void* in, const size_t inLength);

	/**
	 * Decrypt data.
	 *
	 * @throws GcryptException On error.
	 */
	template<typename DataOut, typename DataIn> void decrypt(DataOut& out, const DataIn& in);

	/**
	 * Authenticates encrypted data if supported by the algorithm. Takes same parameters as gcry_cipher_authenticate() except for the handle.
	 */
	bool authenticate(const void* in, const size_t inLength);

	/**
	 * Authenticates encrypted data.
	 */
	template<typename DataIn> bool authenticate(const DataIn& in);
};

typedef std::shared_ptr<Gcrypt> PGcrypt;

}
}
#endif
