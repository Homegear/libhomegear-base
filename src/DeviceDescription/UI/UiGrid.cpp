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

#include "UiGrid.h"
#include "../../BaseLib.h"

namespace BaseLib
{
namespace DeviceDescription
{

UiGrid::UiGrid(BaseLib::SharedObjects* baseLib)
{
    _bl = baseLib;
}

UiGrid::UiGrid(BaseLib::SharedObjects* baseLib, xml_node<>* node) : UiGrid(baseLib)
{
    for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
    {
        _bl->out.printWarning("Warning: Unknown attribute for \"condition\": " + std::string(attr->name()));
    }
    for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
    {
        std::string name(subNode->name());
        std::string value(subNode->value());
        if(name == "width") width = Math::getNumber(value);
        else if(name == "height") height = Math::getNumber(value);
        else if(name == "columns") columns = Math::getNumber(value);
        else if(name == "rows") rows = Math::getNumber(value);
        else _bl->out.printWarning("Warning: Unknown node in \"condition\": " + name);
    }
}

UiGrid::UiGrid(UiGrid const& rhs)
{
    _bl = rhs._bl;

    width = rhs.width;
    height = rhs.height;
    columns = rhs.columns;
    rows = rhs.rows;
}

UiGrid& UiGrid::operator=(const UiGrid& rhs)
{
    if(&rhs == this) return *this;

    _bl = rhs._bl;

    width = rhs.width;
    height = rhs.height;
    columns = rhs.columns;
    rows = rhs.rows;

    return *this;
}

}
}
