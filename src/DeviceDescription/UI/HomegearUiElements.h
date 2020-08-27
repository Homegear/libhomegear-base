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

#ifndef HOMEGEARUIELEMENTS_H_
#define HOMEGEARUIELEMENTS_H_

#include "HomegearUiElement.h"

#include <string>
#include <memory>
#include <unordered_map>

using namespace rapidxml;

namespace BaseLib
{

class SharedObjects;

namespace DeviceDescription
{

class HomegearUiElements;

/**
 * Helper type for HomegearDeviceTranslation pointers.
 */
typedef std::shared_ptr<HomegearUiElements> PHomegearUiElements;

/**
 * Class defining a Homegear device translation. It is a direct representation of the translation XML file.
 */
class HomegearUiElements
{
public:
    HomegearUiElements(BaseLib::SharedObjects* baseLib, std::string xmlFilename);
    virtual ~HomegearUiElements() = default;

    std::unordered_map<std::string, PHomegearUiElement>& getUiElements() { return _uiElements; };

    //{{{ XML entries
    std::string lang;
    //}}}

    bool loaded() { return _loaded; }
protected:
    BaseLib::SharedObjects* _bl = nullptr;
    bool _loaded = false;
    std::unordered_map<std::string, PHomegearUiElement> _uiElements;

    void load(std::string xmlFilename);
    void parseXML(xml_node* node);
};
}
}

#endif
