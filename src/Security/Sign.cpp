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

#include <gnutls/gnutls.h>
#include "Sign.h"

namespace BaseLib
{
namespace Security
{

Sign::Sign(const std::string& privateKey, const std::string& publicKey)
{
    if(!privateKey.empty())
    {
        if(gnutls_privkey_init(&_privateKey) == GNUTLS_E_SUCCESS)
        {
            gnutls_datum_t data;
            data.data = (unsigned char*) privateKey.data();
            data.size = privateKey.size();

            if(gnutls_privkey_import_x509_raw(_privateKey, &data, gnutls_x509_crt_fmt_t::GNUTLS_X509_FMT_PEM, nullptr, 0) != GNUTLS_E_SUCCESS)
            {
                gnutls_privkey_deinit(_privateKey);
                _privateKey = nullptr;
                return;
            }
        }

    }

    if(!publicKey.empty())
    {
        if(gnutls_pubkey_init(&_publicKey) == GNUTLS_E_SUCCESS)
        {
            gnutls_datum_t data;
            data.data = (unsigned char*) publicKey.data();
            data.size = publicKey.size();

            if(gnutls_pubkey_import_x509_raw(_publicKey, &data, gnutls_x509_crt_fmt_t::GNUTLS_X509_FMT_PEM, 0) != GNUTLS_E_SUCCESS)
            {
                gnutls_privkey_deinit(_privateKey);
                _privateKey = nullptr;
                gnutls_pubkey_deinit(_publicKey);
                _publicKey = nullptr;
                return;
            }
        }
    }
}

Sign::~Sign()
{
    if(_privateKey)
    {
        gnutls_privkey_deinit(_privateKey);
        _privateKey = nullptr;
    }
    if(_publicKey)
    {
        gnutls_pubkey_deinit(_publicKey);
        _publicKey = nullptr;
    }
}

std::vector<char> Sign::sign(const std::vector<char>& data)
{
    if(!_privateKey) throw SignException("Private key is not set.");
    if(!_publicKey) throw SignException("Public key is not set.");

    gnutls_digest_algorithm_t hashAlgorithm;
    if(gnutls_pubkey_get_preferred_hash_algorithm(_publicKey, &hashAlgorithm, 0) != GNUTLS_E_SUCCESS) throw SignException("Error determining hash algorithm.");

    gnutls_datum_t gnutlsData;
    gnutlsData.data = (unsigned char*)data.data();
    gnutlsData.size = data.size();

    gnutls_datum_t gnutlsSignature;
    gnutls_privkey_sign_data(_privateKey, hashAlgorithm, 0, &gnutlsData, &gnutlsSignature);

    try
    {
        std::vector<char> signature(gnutlsSignature.data, gnutlsSignature.data + gnutlsSignature.size);
        gnutls_free(gnutlsSignature.data);
        return signature;
    }
    catch(std::exception& ex)
    {
        gnutls_free(gnutlsSignature.data);
        throw SignException(std::string("Error signing data: ") + ex.what());
    }
}

bool Sign::verify(const std::vector<char>& data, const std::vector<char>& signature)
{
    if(!_publicKey) throw SignException("Public key is not set.");

    gnutls_digest_algorithm_t hashAlgorithm;
    if(gnutls_pubkey_get_preferred_hash_algorithm(_publicKey, &hashAlgorithm, 0) != GNUTLS_E_SUCCESS) throw SignException("Error determining hash algorithm.");

    int result = gnutls_pubkey_get_pk_algorithm(_publicKey, nullptr);
    if(result < 0) throw SignException("Error determining public key algorithm of private key.");

    gnutls_sign_algorithm_t signAlgorithm = gnutls_pk_to_sign((gnutls_pk_algorithm_t)result, hashAlgorithm);
    if(signAlgorithm == GNUTLS_SIGN_UNKNOWN) throw SignException("Error determining signature algorithm.");

    gnutls_datum_t gnutlsData;
    gnutlsData.data = (unsigned char*)data.data();
    gnutlsData.size = data.size();

    gnutls_datum_t gnutlsSignature;
    gnutlsSignature.data = (unsigned char*)signature.data();
    gnutlsSignature.size = signature.size();

    return gnutls_pubkey_verify_data2(_publicKey, signAlgorithm, 0, &gnutlsData, &gnutlsSignature) >= 0;
}

}
}
