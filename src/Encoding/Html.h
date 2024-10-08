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

#ifndef HTML_H_
#define HTML_H_

#include "../Exception.h"
#include <string>
#include <map>
#include <cstdint>

namespace BaseLib
{

class HtmlException : public BaseLib::Exception
{
public:
	HtmlException(std::string message) : BaseLib::Exception(message) {}
};

class Html
{
public:
	virtual ~Html();

	static void unescapeHtmlEntities(std::string& in, std::string& out);
private:
	typedef std::map<std::string, unsigned int> EntityNameMap;
	typedef std::pair<std::string, unsigned int> EntityNamePair;
	static EntityNameMap _entityNames;

	Html();
};
}
#endif
