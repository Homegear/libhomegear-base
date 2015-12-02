/* Copyright 2013-2015 Sathya Laufer
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

#ifndef LICENSING_H_
#define LICENSING_H_

#include <string>
#include <vector>
#include <map>

namespace BaseLib
{

class Obj;

namespace Licensing
{

class Licensing
{
public:
	Licensing(BaseLib::Obj* bl);
	virtual ~Licensing();

	virtual bool init() = 0;
	virtual void dispose();
	virtual void load();

	virtual int32_t getModuleId() { return _moduleId; }

	/**
	 * Checks if a license key is valid.
	 *
	 * @param familyId The family id the license key is for.
	 * @param deviceId The device id the license key is for or -1 if not applicable.
	 * @param licenseKey The license key to check. If empty the mothod reads the license key for this device from the database.
	 * @return Returns -1 on application errors, -2 if the licensing module could not be found, -3 on communication errors, -4 if the activation server returned invalid data, -5 if the license key is invalid, -6 if the license key is valid but blocked, -7 if the activation key validation failed, -8 if no license key was passed and no license key was found in the database, -9 if no license key was passed and the activation key verification failed 0 if the license key is valid and already known and 1 if the license key is valid and on new activation.
	 */
	virtual int32_t checkLicense(int32_t familyId, int32_t deviceId, const std::string& licenseKey = "") = 0;
	virtual void decryptScript(const std::vector<char>& input, std::string& output) = 0;
	virtual void decryptDeviceDescription(const std::vector<char>& input, std::vector<char>& output) = 0;
protected:
	struct LicenseData
	{
		std::string licenseKey;
		std::string activationKey;
	};

	BaseLib::Obj* _bl = nullptr;
	bool _disposed = false;
	int32_t _moduleId = -1;
	std::map<uint32_t, uint32_t> _variableDatabaseIds;
	std::map<uint64_t, LicenseData> _licenseData;

	virtual void loadVariables();
	virtual void saveVariable(uint64_t index, int32_t intValue);
    virtual void saveVariable(uint64_t index, int64_t intValue);
    virtual void saveVariable(uint64_t index, std::string& stringValue);
    virtual void saveVariable(uint64_t index, std::vector<uint8_t>& binaryValue);
    virtual void saveVariable(uint64_t index, LicenseData& licenseData);
};

}
}

#endif
