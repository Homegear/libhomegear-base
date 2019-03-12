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

#ifndef DEVICETRANSLATIONS_H_
#define DEVICETRANSLATIONS_H_

#include <vector>
#include <memory>
#include <mutex>

#include "HomegearDeviceTranslation.h"
#include "ParameterGroup.h"

namespace BaseLib
{

class SharedObjects;

namespace DeviceDescription
{

/**
 * Class to work with translations of device description files of one device family. It is used to load all translations and retrieve the translation of a device.
 */
class DeviceTranslations
{
public:
	DeviceTranslations(BaseLib::SharedObjects* baseLib, int32_t family);
	virtual ~DeviceTranslations() = default;
	void clear();
    std::string getTypeDescription(const std::string& filename, std::string& language, const std::string& deviceId);
    std::string getTypeLongDescription(const std::string& filename, std::string& language, const std::string& deviceId);
    std::pair<std::string, std::string> getParameterTranslations(const std::string& filename, std::string& language, ParameterGroup::Type::Enum parameterGroupType, const std::string& parameterGroupId, const std::string& parameterId);
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	int32_t _family = -1;
    std::mutex _deviceTranslationsMutex;
	std::unordered_map<std::string, std::unordered_map<std::string, PHomegearDeviceTranslation>> _deviceTranslations;

    PHomegearDeviceTranslation getTranslation(const std::string& filename, std::string& language);
	PHomegearDeviceTranslation load(const std::string& filename, std::string& language);
};

}
}
#endif
