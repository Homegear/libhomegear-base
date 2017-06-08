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

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "Encoding/RapidXml/rapidxml.hpp"
#include "DeviceDescription/Logical.h"
#include "DeviceDescription/Physical.h"

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <map>
#include <list>

using namespace rapidxml;

namespace BaseLib
{

enum class VariableType
{
	tVoid = 0x00,
	tInteger = 0x01,
	tBoolean = 0x02,
	tString = 0x03,
	tFloat = 0x04,
	tArray = 0x100,
	tStruct = 0x101,
	//rpcDate = 0x10,
	tBase64 = 0x11,
	tBinary = 0xD0,
	tInteger64 = 0xD1,
	tVariant = 0x1111,
};

class Variable;

typedef std::shared_ptr<Variable> PVariable;
typedef std::shared_ptr<PVariable> PPVariable;
typedef std::pair<std::string, PVariable> StructElement;
typedef std::map<std::string, PVariable> Struct;
typedef std::shared_ptr<std::map<std::string, PVariable>> PStruct;
typedef std::vector<PVariable> Array;
typedef std::shared_ptr<Array> PArray;
typedef std::list<PVariable> List;
typedef std::shared_ptr<List> PList;

class Variable
{
private:
	typedef void (Variable::*bool_type)() const;

	void this_type_does_not_support_comparisons() const {}
	std::string print(PVariable variable, std::string indent, bool oneLine);
	std::string printStruct(PStruct rpcStruct, std::string indent, bool oneLine);
	std::string printArray(PArray rpcArray, std::string indent, bool oneLine);

	/**
	 * Converts a XML node to a struct. Important: Multiple usage of the same name on the same level is not possible.
	 */
	void parseXmlNode(xml_node<>* node, PStruct& xmlStruct);
public:
	bool errorStruct = false;
	VariableType type;
	std::string stringValue;
	int32_t integerValue = 0;
	int64_t integerValue64 = 0;
	double floatValue = 0;
	bool booleanValue = false;
	PArray arrayValue;
	PStruct structValue;
	std::vector<uint8_t> binaryValue;

	Variable() { type = VariableType::tVoid; arrayValue = PArray(new Array()); structValue = PStruct(new Struct()); }
	Variable(Variable const& rhs);
	Variable(VariableType variableType) : Variable() { type = variableType; if(type == VariableType::tVariant) type = VariableType::tVoid; }
	Variable(DeviceDescription::ILogical::Type::Enum variableType);
	Variable(uint8_t integer) : Variable() { type = VariableType::tInteger; integerValue = (int32_t)integer; }
	Variable(int32_t integer) : Variable() { type = VariableType::tInteger; integerValue = integer; integerValue64 = integer; }
	Variable(uint32_t integer) : Variable() { type = VariableType::tInteger; integerValue = (int32_t)integer; integerValue64 = integer; }
	Variable(int64_t integer) : Variable() { type = VariableType::tInteger64; integerValue = (int32_t)integer; integerValue64 = integer; }
	Variable(uint64_t integer) : Variable() { type = VariableType::tInteger64; integerValue = (int32_t)integer; integerValue64 = (int64_t)integer; }
	Variable(std::string string) : Variable() { type = VariableType::tString; stringValue = string; }
	Variable(const char* string) : Variable() { type = VariableType::tString; stringValue = std::string(string); }
	Variable(bool boolean) : Variable() { type = VariableType::tBoolean; booleanValue = boolean; }
	Variable(double floatVal) : Variable() { type = VariableType::tFloat; floatValue = floatVal; }
	Variable(PArray arrayVal) : Variable() { type = VariableType::tArray; arrayValue = arrayVal; }
	Variable(std::vector<std::string>& arrayVal) : Variable() { type = VariableType::tArray; arrayValue->reserve(arrayVal.size()); for(std::vector<std::string>::iterator i = arrayVal.begin(); i != arrayVal.end(); ++i) arrayValue->push_back(PVariable(new Variable(*i))); }
	Variable(PStruct structVal) : Variable() { type = VariableType::tStruct; structValue = structVal; }
	Variable(std::vector<uint8_t>& binaryVal) : Variable() { type = VariableType::tBinary; binaryValue = binaryVal; }
	Variable(std::vector<char>& binaryVal) : Variable() { type = VariableType::tBinary; binaryValue.clear(); binaryValue.insert(binaryValue.end(), binaryVal.begin(), binaryVal.end()); }
	Variable(xml_node<>* node);
	virtual ~Variable();
	static PVariable createError(int32_t faultCode, std::string faultString);
	std::string print(bool stdout = false, bool stderr = false, bool oneLine = false);
	static std::string getTypeString(VariableType type);
	void setType(VariableType value) { type = value; };
	void setType(DeviceDescription::ILogical::Type::Enum value);
	static PVariable fromString(std::string& value, DeviceDescription::ILogical::Type::Enum type);
	static PVariable fromString(std::string& value, DeviceDescription::IPhysical::Type::Enum type);
	static PVariable fromString(std::string& value, VariableType type);
	std::string toString();
	Variable& operator=(const Variable& rhs);
	bool operator==(const Variable& rhs);
	bool operator<(const Variable& rhs);
	bool operator<=(const Variable& rhs);
	bool operator>(const Variable& rhs);
	bool operator>=(const Variable& rhs);
	bool operator!=(const Variable& rhs);
	operator bool_type() const;
};

}

#endif
