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

#ifndef INODE_H_
#define INODE_H_

#include "../Variable.h"

#include <atomic>
#include <string>
#include <memory>

namespace BaseLib
{

class SharedObjects;

namespace Flows
{

class INode
{
public:
	INode(std::string path, std::string name);
	virtual ~INode();

	std::string getName() { return _name; }
	std::string getPath() { return _path; }
	std::string getId() { return _id; }
	void setId(std::string value) { _id = value; }

	virtual bool start() { return true; }
	virtual void stop() {}

	virtual void variableEvent(uint64_t peerId, int32_t channel, std::string variable, BaseLib::PVariable value) {}

	void setSubscribePeer(std::function<void(std::string, uint64_t, int32_t, std::string)> value) { _subscribePeer.swap(value); }
	void setUnsubscribePeer(std::function<void(std::string, uint64_t, int32_t, std::string)> value) { _unsubscribePeer.swap(value); }
	void setOutput(std::function<void(std::string, uint32_t, BaseLib::PVariable)> value) { _output.swap(value); }

	virtual PVariable input(PVariable message) { return PVariable(); }
protected:
	void suscribePeer(uint64_t peerId, int32_t channel = -1, std::string variable = "");
	void unsuscribePeer(uint64_t peerId, int32_t channel = -1, std::string variable = "");
	void output(uint32_t index, BaseLib::PVariable message);
private:
	std::atomic_bool _locked;
	std::atomic_int _referenceCounter;
	std::string _path;
	std::string _name;
	std::string _id;
	std::function<void(std::string, uint64_t, int32_t, std::string)> _subscribePeer;
	std::function<void(std::string, uint64_t, int32_t, std::string)> _unsubscribePeer;
	std::function<void(std::string, uint32_t, BaseLib::PVariable)> _output;

	INode(const INode&) = delete;
	INode& operator=(const INode&) = delete;
};

typedef std::shared_ptr<INode> PINode;

}
}
#endif
