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

#include "UiControl.h"
#include "HomegearUiElement.h"
#include "../../BaseLib.h"

namespace BaseLib
{
namespace DeviceDescription
{

UiControl::UiControl(BaseLib::SharedObjects* baseLib)
{
    _bl = baseLib;
}

UiControl::UiControl(BaseLib::SharedObjects* baseLib, xml_node<>* node) : UiControl(baseLib)
{
    for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
    {
        std::string attributeName(attr->name());
        std::string attributeValue(attr->value());
        if(attributeName == "id")
        {
            id = attributeValue;
        }
        else _bl->out.printWarning("Warning: Unknown attribute for \"control\": " + attributeName);
    }
    for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
    {
        std::string name(subNode->name());
        std::string value(subNode->value());
        if(name == "x") x = Math::getNumber(value);
        else if(name == "y") y = Math::getNumber(value);
        else if(name == "columns") columns = Math::getNumber(value);
        else if(name == "rows") rows = Math::getNumber(value);
        else if(name == "metadata")
        {
            for(xml_node<>* metadataNode = subNode->first_node(); metadataNode; metadataNode = metadataNode->next_sibling())
            {
                std::string metadataNodeName(metadataNode->name());
                metadata.emplace(metadataNodeName, std::string(metadataNode->value()));
            }
        }
        else _bl->out.printWarning("Warning: Unknown node in \"control\": " + name);
    }
}

UiControl::UiControl(UiControl const& rhs)
{
    _bl = rhs._bl;

    id = rhs.id;
    x = rhs.x;
    y = rhs.y;
    columns = rhs.columns;
    rows = rhs.rows;
    metadata = rhs.metadata;

    if(rhs.uiElement)
    {
        uiElement = std::make_shared<HomegearUiElement>(_bl);
        *uiElement = *rhs.uiElement;
    }
}

UiControl& UiControl::operator=(const UiControl& rhs)
{
    if(&rhs == this) return *this;

    _bl = rhs._bl;

    id = rhs.id;
    x = rhs.x;
    y = rhs.y;
    columns = rhs.columns;
    rows = rhs.rows;
    metadata = rhs.metadata;

    if(rhs.uiElement)
    {
        uiElement = std::make_shared<HomegearUiElement>(_bl);
        *uiElement = *rhs.uiElement;
    }

    return *this;
}

}
}
