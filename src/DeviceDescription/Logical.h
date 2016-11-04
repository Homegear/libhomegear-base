/* Copyright 2013-2016 Sathya Laufer
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

#ifndef DEVICEPARAMETERLOGICAL_H_
#define DEVICEPARAMETERLOGICAL_H_

#include "../Encoding/RapidXml/rapidxml.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

using namespace rapidxml;

namespace BaseLib
{

class SharedObjects;
class Variable;

namespace DeviceDescription
{

class LogicalAction;
class LogicalArray;
class LogicalBoolean;
class LogicalDecimal;
class LogicalEnumeration;
class LogicalInteger;
class LogicalInteger64;
class LogicalString;
class LogicalStruct;

typedef std::shared_ptr<LogicalAction> PLogicalAction;
typedef std::shared_ptr<LogicalArray> PLogicalArray;
typedef std::shared_ptr<LogicalBoolean> PLogicalBoolean;
typedef std::shared_ptr<LogicalDecimal> PLogicalDecimal;
typedef std::shared_ptr<LogicalEnumeration> PLogicalEnumeration;
typedef std::shared_ptr<LogicalInteger> PLogicalInteger;
typedef std::shared_ptr<LogicalInteger64> PLogicalInteger64;
typedef std::shared_ptr<LogicalString> PLogicalString;
typedef std::shared_ptr<LogicalStruct> PLogicalStruct;


class EnumerationValue
{
public:
	std::string id;
	bool indexDefined = false;
	int32_t index = -1;

	EnumerationValue() {}
	EnumerationValue(std::string id, int32_t index) { this->id = id; this->index = index; indexDefined = true; }
	EnumerationValue(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~EnumerationValue() {}
};

class ILogical
{
public:
	struct Type
	{
		enum Enum { none = 0x00, tInteger = 0x01, tBoolean = 0x02, tString = 0x03, tFloat = 0x04, tEnum = 0x20, tAction = 0x30, tInteger64 = 0xD1, tArray = 0x100, tStruct = 0x101 };
	};

	ILogical(BaseLib::SharedObjects* baseLib);
	ILogical(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~ILogical() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing() = 0;
	virtual std::shared_ptr<Variable> getDefaultValue() = 0;

	Type::Enum type = Type::none;
	bool defaultValueExists = false;
	bool setToValueOnPairingExists = false;
protected:
	BaseLib::SharedObjects* _bl = nullptr;
};

class LogicalInteger : public ILogical
{
public:
	int32_t minimumValue = -2147483648;
	int32_t maximumValue = 2147483647;
	int32_t defaultValue = 0;
	int32_t setToValueOnPairing = 0;
	std::unordered_map<std::string, int32_t> specialValuesStringMap;
	std::unordered_map<int32_t, std::string> specialValuesIntegerMap;

	LogicalInteger(BaseLib::SharedObjects* baseLib);
	LogicalInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalInteger() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalInteger64 : public ILogical
{
public:
	int64_t minimumValue = -9223372036854775807ll - 1;
	int64_t maximumValue = 9223372036854775807ll;
	int64_t defaultValue = 0;
	int64_t setToValueOnPairing = 0;
	std::unordered_map<std::string, int64_t> specialValuesStringMap;
	std::unordered_map<int64_t, std::string> specialValuesIntegerMap;

	LogicalInteger64(BaseLib::SharedObjects* baseLib);
	LogicalInteger64(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalInteger64() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalDecimal : public ILogical
{
public:
	double minimumValue = -3.40282347e+38f;
	double maximumValue = 3.40282347e+38f;
	double defaultValue = 0;
	double setToValueOnPairing = 0;
	std::unordered_map<std::string, double> specialValuesStringMap;
	std::unordered_map<double, std::string> specialValuesFloatMap;

	LogicalDecimal(BaseLib::SharedObjects* baseLib);
	LogicalDecimal(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalDecimal() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalEnumeration : public ILogical
{
public:
	int32_t minimumValue = 0;
	int32_t maximumValue = 0;
	int32_t defaultValue = 0;
	int32_t setToValueOnPairing = 0;

	LogicalEnumeration(BaseLib::SharedObjects* baseLib);
	LogicalEnumeration(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalEnumeration() {}
	std::vector<EnumerationValue> values;
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalBoolean : public ILogical
{
public:
	bool defaultValue = false;
	bool setToValueOnPairing = false;

	LogicalBoolean(BaseLib::SharedObjects* baseLib);
	LogicalBoolean(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalBoolean() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalString : public ILogical
{
public:
	std::string defaultValue;
	std::string setToValueOnPairing;

	LogicalString(BaseLib::SharedObjects* baseLib);
	LogicalString(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalString() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalAction : public ILogical
{
public:
	bool defaultValue = false;
	bool setToValueOnPairing = false;

	LogicalAction(BaseLib::SharedObjects* baseLib);
	LogicalAction(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalAction() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalArray : public ILogical
{
public:
	LogicalArray(BaseLib::SharedObjects* baseLib);
	LogicalArray(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalArray() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

class LogicalStruct : public ILogical
{
public:
	LogicalStruct(BaseLib::SharedObjects* baseLib);
	LogicalStruct(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	virtual ~LogicalStruct() {}
	virtual std::shared_ptr<Variable> getSetToValueOnPairing();
	virtual std::shared_ptr<Variable> getDefaultValue();
};

}
}

#endif
