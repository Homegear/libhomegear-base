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

#ifndef BASELIB_SECURITY_SIGN_H_
#define BASELIB_SECURITY_SIGN_H_

#include "../Exception.h"
#include <gnutls/gnutls.h>
#include <gnutls/abstract.h>

#include <memory>
#include <vector>
#include <cstdint>

namespace BaseLib
{
namespace Security
{

/**
 * Exception class for GnuTls.
 *
 * @see GnuTls
 */
class SignException : public Exception
{
public:
    SignException(std::string message) : Exception(message) {}
};

class Sign
{
private:
    gnutls_privkey_t _privateKey = nullptr;
    gnutls_pubkey_t _publicKey = nullptr;
public:
    /**
     * Initializes the GNUTLS key objects.
     *
     * @param privateKey The PEM encoded X509 private key.
     * @param publicKey The PEM encoced X509 public key.
     */
    Sign(const std::string& privateKey, const std::string& publicKey);
    ~Sign();

    /**
     * Signs the given data using the private key.
     *
     * @param data The data to sign.
     * @throw SignException
     * @return Returns the signature.
     */
    std::vector<char> sign(const std::vector<char>& data);

    /**
     * Verifies the signature of the given data using the public key.
     *
     * @param data The data to verify.
     * @param signature The signature of the data generated using the private key.
     * @throw SignException
     * @return Returns "true" if the signature is valid and "false" otherwise.
     */
    bool verify(const std::vector<char>& data, const std::vector<char>& signature);
};

typedef std::shared_ptr<Sign> PSign;

}
}
#endif
