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

#include "PhysicalInterfaces.h"
#include "IPhysicalInterface.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Systems
{

PhysicalInterfaces::PhysicalInterfaces(BaseLib::Obj* bl, int32_t familyId, std::vector<std::shared_ptr<PhysicalInterfaceSettings>> physicalInterfaceSettings)
{
	try
	{
		_bl = bl;
		_familyId = familyId;
		_physicalInterfaceSettings = physicalInterfaceSettings;
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

PhysicalInterfaces::~PhysicalInterfaces()
{
}

void PhysicalInterfaces::dispose()
{
	_physicalInterfacesMutex.lock();
	_physicalInterfaceSettings.clear();
	_physicalInterfaces.clear();
	_physicalInterfacesMutex.unlock();
}

bool PhysicalInterfaces::lifetick()
{
	try
	{
		_physicalInterfacesMutex.lock();
		for(std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j)
		{
			if(!j->second->lifetick())
			{
				_physicalInterfacesMutex.unlock();
				return false;
			}
		}
		_physicalInterfacesMutex.unlock();
		return true;
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    _physicalInterfacesMutex.unlock();
    return false;
}

uint32_t PhysicalInterfaces::count()
{
	uint32_t size = 0;
	try
	{
		_physicalInterfacesMutex.lock();
		size = _physicalInterfaces.size();
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    _physicalInterfacesMutex.unlock();
    return size;
}

bool PhysicalInterfaces::isOpen()
{
	try
	{
		if(_physicalInterfaces.empty()) return true;
		_physicalInterfacesMutex.lock();
		for(std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j)
		{
			if(!j->second->isNetworkDevice() && !j->second->isOpen())
			{
				_physicalInterfacesMutex.unlock();
				return false;
			}
		}
		_physicalInterfacesMutex.unlock();
		return true;
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    _physicalInterfacesMutex.unlock();
	return false;
}

void PhysicalInterfaces::startListening()
{
	try
	{
		_physicalInterfacesMutex.lock();
		for(std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j)
		{
			j->second->startListening();
		}
		_physicalInterfacesMutex.unlock();
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void PhysicalInterfaces::stopListening()
{
	try
	{
		_physicalInterfacesMutex.lock();
		for(std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j)
		{
			j->second->stopListening();
		}
		_physicalInterfacesMutex.unlock();
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void PhysicalInterfaces::setup(int32_t userID, int32_t groupID)
{
	try
	{
		_physicalInterfacesMutex.lock();
		for(std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j)
		{
			if(!j->second)
			{
				_bl->out.printCritical("Critical: Could not setup device: Device pointer was empty.");
				continue;
			}
			_bl->out.printDebug("Debug: Setting up physical device.");
			j->second->setup(userID, groupID);
		}
		_physicalInterfacesMutex.unlock();
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

BaseLib::PVariable PhysicalInterfaces::listInterfaces(int32_t centralAddress)
{
	try
	{
		BaseLib::PVariable array(new BaseLib::Variable(BaseLib::VariableType::tArray));

		for(std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j)
		{
			BaseLib::PVariable interface(new BaseLib::Variable(BaseLib::VariableType::tStruct));

			interface->structValue->insert(BaseLib::StructElement("FAMILYID", BaseLib::PVariable(new BaseLib::Variable(_familyId))));
			interface->structValue->insert(BaseLib::StructElement("ID", BaseLib::PVariable(new BaseLib::Variable(j->second->getID()))));
			interface->structValue->insert(BaseLib::StructElement("PHYSICALADDRESS", BaseLib::PVariable(new BaseLib::Variable(centralAddress))));
			interface->structValue->insert(BaseLib::StructElement("TYPE", BaseLib::PVariable(new BaseLib::Variable(j->second->getType()))));
			interface->structValue->insert(BaseLib::StructElement("CONNECTED", BaseLib::PVariable(new BaseLib::Variable(j->second->isOpen()))));
			interface->structValue->insert(BaseLib::StructElement("DEFAULT", BaseLib::PVariable(new BaseLib::Variable(j->second->isDefault()))));
			interface->structValue->insert(BaseLib::StructElement("LASTPACKETSENT", BaseLib::PVariable(new BaseLib::Variable((uint32_t)(j->second->lastPacketSent() / 1000)))));
			interface->structValue->insert(BaseLib::StructElement("LASTPACKETRECEIVED", BaseLib::PVariable(new BaseLib::Variable((uint32_t)(j->second->lastPacketReceived() / 1000)))));
			array->arrayValue->push_back(interface);
		}
		return array;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return BaseLib::Variable::createError(-32500, "Unknown application error.");
}

}
}
