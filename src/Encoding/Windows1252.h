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

#ifndef WINDOWS1252_H_
#define WINDOWS1252_H_

#include <string.h>
#include <string>
#include <map>
#include <vector>

namespace BaseLib
{
class Windows1252
{
public:
    Windows1252(bool toUtf8, bool toWindows1252);
    virtual ~Windows1252() {}

    std::string toUtf8(const std::string& windows1252String);
    std::string toUtf8(const char* windows1252String, uint32_t length);
    std::string toWindows1252(const std::string& utf8String);
    std::string toWindows1252(const char* utf8String, uint32_t length);
protected:
    bool _toUtf8 = false;
    bool _toWindows1252 = false;

    std::vector<std::vector<uint8_t>> _utf8Lookup;
    std::map<uint32_t, uint8_t> _windows1252Lookup;
};
}
#endif
