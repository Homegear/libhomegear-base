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

#ifndef HASH_H_
#define HASH_H_

namespace BaseLib
{
namespace Security
{

class Hash
{
public:
	/**
	 * Destructor.
	 * Does nothing.
	 */
	virtual ~Hash();

	/**
	 * Calculates the SHA1 of the passed binary data.
	 *
	 * @param[in] in The data to calculate the SHA1 for.
	 * @param[out] out A vector to store the calculated SHA1 in.
	 * @return Returns "true" on success and "false" on error.
	 */
	template<typename Data> static bool sha1(const Data& in, Data& out);

	/**
	 * Calculates the SHA256 of the passed binary data.
	 *
	 * @param[in] in The data to calculate the SHA256 for.
	 * @param[out] out A vector to store the calculated SHA256 in.
	 * @return Returns "true" on success and "false" on error.
	 */
	template<typename Data> static bool sha256(const Data& in, Data& out);

	/**
	 * Calculates the MD5 of the passed binary data.
	 *
	 * @param[in] in The data to calculate the MD5 for.
	 * @param[out] out A vector to store the calculated MD5 in.
	 * @return Returns "true" on success and "false" on error.
	 */
	template<typename Data> static bool md5(const Data& in, Data& out);
protected:
	/**
	 * Constructor. It is protected, because the class only contains static methods.
	 * It does nothing.
	 */
	Hash();
};

}
}
#endif
