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

#ifndef LICENSING_H_
#define LICENSING_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace BaseLib
{

class SharedObjects;

namespace Licensing
{

class Licensing
{
public:
	struct DeviceInfo
	{
		int32_t moduleId = -1;
		int32_t familyId = -1;
		int32_t deviceId = -1;
		bool state = false;
		std::string licenseKey;
	} __attribute__((aligned(64)));

	typedef std::shared_ptr<DeviceInfo> PDeviceInfo;
	typedef std::map<int32_t, std::map<int32_t, PDeviceInfo>> DeviceStates;

	explicit Licensing(BaseLib::SharedObjects* bl);
	virtual ~Licensing();

	virtual bool init() = 0;
	virtual void dispose();
	virtual void load();

	virtual int32_t getModuleId() { return _moduleId; }

	/**
	 * Returns a map with all unlicensed devices.
	 * @return Returns a map with all unlicensed devices. The first pair element is the family id the second the device id. When the device id is "-1" the whole device family is not activated.
	 */
	virtual DeviceStates getDeviceStates();

	/**
	 * Returns the activation status of a device or family.
	 * @param familyId The family id.
	 * @param deviceId The device id or -1.
	 * @return Returns true when the device or family is activated otherwise false. When the device is unknown, false is returned.
	 */
	virtual bool getDeviceState(int32_t familyId, int32_t deviceId);

	/**
	 * Checks if a license key is valid.
	 *
	 * @param familyId The family id the license key is for.
	 * @param deviceId The device id the license key is for or -1 if not applicable.
	 * @param licenseKey The license key to check. If empty the mothod reads the license key for this device from the database.
	 * @return Returns -1 on application errors, -2 if the licensing module could not be found, -3 on communication errors, -4 if the activation server returned invalid data, -5 if the license key is invalid, -6 if the license key is valid but blocked, -7 if the activation key validation failed, -8 if no license key was passed and no license key was found in the database, -9 if no license key was passed and the activation key verification failed, -10 if the trial period has expired, 0 if the license key is valid and already known, 1 if the license key is valid and on new activation, 2 on successful trial activation and 3 with valid trial key which is already known.
	 */
	virtual int32_t checkLicense(int32_t familyId, int32_t deviceId, const std::string& licenseKey = "") = 0;

	/**
	 * Removes a license.
	 *
	 * @param familyId The family id the license key is for.
	 * @param deviceId The device id the license key is for or -1 if not applicable.
	 */
	virtual void removeLicense(int32_t familyId, int32_t deviceId);

	/**
	 * Returns the license key stored in the database.
	 *
	 * @param familyId The family id the license key is for.
	 * @param deviceId The device id the license key is for or -1 if not applicable.
	 * @return Returns the license key or an empty string if no license key was found.
	 */
	virtual std::string getLicenseKey(int32_t familyId, int32_t deviceId);

	/**
	 * Returns the start time of a trial version. The default method reads the start time from the license key, if it is the format: "trial<unix timestamp in milliseconds>", e. g. "trial1487608697439".
	 *
	 * @param familyId The family id the license key is for.
	 * @param deviceId The device id the license key is for or -1 if not applicable.
	 * @return Returns the start time of a trial version as unix timestamp in milliseconds or -1 if not applicable.
	 */
	virtual int64_t getTrialStartTime(int32_t familyId, int32_t deviceId);

	virtual void decryptScript(const std::vector<char>& input, std::string& output) = 0;
	virtual void decryptDeviceDescription(const std::vector<char>& input, std::vector<char>& output) = 0;
protected:
	struct LicenseData
	{
		std::string licenseKey;
		std::string activationKey;
	} __attribute__((aligned(64)));

	BaseLib::SharedObjects* _bl = nullptr;
	bool _disposed = false;
	int32_t _moduleId = -1;
	std::map<uint64_t, uint32_t> _variableDatabaseIds;
	std::map<uint64_t, LicenseData> _licenseData;
	std::mutex _devicesMutex;
	DeviceStates _devices;

	uint64_t getMapKey(int32_t familyId, int32_t deviceId);
	virtual void addDevice(int32_t familyId, int32_t deviceId, bool state, std::string licenseKey);
	virtual void removeDevice(int32_t familyId, int32_t deviceId);
	virtual void updateDevice(int32_t familyId, int32_t deviceId, bool state, std::string licenseKey);
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
