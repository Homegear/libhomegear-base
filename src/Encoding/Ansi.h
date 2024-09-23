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

#ifndef ANSI_H_
#define ANSI_H_

#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <cstdint>

namespace BaseLib
{
class Ansi
{
public:
	Ansi(bool ansiToUtf8, bool utf8ToAnsi);
	virtual ~Ansi() {}

	std::string toUtf8(const std::string& ansiString);
	std::string toUtf8(const char* ansiString, uint32_t length);
	std::string toAnsi(const std::string& utf8String);
	std::string toAnsi(const char* utf8String, uint32_t length);
protected:
	bool _ansiToUtf8 = false;
	bool _utf8ToAnsi = false;

	std::vector<std::vector<uint8_t>> _utf8Lookup;
	std::map<uint32_t, uint8_t> _ansiLookup;
};
}
#endif
