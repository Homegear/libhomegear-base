/* Copyright 2013-2016 Sathya Laufer
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Homegear.  If not, see
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

#ifndef SYSTEMSPHYSICALINTERFACES_H_
#define SYSTEMSPHYSICALINTERFACES_H_

#include "../Variable.h"
#include "PhysicalInterfaceSettings.h"

#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <vector>

namespace BaseLib
{

class Obj;

namespace Systems
{

class IPhysicalInterface;

class PhysicalInterfaces
{
public:
	PhysicalInterfaces(BaseLib::Obj* bl, int32_t familyId, std::vector<std::shared_ptr<PhysicalInterfaceSettings>> physicalInterfaceSettings);
	virtual ~PhysicalInterfaces();
	void dispose();

	uint32_t count();
	void clear();
	void stopListening();
	void startListening();
	bool lifetick();
	bool isOpen();
	void setup(int32_t userID, int32_t groupID);
	PVariable listInterfaces(int32_t centralAddress);
protected:
	BaseLib::Obj* _bl = nullptr;
	int32_t _familyId = -1;
	std::vector<std::shared_ptr<PhysicalInterfaceSettings>> _physicalInterfaceSettings;
	std::mutex _physicalInterfacesMutex;
	std::map<std::string, std::shared_ptr<IPhysicalInterface>> _physicalInterfaces;

	virtual void create() {};
};

}

}
#endif
