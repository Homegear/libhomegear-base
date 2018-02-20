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

#include "Mac.h"
#include "Gcrypt.h"

namespace BaseLib
{
namespace Security
{

template<typename Data> bool Mac::cmac(const Data& key, const Data& iv, const Data& in, Data& out)
{
    out.clear();
    gcry_mac_hd_t hd;

    gcry_error_t result = gcry_mac_open(&hd, GCRY_MAC_CMAC_AES, GCRY_MAC_FLAG_SECURE, nullptr);
    if(result != GPG_ERR_NO_ERROR || !hd) throw GcryptException(Gcrypt::getError(result));

    result = gcry_mac_setkey(hd, key.data(), key.size());
    if(result != GPG_ERR_NO_ERROR)
    {
        gcry_mac_close(hd);
        throw GcryptException(Gcrypt::getError(result));
    }
    if(!iv.empty())
    {
        result = gcry_mac_setiv(hd, key.data(), key.size());
        if(result != GPG_ERR_NO_ERROR)
        {
            gcry_mac_close(hd);
            throw GcryptException(Gcrypt::getError(result));
        }
    }

    result = gcry_mac_write(hd, in.data(), in.size());
    if(result != GPG_ERR_NO_ERROR)
    {
        gcry_mac_close(hd);
        throw GcryptException(Gcrypt::getError(result));
    }

    out.resize(gcry_mac_get_algo_maclen(GCRY_MAC_CMAC_AES));
    size_t outputSize = out.size();
    result = gcry_mac_read(hd, out.data(), &outputSize);
    if(result != GPG_ERR_NO_ERROR)
    {
        gcry_mac_close(hd);
        throw GcryptException(Gcrypt::getError(result));
    }

    gcry_mac_close(hd);

    if(outputSize != out.size()) return false;

    return true;
}

template bool Mac::cmac<std::vector<char>>(const std::vector<char>& key, const std::vector<char>& iv, const std::vector<char>& in, std::vector<char>& out);
template bool Mac::cmac<std::vector<uint8_t>>(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv, const std::vector<uint8_t>& in, std::vector<uint8_t>& out);

}
}
