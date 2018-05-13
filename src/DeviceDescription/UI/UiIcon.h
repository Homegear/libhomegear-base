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

#ifndef UIICON_H_
#define UIICON_H_

#include "../../Encoding/RapidXml/rapidxml.hpp"
#include <string>
#include <map>
#include <memory>

using namespace rapidxml;

namespace BaseLib
{

class SharedObjects;

namespace DeviceDescription
{

class UiIcon;

typedef std::shared_ptr<UiIcon> PUiIcon;

class UiIcon
{
public:
    UiIcon(BaseLib::SharedObjects* baseLib);
    UiIcon(BaseLib::SharedObjects* baseLib, xml_node<>* node);
    UiIcon(UiIcon const& rhs);
    virtual ~UiIcon() = default;

    UiIcon& operator=(const UiIcon& rhs);

    //Elements
    std::string name;
    std::string conditionOperator;
    std::string conditionValue;
protected:
    BaseLib::SharedObjects* _bl = nullptr;
};

}
}

#endif
