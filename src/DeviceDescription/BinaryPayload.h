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

#ifndef BINARYPAYLOAD_H_
#define BINARYPAYLOAD_H_

#include <string>
#include <vector>
#include <memory>

namespace rapidxml
{
class xml_node;
}

using namespace rapidxml;

namespace BaseLib
{

class SharedObjects;

namespace DeviceDescription
{

class BinaryPayload;

/**
 * Helper type for BinaryPayload pointers.
 */
typedef std::shared_ptr<BinaryPayload> PBinaryPayload;

/**
 * Helper type for arrays of BinaryPayload pointers.
 */
typedef std::vector<PBinaryPayload> BinaryPayloads;

/**
 * Class describing binary payloads.
 */
class BinaryPayload
{
public:
	BinaryPayload(BaseLib::SharedObjects* baseLib);
	BinaryPayload(BaseLib::SharedObjects* baseLib, xml_node* node);
	virtual ~BinaryPayload() {}

	double index = 0;
	double size = 1.0;
	double index2 = 0;
	double size2 = 0;
	uint32_t bitIndex = 0;
	uint32_t bitSize = 0;
	int32_t index2Offset = -1;
	int32_t constValueInteger = -1;
	double constValueDecimal = -1;
	std::string constValueString;
	bool isSigned = false;
	bool omitIfSet = false;
	int32_t omitIf = 0;
	std::string parameterId;
	int32_t parameterChannel = -1;
	int32_t metaInteger1 = -1;
	int32_t metaInteger2 = -1;
	int32_t metaInteger3 = -1;
	int32_t metaInteger4 = -1;
protected:
	BaseLib::SharedObjects* _bl = nullptr;
};
}
}

#endif
