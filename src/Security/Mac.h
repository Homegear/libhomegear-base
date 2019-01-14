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

#ifndef MAC_H_
#define MAC_H_

namespace BaseLib
{
namespace Security
{

class Mac
{
public:
    /**
     * Destructor.
     * Does nothing.
     */
    virtual ~Mac();

    /**
     * Calculates the CMAC of the passed binary data.
     *
     * @param[in] key The AES key to use for CMAC calculation.
     * @param[in] iv The AES IV to use for CMAC calculation. Can be an empty array.
     * @param[in] in The data to calculate the CMAC for.
     * @param[out] out A vector to store the calculated MAC in.
     * @return Returns "true" on success and "false" on error.
     * @throws GcryptException On errors.
     */
    template<typename Data> static bool cmac(const Data& key, const Data& iv, const Data& in, Data& out);
protected:
    /**
     * Constructor. It is protected, because the class only contains static methods.
     * It does nothing.
     */
    Mac();
};

}
}
#endif
