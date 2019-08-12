/* Copyright 2013-2019 Homegear GmbH
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

#ifndef IDATABASECONTROLLER_H_
#define IDATABASECONTROLLER_H_

#include "DatabaseTypes.h"
#include "../Variable.h"
#include "../Sockets/RpcClientInfo.h"
#include "../Systems/Peer.h"
#include <set>

namespace BaseLib
{
namespace Database
{

struct SystemVariable
{
	std::string name;
	uint64_t room = 0;
	std::set<uint64_t> categories;
    std::set<uint64_t> roles;
	int32_t flags = 0;
	BaseLib::PVariable value;
};
typedef std::shared_ptr<SystemVariable> PSystemVariable;

/**
 * Base class for the database controller.
 */
class IDatabaseController
{
public:
	struct HomegearVariables
	{
		enum Enum { version = 0, upnpusn = 1 };
	};

	IDatabaseController() {}
	virtual ~IDatabaseController() {}
	virtual void dispose() = 0;
	virtual void init() = 0;

	//General
	virtual void open(std::string databasePath, std::string databaseFilename, bool databaseSynchronous, bool databaseMemoryJournal, bool databaseWALJournal, std::string backupPath, std::string backupFilename) = 0;
	virtual void hotBackup() = 0;
	virtual bool isOpen() = 0;
	virtual void initializeDatabase() = 0;
	virtual bool convertDatabase() = 0;
	virtual void createSavepointSynchronous(std::string& name) = 0;
	virtual void releaseSavepointSynchronous(std::string& name) = 0;
	virtual void createSavepointAsynchronous(std::string& name) = 0;
	virtual void releaseSavepointAsynchronous(std::string& name) = 0;
	//End general

	//Homegear variables
	virtual bool getHomegearVariableString(HomegearVariables::Enum id, std::string& value) = 0;
	virtual void setHomegearVariableString(HomegearVariables::Enum id, std::string& value) = 0;
	//End Homegear variables

	// {{{ Data
	virtual BaseLib::PVariable setData(std::string& component, std::string& key, BaseLib::PVariable& value) = 0;
	virtual BaseLib::PVariable getData(std::string& component, std::string& key) = 0;
	virtual BaseLib::PVariable deleteData(std::string& component, std::string& key) = 0;
	// }}}

	// {{{ UI
	virtual uint64_t addUiElement(std::string& elementId, BaseLib::PVariable data) = 0;
    virtual std::shared_ptr<DataTable> getUiElements() = 0;
	virtual void removeUiElement(uint64_t databaseId) = 0;
	// }}}

    // {{{ Stories
    virtual BaseLib::PVariable addRoomToStory(uint64_t storyId, uint64_t roomId) = 0;
    virtual BaseLib::PVariable createStory(BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
    virtual BaseLib::PVariable deleteStory(uint64_t storyId) = 0;
    virtual BaseLib::PVariable getRoomsInStory(PRpcClientInfo clientInfo, uint64_t storyId, bool checkAcls) = 0;
    virtual BaseLib::PVariable getStoryMetadata(uint64_t storyId) = 0;
    virtual BaseLib::PVariable getStories(std::string languageCode) = 0;
    virtual BaseLib::PVariable removeRoomFromStories(uint64_t roomId) = 0;
    virtual BaseLib::PVariable removeRoomFromStory(uint64_t storyId, uint64_t roomId) = 0;
    virtual bool storyExists(uint64_t storyId) = 0;
    virtual BaseLib::PVariable setStoryMetadata(uint64_t storyId, BaseLib::PVariable metadata) = 0;
    virtual BaseLib::PVariable updateStory(uint64_t storyId, BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
    // }}}

	// {{{ Rooms
	virtual BaseLib::PVariable createRoom(BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
	virtual BaseLib::PVariable deleteRoom(uint64_t roomId) = 0;
    virtual std::string getRoomName(PRpcClientInfo clientInfo, uint64_t roomId) = 0;
    virtual BaseLib::PVariable getRoomMetadata(uint64_t roomId) = 0;
	virtual BaseLib::PVariable getRooms(PRpcClientInfo clientInfo, std::string languageCode, bool checkAcls) = 0;
	virtual bool roomExists(uint64_t roomId) = 0;
    virtual BaseLib::PVariable setRoomMetadata(uint64_t roomId, BaseLib::PVariable metadata) = 0;
	virtual BaseLib::PVariable updateRoom(uint64_t roomId, BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
	// }}}

	// {{{ Categories
	virtual BaseLib::PVariable createCategory(BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
	virtual BaseLib::PVariable deleteCategory(uint64_t categoryId) = 0;
	virtual BaseLib::PVariable getCategories(PRpcClientInfo clientInfo, std::string languageCode, bool checkAcls) = 0;
    virtual BaseLib::PVariable getCategoryMetadata(uint64_t categoryId) = 0;
	virtual bool categoryExists(uint64_t categoryId) = 0;
    virtual BaseLib::PVariable setCategoryMetadata(uint64_t categoryId, BaseLib::PVariable metadata) = 0;
	virtual BaseLib::PVariable updateCategory(uint64_t categoryId, BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
	// }}}

	// {{{ Roles
	virtual BaseLib::PVariable createRole(BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
	virtual BaseLib::PVariable deleteRole(uint64_t roleId) = 0;
	virtual BaseLib::PVariable getRoles(PRpcClientInfo clientInfo, std::string languageCode, bool checkAcls) = 0;
	virtual BaseLib::PVariable getRoleMetadata(uint64_t roleId) = 0;
	virtual bool roleExists(uint64_t roleId) = 0;
	virtual BaseLib::PVariable setRoleMetadata(uint64_t roleId, BaseLib::PVariable metadata) = 0;
	virtual BaseLib::PVariable updateRole(uint64_t roleId, BaseLib::PVariable translations, BaseLib::PVariable metadata) = 0;
	// }}}

	// {{{ Node data
	virtual BaseLib::PVariable setNodeData(std::string& node, std::string& key, BaseLib::PVariable& value) = 0;
	virtual BaseLib::PVariable getNodeData(std::string& node, std::string& key, bool requestFromTrustedServer = false) = 0;
	virtual std::set<std::string> getAllNodeDataNodes() = 0;
	virtual BaseLib::PVariable deleteNodeData(std::string& node, std::string& key) = 0;
	// }}}

	//Metadata
	virtual BaseLib::PVariable setMetadata(PRpcClientInfo clientInfo, uint64_t peerId, std::string& serialNumber, std::string& dataId, BaseLib::PVariable& metadata) = 0;
	virtual BaseLib::PVariable getMetadata(uint64_t peerId, std::string& dataId) = 0;
	virtual BaseLib::PVariable getAllMetadata(PRpcClientInfo clientInfo, std::shared_ptr<Systems::Peer> peer, bool checkAcls) = 0;
	virtual BaseLib::PVariable deleteMetadata(uint64_t peerId, std::string& serialNumber, std::string& dataId) = 0;
	//End metadata

	//System variables
	virtual void deleteSystemVariable(std::string& variableId) = 0;
    virtual std::shared_ptr<BaseLib::Database::DataTable> getAllSystemVariables() = 0;
	virtual std::shared_ptr<BaseLib::Database::DataTable> getSystemVariable(const std::string& variableId) = 0;
    virtual std::shared_ptr<BaseLib::Database::DataTable> getSystemVariablesInRoom(uint64_t roomId) = 0;
    virtual void removeCategoryFromSystemVariables(uint64_t categoryId) = 0;
    virtual void removeRoleFromSystemVariables(uint64_t roleId) = 0;
    virtual void removeRoomFromSystemVariables(uint64_t roomId) = 0;
	virtual BaseLib::PVariable setSystemVariable(std::string& variableId, BaseLib::PVariable& value, uint64_t roomId, const std::string& categories, const std::string& roles, int32_t flags) = 0;
    virtual BaseLib::PVariable setSystemVariableCategories(std::string& variableId, const std::string& categories) = 0;
    virtual BaseLib::PVariable setSystemVariableRoles(std::string& variableId, const std::string& roles) = 0;
    virtual BaseLib::PVariable setSystemVariableRoom(std::string& variableId, uint64_t room) = 0;
	//End system variables

	//Users
	virtual bool createUser(const std::string& name, const std::vector<uint8_t>& passwordHash, const std::vector<uint8_t>& salt, const std::vector<uint64_t>& groups) = 0;
	virtual bool deleteUser(uint64_t userId) = 0;
    virtual std::shared_ptr<DataTable> getPassword(const std::string& name) = 0;
	virtual uint64_t getUserId(const std::string& name) = 0;
	virtual int64_t getUserKeyIndex1(uint64_t userId) = 0;
    virtual int64_t getUserKeyIndex2(uint64_t userId) = 0;
	virtual BaseLib::PVariable getUserMetadata(uint64_t userId) = 0;
	virtual std::shared_ptr<DataTable> getUsers() = 0;
    virtual std::vector<uint64_t> getUsersGroups(uint64_t userId) = 0;
	virtual bool updateUser(uint64_t userId, const std::vector<uint8_t>& passwordHash, const std::vector<uint8_t>& salt, const std::vector<uint64_t>& groups) = 0;
	virtual void setUserKeyIndex1(uint64_t userId, int64_t keyIndex) = 0;
    virtual void setUserKeyIndex2(uint64_t userId, int64_t keyIndex) = 0;
    virtual BaseLib::PVariable setUserMetadata(uint64_t userId, BaseLib::PVariable metadata) = 0;
    virtual bool userNameExists(const std::string& name) = 0;
	//End users

	//{{{ User data
    virtual BaseLib::PVariable setUserData(uint64_t userId, const std::string& component, const std::string& key, const BaseLib::PVariable& value) = 0;
    virtual BaseLib::PVariable getUserData(uint64_t userId, const std::string& component, const std::string& key) = 0;
    virtual BaseLib::PVariable deleteUserData(uint64_t userId, const std::string& component, const std::string& key) = 0;
	//}}}

    //Groups
    virtual BaseLib::PVariable createGroup(BaseLib::PVariable translations, BaseLib::PVariable acl) = 0;
    virtual BaseLib::PVariable deleteGroup(uint64_t groupId) = 0;
	virtual BaseLib::PVariable getAcl(uint64_t groupId) = 0;
	virtual BaseLib::PVariable getGroup(uint64_t groupId, std::string languageCode) = 0;
    virtual BaseLib::PVariable getGroups(std::string languageCode) = 0;
    virtual bool groupExists(uint64_t groupId) = 0;
    virtual BaseLib::PVariable updateGroup(uint64_t groupId, BaseLib::PVariable translations, BaseLib::PVariable acl) = 0;
    //End groups

	//Events
	virtual std::shared_ptr<DataTable> getEvents() = 0;
	virtual void saveEventAsynchronous(DataRow& event) = 0;
	virtual void deleteEvent(std::string& name) = 0;
	//End events

	//Family
	virtual void deleteFamily(int32_t familyId) = 0;
	virtual void saveFamilyVariableAsynchronous(int32_t familyId, BaseLib::Database::DataRow& data) = 0;
	virtual std::shared_ptr<BaseLib::Database::DataTable> getFamilyVariables(int32_t familyId) = 0;
	virtual void deleteFamilyVariable(BaseLib::Database::DataRow& data) = 0;
	//End family

	//Device
	virtual std::shared_ptr<DataTable> getDevices(uint32_t family) = 0;
	virtual void deleteDevice(uint64_t id) = 0;
	virtual uint64_t saveDevice(uint64_t id, int32_t address, std::string& serialNumber, uint32_t type, uint32_t family) = 0;
	virtual void saveDeviceVariableAsynchronous(DataRow& data) = 0;
	virtual void deletePeers(int32_t deviceID) = 0;
	virtual std::shared_ptr<DataTable> getPeers(uint64_t deviceID) = 0;
	virtual std::shared_ptr<DataTable> getDeviceVariables(uint64_t deviceID) = 0;
	//End device

	//Peer
	virtual void deletePeer(uint64_t id) = 0;
	virtual uint64_t savePeer(uint64_t id, uint32_t parentID, int32_t address, std::string& serialNumber, uint32_t type) = 0;
	virtual void savePeerParameterAsynchronous(DataRow& data) = 0;
    virtual void saveSpecialPeerParameterAsynchronous(DataRow& data) = 0;
	virtual void savePeerParameterRoomAsynchronous(BaseLib::Database::DataRow& data) = 0;
	virtual void savePeerParameterCategoriesAsynchronous(BaseLib::Database::DataRow& data) = 0;
	virtual void savePeerParameterRolesAsynchronous(BaseLib::Database::DataRow& data) = 0;
	virtual void savePeerVariableAsynchronous(DataRow& data) = 0;
	virtual std::shared_ptr<DataTable> getPeerParameters(uint64_t peerID) = 0;
	virtual std::shared_ptr<DataTable> getPeerVariables(uint64_t peerID) = 0;
	virtual void deletePeerParameter(uint64_t peerID, DataRow& data) = 0;

	virtual bool peerExists(uint64_t peerId) = 0;

	/**
	 * Changes the ID of a peer.
	 *
	 * @param oldPeerId The old ID of the peer.
	 * @param newPeerId The new ID of the peer.
	 * @return Returns "true" on success or "false" when the new ID is already in use.
	 */
	virtual bool setPeerID(uint64_t oldPeerId, uint64_t newPeerId) = 0;
	//End Peer

	//Service messages
	virtual std::shared_ptr<DataTable> getServiceMessages(uint64_t peerId) = 0;
	virtual void saveServiceMessageAsynchronous(uint64_t peerId, DataRow& data) = 0;
	virtual void saveGlobalServiceMessageAsynchronous(DataRow& data) = 0;
	virtual void deleteServiceMessage(uint64_t databaseID) = 0;
	virtual void deleteGlobalServiceMessage(int32_t familyId, int32_t messageId, std::string& messageSubId, std::string& message) = 0;
	//End service messages

	// {{{ License modules
	virtual std::shared_ptr<DataTable> getLicenseVariables(int32_t moduleId) = 0;
	virtual void saveLicenseVariable(int32_t moduleId, DataRow& data) = 0;
	virtual void deleteLicenseVariable(int32_t moduleId, uint64_t mapKey) = 0;
	// }}}
};

}
}
#endif
