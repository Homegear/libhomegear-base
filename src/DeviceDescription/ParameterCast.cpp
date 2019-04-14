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

#include "ParameterCast.h"
#include "../BaseLib.h"

#include <iomanip>

namespace BaseLib
{
namespace DeviceDescription
{
namespace ParameterCast
{

ICast::ICast(BaseLib::SharedObjects* baseLib)
{
	_bl = baseLib;
}

ICast::ICast(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib)
{
	_parameter = parameter;
}

void ICast::fromPacket(PVariable value)
{
}

void ICast::toPacket(PVariable value)
{
}

DecimalIntegerScale::DecimalIntegerScale(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

DecimalIntegerScale::DecimalIntegerScale(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"decimalIntegerScale\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "factor")
		{
			factor = Math::getDouble(value);
			if(factor == 0) factor = 1;
		}
		else if(name == "offset") offset = Math::getDouble(value);
		else _bl->out.printWarning("Warning: Unknown node in \"decimalIntegerScale\": " + name);
	}
}

void DecimalIntegerScale::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tFloat;
	value->floatValue = ((double)value->integerValue / factor) - offset;
	value->integerValue = 0;
}

void DecimalIntegerScale::toPacket(PVariable value)
{
	if(!value) return;
	value->integerValue = std::lround((value->floatValue + offset) * factor);
	value->type = VariableType::tInteger;
	value->floatValue = 0;
}

IntegerIntegerScale::IntegerIntegerScale(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

IntegerIntegerScale::IntegerIntegerScale(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"integerIntegerScale\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "factor")
		{
			factor = Math::getDouble(value);
			if(factor == 0) factor = 1;
		}
		else if(name == "operation")
		{
			if(value == "division") operation = Operation::Enum::division;
			else if(value == "multiplication") operation = Operation::Enum::multiplication;
			else _bl->out.printWarning("Warning: Unknown value for \"integerIntegerScale\\operation\": " + value);
		}
		else if(name == "offset") offset = Math::getNumber(value);
		else _bl->out.printWarning("Warning: Unknown node in \"integerIntegerScale\": " + name);
	}
}

void IntegerIntegerScale::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(operation == Operation::Enum::division) value->integerValue = std::lround((double)value->integerValue * factor) - offset;
	else if(operation == Operation::Enum::multiplication) value->integerValue = std::lround((double)value->integerValue / factor) - offset;
	else _bl->out.printWarning("Warning: Operation is not set for parameter conversion integerIntegerScale.");
}

void IntegerIntegerScale::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(operation == Operation::Enum::multiplication) value->integerValue = std::lround((double)(value->integerValue + offset) * factor);
	else if(operation == Operation::Enum::division) value->integerValue = std::lround((double)(value->integerValue + offset) / factor);
	else _bl->out.printWarning("Warning: Operation is not set for parameter conversion integerIntegerScale.");
}

IntegerOffset::IntegerOffset(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

IntegerOffset::IntegerOffset(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"integerOffset\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "addOffset")
		{
			offset = Math::getNumber(value);
			addOffset = true;
		}
		else if(name == "subtractFromOffset") offset = Math::getNumber(value);
		else if(name == "direction") directionToPacket = (value != "fromPacket");
		else _bl->out.printWarning("Warning: Unknown node in \"integerOffset\": " + name);
	}
}

void IntegerOffset::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(directionToPacket) value->integerValue = addOffset ? value->integerValue - offset : offset - value->integerValue;
	else value->integerValue = addOffset ? value->integerValue + offset : offset - value->integerValue;
}

void IntegerOffset::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(directionToPacket) value->integerValue = addOffset ? value->integerValue + offset : offset - value->integerValue;
	else value->integerValue = addOffset ? value->integerValue - offset : offset - value->integerValue;
}

DecimalOffset::DecimalOffset(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

DecimalOffset::DecimalOffset(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"decimalOffset\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "addOffset")
		{
			offset = Math::getDouble(value);
			addOffset = true;
		}
		else if(name == "subtractFromOffset") offset = Math::getDouble(value);
		else if(name == "direction") directionToPacket = (value != "fromPacket");
		else _bl->out.printWarning("Warning: Unknown node in \"decimalOffset\": " + name);
	}
}

void DecimalOffset::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tFloat;
	if(directionToPacket) value->floatValue = addOffset ? value->floatValue - offset : offset - value->floatValue;
	else value->floatValue = addOffset ? value->floatValue + offset : offset - value->floatValue;
}

void DecimalOffset::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tFloat;
	if(directionToPacket) value->floatValue = addOffset ? value->floatValue + offset : offset - value->floatValue;
	else value->floatValue = addOffset ? value->floatValue - offset : offset - value->floatValue;
}

IntegerIntegerMap::IntegerIntegerMap(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

IntegerIntegerMap::IntegerIntegerMap(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"integerIntegerMap\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "value")
		{
			for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
			{
				_bl->out.printWarning("Warning: Unknown attribute for \"integerIntegerMap\\value\": " + std::string(attr->name()));
			}
			int32_t physicalValue = 0;
			int32_t logicalValue = 0;
			for(xml_node<>* valueNode = subNode->first_node(); valueNode; valueNode = valueNode->next_sibling())
			{
				std::string valueName(valueNode->name());
				std::string valueValue(valueNode->value());
				if(valueName == "physical") physicalValue = Math::getNumber(valueValue);
				else if(valueName == "logical") logicalValue = Math::getNumber(valueValue);
				else _bl->out.printWarning("Warning: Unknown element in \"integerIntegerMap\\value\": " + valueName);
			}
			integerValueMapFromDevice[physicalValue] = logicalValue;
			integerValueMapToDevice[logicalValue] = physicalValue;
		}
		else if(name == "direction")
		{
			if(value == "fromDevice") direction = Direction::Enum::fromDevice;
			else if(value == "toDevice") direction = Direction::Enum::toDevice;
			else if(value == "both") direction = Direction::Enum::both;
			else _bl->out.printWarning("Warning: Unknown value for \"integerIntegerMap\\direction\": " + value);
		}
		else _bl->out.printWarning("Warning: Unknown node in \"integerIntegerMap\": " + name);
	}
}

void IntegerIntegerMap::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(direction == Direction::Enum::fromDevice || direction == Direction::Enum::both)
	{
		std::map<int32_t, int32_t>::const_iterator element = integerValueMapFromDevice.find(value->integerValue);
		if(element != integerValueMapFromDevice.end()) value->integerValue = element->second;
	}
}

void IntegerIntegerMap::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(direction == Direction::Enum::toDevice || direction == Direction::Enum::both)
	{
		std::map<int32_t, int32_t>::const_iterator element = integerValueMapToDevice.find(value->integerValue);
		if(element != integerValueMapToDevice.end()) value->integerValue = element->second;
	}
}

BooleanInteger::BooleanInteger(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

BooleanInteger::BooleanInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"booleanInteger\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "trueValue") trueValue = Math::getNumber(value);
		else if(name == "falseValue") falseValue = Math::getNumber(value);
		else if(name == "invert") { if(value == "true") invert = true; }
		else if(name == "threshold") threshold = Math::getNumber(value);
		else _bl->out.printWarning("Warning: Unknown node in \"booleanInteger\": " + name);
	}
}

void BooleanInteger::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tBoolean;
	if(trueValue == 0 && falseValue == 0)
	{
		if(value->integerValue >= threshold) value->booleanValue = true;
		else value->booleanValue = false;
	}
	else
	{
		if(value->integerValue == falseValue) value->booleanValue = false;
		if(value->integerValue == trueValue || value->integerValue >= threshold) value->booleanValue = true;
	}
	if(invert) value->booleanValue = !value->booleanValue;
	value->integerValue = 0;
}

void BooleanInteger::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(invert) value->booleanValue = !value->booleanValue;
	if(trueValue == 0 && falseValue == 0) value->integerValue = (int32_t)value->booleanValue;
	else if(value->booleanValue) value->integerValue = trueValue;
	else value->integerValue = falseValue;
	value->booleanValue = false;
}

BooleanString::BooleanString(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

BooleanString::BooleanString(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"booleanString\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "trueValue") trueValue = value;
		else if(name == "falseValue") falseValue = value;
		else if(name == "invert") { if(value == "true") invert = true; }
		else _bl->out.printWarning("Warning: Unknown node in \"booleanString\": " + name);
	}
}

void BooleanString::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tBoolean;
	value->booleanValue = (value->stringValue == trueValue);
	if(invert) value->booleanValue = !value->booleanValue;
	value->stringValue.clear();
}

void BooleanString::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tString;
	if(invert) value->booleanValue = !value->booleanValue;
	if(value->booleanValue) value->stringValue = trueValue;
	else value->stringValue = falseValue;
	value->booleanValue = false;
}

DecimalConfigTime::DecimalConfigTime(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

DecimalConfigTime::DecimalConfigTime(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"decimalConfigTime\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "factors")
		{
			for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
			{
				_bl->out.printWarning("Warning: Unknown attribute for \"decimalConfigTime\\factors\": " + std::string(attr->name()));
			}
			for(xml_node<>* factorNode = subNode->first_node(); factorNode; factorNode = factorNode->next_sibling())
			{
				std::string factorName(factorNode->name());
				std::string factorValue(factorNode->value());
				if(factorName == "factor")
				{
					factors.push_back(Math::getDouble(factorValue));
					if(factors.back() == 0) factors.back() = 1;
				}
				else _bl->out.printWarning("Warning: Unknown element in \"decimalConfigTime\\factors\": " + factorName);
			}
		}
		else if(name == "valueSize") valueSize = Math::getDouble(value);
		else _bl->out.printWarning("Warning: Unknown node in \"decimalConfigTime\": " + name);
	}
}

void DecimalConfigTime::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tFloat;
	if(value < 0)
	{
		value->floatValue = 0;
		return;
	}
	if(valueSize > 0 && factors.size() > 0)
	{
		uint32_t bits = std::lround(std::floor(valueSize)) * 8;
		bits += std::lround(valueSize * 10) % 10;
		double factor = factors.at(value->integerValue >> bits);
		value->floatValue = (value->integerValue & (0xFFFFFFFF >> (32 - bits))) * factor;
	}
	else
	{
		int32_t factorIndex = (value->integerValue & 0xFF) >> 5;
		double factor = 0;
		switch(factorIndex)
		{
		case 0:
			factor = 0.1;
			break;
		case 1:
			factor = 1;
			break;
		case 2:
			factor = 5;
			break;
		case 3:
			factor = 10;
			break;
		case 4:
			factor = 60;
			break;
		case 5:
			factor = 300;
			break;
		case 6:
			factor = 600;
			break;
		case 7:
			factor = 3600;
			break;
		}
		value->floatValue = (value->integerValue & 0x1F) * factor;
	}
	value->integerValue = 0;
}

void DecimalConfigTime::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	if(valueSize > 0 && factors.size() > 0)
	{
		uint32_t bits = std::lround(std::floor(valueSize)) * 8;
		bits += std::lround(valueSize * 10) % 10;
		if(value->floatValue < 0) value->floatValue = 0;
		int32_t maxNumber = (1 << bits) - 1;
		int32_t factorIndex = 0;
		while(factorIndex < (signed)factors.size() && (value->floatValue / factors.at(factorIndex)) > maxNumber) factorIndex++;

		value->integerValue = (factorIndex << bits) | std::lround(value->floatValue / factors.at(factorIndex));
	}
	else
	{
		int32_t factorIndex = 0;
		double factor = 0.1;
		if(value->floatValue < 0) value->floatValue = 0;
		if(value->floatValue <= 3.1) { factorIndex = 0; factor = 0.1; }
		else if(value->floatValue <= 31) { factorIndex = 1; factor = 1; }
		else if(value->floatValue <= 155) { factorIndex = 2; factor = 5; }
		else if(value->floatValue <= 310) { factorIndex = 3; factor = 10; }
		else if(value->floatValue <= 1860) { factorIndex = 4; factor = 60; }
		else if(value->floatValue <= 9300) { factorIndex = 5; factor = 300; }
		else if(value->floatValue <= 18600) { factorIndex = 6; factor = 600; }
		else { factorIndex = 7; factor = 3600; }

		value->integerValue = ((factorIndex << 5) | std::lround(value->floatValue / factor)) & 0xFF;
	}
	value->floatValue = 0;
}

IntegerTinyFloat::IntegerTinyFloat(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

IntegerTinyFloat::IntegerTinyFloat(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"integerTinyFloat\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "mantissaStart") mantissaStart = Math::getNumber(value);
		else if(name == "mantissaSize") mantissaSize = Math::getNumber(value);
		else if(name == "exponentStart") exponentStart = Math::getNumber(value);
		else if(name == "exponentSize") exponentSize = Math::getNumber(value);
		else _bl->out.printWarning("Warning: Unknown node in \"integerTinyFloat\": " + name);
	}
}

void IntegerTinyFloat::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	int32_t mantissa = (mantissaSize == 0) ? 1 : ((value->integerValue >> mantissaStart) & ((1 << mantissaSize) - 1));
	int32_t exponent = (value->integerValue >> exponentStart) & ((1 << exponentSize) - 1);
	value->integerValue = mantissa * (1 << exponent);
}

void IntegerTinyFloat::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	int64_t maxMantissa = (((int64_t)1 << mantissaSize) - 1);
	int64_t maxExponent = (((int64_t)1 << exponentSize) - 1);
	int64_t mantissa = value->integerValue;
	int64_t exponent = 0;
	if(maxMantissa > 0)
	{
		while(mantissa >= maxMantissa)
		{
			mantissa = mantissa >> 1;
			exponent++;
		}
	}
	if(mantissa > maxMantissa) mantissa = maxMantissa;
	if(exponent > maxExponent) exponent = maxExponent;
	exponent = exponent << exponentStart;
	value->integerValue = (mantissa << mantissaStart) | exponent;
}

StringUnsignedInteger::StringUnsignedInteger(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

StringUnsignedInteger::StringUnsignedInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"stringUnsignedInteger\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"stringUnsignedInteger\": " + std::string(subNode->name()));
	}
}

void StringUnsignedInteger::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tString;
	value->stringValue = std::to_string((uint32_t)value->integerValue);
	value->integerValue = 0;
}

void StringUnsignedInteger::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	value->integerValue = Math::getUnsignedNumber(value->stringValue);
	value->stringValue.clear();
}

BlindTest::BlindTest(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

BlindTest::BlindTest(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"blindTest\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "value") value = Math::getNumber(value);
		else _bl->out.printWarning("Warning: Unknown node in \"blindTest\": " + name);
	}
}

void BlindTest::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	value->integerValue = this->value;
}

void BlindTest::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	value->integerValue = this->value;
}

OptionString::OptionString(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

OptionString::OptionString(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"optionString\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"optionString\": " + std::string(subNode->name()));
	}
}

void OptionString::fromPacket(PVariable value)
{
	if(!value || !_parameter) return;
	value->type = VariableType::tInteger;
	LogicalEnumeration* logicalEnum = (LogicalEnumeration*)_parameter->logical.get();
	value->integerValue = -1;
	for(std::vector<EnumerationValue>::iterator i = logicalEnum->values.begin(); i != logicalEnum->values.end(); ++i)
	{
		if(i->id == value->stringValue)
		{
			value->integerValue = i->index;
			break;
		}
	}
	if(value->integerValue < 0)
	{
		_bl->out.printWarning("Warning: Cannot convert JSON string to enum, because no matching element could be found for \"" + value->stringValue + "\".");
		value->integerValue = 0;
	}
	value->stringValue = "";
}

void OptionString::toPacket(PVariable value)
{
	if(!value || !_parameter) return;
	if(_parameter->logical->type == ILogical::Type::Enum::tEnum)
	{
		value->type = VariableType::tString;
		LogicalEnumeration* logicalEnum = (LogicalEnumeration*)_parameter->logical.get();
		if(value->integerValue >= 0 && value->integerValue < (signed)logicalEnum->values.size())
		{
			value->stringValue = logicalEnum->values.at(value->integerValue).id;
		}
		else _bl->out.printWarning("Warning: Cannot convert variable, because enum index is not valid.");
		value->integerValue = 0;
	}
}

OptionInteger::OptionInteger(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

OptionInteger::OptionInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"optionInteger\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "value")
		{
			for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
			{
				_bl->out.printWarning("Warning: Unknown attribute for \"optionInteger\\value\": " + std::string(attr->name()));
			}
			int32_t physicalValue = 0;
			int32_t logicalValue = 0;
			for(xml_node<>* valueNode = subNode->first_node(); valueNode; valueNode = valueNode->next_sibling())
			{
				std::string valueName(valueNode->name());
				std::string valueValue(valueNode->value());
				if(valueName == "physical") physicalValue = Math::getNumber(valueValue);
				else if(valueName == "logical") logicalValue = Math::getNumber(valueValue);
				else _bl->out.printWarning("Warning: Unknown element in \"optionInteger\\value\": " + valueName);
			}
			valueMapFromDevice[physicalValue] = logicalValue;
			valueMapToDevice[logicalValue] = physicalValue;
		}
		else _bl->out.printWarning("Warning: Unknown node in \"optionInteger\": " + name);
	}
}

void OptionInteger::fromPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	std::map<int32_t, int32_t>::const_iterator element = valueMapFromDevice.find(value->integerValue);
	if(element != valueMapFromDevice.end()) value->integerValue = element->second;
}

void OptionInteger::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tInteger;
	std::map<int32_t, int32_t>::const_iterator element = valueMapToDevice.find(value->integerValue);
	if(element != valueMapToDevice.end()) value->integerValue = element->second;
}

StringJsonArrayDecimal::StringJsonArrayDecimal(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

StringJsonArrayDecimal::StringJsonArrayDecimal(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"stringJsonArrayDecimal\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"stringJsonArrayDecimal\": " + std::string(subNode->name()));
	}
}

void StringJsonArrayDecimal::fromPacket(PVariable value)
{
	if(!value || !_parameter) return;
	if(_parameter->logical->type == ILogical::Type::Enum::tString)
	{
		value->type = VariableType::tString;
		if(value->arrayValue->size() > 0) value->stringValue = std::to_string(value->arrayValue->at(0)->floatValue);
		if(value->arrayValue->size() > 1)
		{
			for(std::vector<std::shared_ptr<Variable>>::iterator i = value->arrayValue->begin() + 1; i != value->arrayValue->end(); ++i)
			{
				value->stringValue += ';' + std::to_string((*i)->floatValue);
			}
		}
		value->arrayValue->clear();
	}
	else _bl->out.printWarning("Warning: Only strings can be created from Json arrays.");
}

void StringJsonArrayDecimal::toPacket(PVariable value)
{
	if(!value || !_parameter) return;
	if(_parameter->logical->type == ILogical::Type::Enum::tString)
	{
		std::vector<std::string> arrayElements = HelperFunctions::splitAll(value->stringValue, ';');
		for(std::vector<std::string>::iterator i = arrayElements.begin(); i != arrayElements.end(); ++i)
		{
			value->arrayValue->push_back(std::shared_ptr<Variable>(new Variable(Math::getDouble(*i))));
		}
		value->type = VariableType::tArray;
		value->stringValue = "";
	}
	else _bl->out.printWarning("Warning: Only strings can be converted to Json arrays.");
}

RpcBinary::RpcBinary(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
	_binaryEncoder = std::shared_ptr<BaseLib::Rpc::RpcEncoder>(new BaseLib::Rpc::RpcEncoder(_bl));
	_binaryDecoder = std::shared_ptr<BaseLib::Rpc::RpcDecoder>(new BaseLib::Rpc::RpcDecoder(_bl));
}

RpcBinary::RpcBinary(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	_binaryEncoder = std::shared_ptr<BaseLib::Rpc::RpcEncoder>(new BaseLib::Rpc::RpcEncoder(_bl));
	_binaryDecoder = std::shared_ptr<BaseLib::Rpc::RpcDecoder>(new BaseLib::Rpc::RpcDecoder(_bl));

	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"rpcBinary\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"rpcBinary\": " + std::string(subNode->name()));
	}
}

void RpcBinary::fromPacket(PVariable value)
{
	if(!value) return;
	_binaryDecoder->decodeResponse(value);
}

void RpcBinary::toPacket(PVariable value)
{
	if(!value) return;
	_binaryEncoder->encodeResponse(value, value->binaryValue);
	value->type = VariableType::tBinary;
}

Toggle::Toggle(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

Toggle::Toggle(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"toggle\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{

		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "parameter") this->parameter = value;
		else if(name == "on") on = Math::getNumber(value);
		else if(name == "off") off = Math::getNumber(value);
		else _bl->out.printWarning("Warning: Unknown node in \"toggle\": " + name);
	}
}

void Toggle::fromPacket(PVariable value)
{
}

void Toggle::toPacket(PVariable value)
{
}

CcrtdnParty::CcrtdnParty(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

CcrtdnParty::CcrtdnParty(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"ccrtdnParty\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"ccrtdnParty\": " + std::string(subNode->name()));
	}
}

void CcrtdnParty::fromPacket(PVariable value)
{
	value->stringValue = "";
	value->type = VariableType::tString;
}

void CcrtdnParty::toPacket(PVariable value)
{
	if(!value) return;
	//Cannot currently easily be handled by ParameterConversion::toPacket
	value->binaryValue.resize(8, 0);
	if(value->stringValue.empty()) return;
	std::istringstream stringStream(value->stringValue);
	std::string element;

	//"TEMP,START_TIME,DAY,MONTH,YEAR,END_TIME,DAY,MONTH,YEAR"
	for(uint32_t i = 0; std::getline(stringStream, element, ',') && i < 9; i++)
	{
		//Temperature
		if(i == 0) value->binaryValue.at(0) = std::lround(2 * Math::getDouble(element));
		//Start time
		else if(i == 1) value->binaryValue.at(1) = Math::getNumber(element) / 30;
		//Start day
		else if(i == 2) value->binaryValue.at(2) = Math::getNumber(element);
		//Start month
		else if(i == 3) value->binaryValue.at(7) = Math::getNumber(element) << 4;
		//Start year
		else if(i == 4) value->binaryValue.at(3) = Math::getNumber(element);
		//End time
		else if(i == 5) value->binaryValue.at(4) = Math::getNumber(element) / 30;
		//End day
		else if(i == 6) value->binaryValue.at(5) = Math::getNumber(element);
		//End month
		else if(i == 7) value->binaryValue.at(7) |= Math::getNumber(element);
		//End year
		else if(i == 8) value->binaryValue.at(6) = Math::getNumber(element);
	}
	value->type = VariableType::tBinary;
}

Cfm::Cfm(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

Cfm::Cfm(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"cfm\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"cfm\": " + std::string(subNode->name()));
	}
}

void Cfm::fromPacket(PVariable value)
{
	value->stringValue = "";
	value->type = VariableType::tString;
}

void Cfm::toPacket(PVariable value)
{
	if(!value) return;
	value->binaryValue.resize(14, 0);
	if(value->stringValue.empty() || value->stringValue == "0") return;
	std::istringstream stringStream(value->stringValue);
	std::string element;

	for(uint32_t i = 0; std::getline(stringStream, element, ',') && i < 13; i++)
	{
		if(i == 0)
		{
			value->binaryValue.at(0) = std::lround(200 * Math::getDouble(element));
		}
		else if(i == 1)
		{
			value->binaryValue.at(1) = Math::getNumber(element);
		}
		else if(i == 2)
		{
			value->integerValue = std::lround(Math::getDouble(element) * 10);
			IntegerTinyFloat cast(_bl);
			cast.toPacket(value);
			std::vector<uint8_t> time;
			_bl->hf.memcpyBigEndian(time, value->integerValue);
			if(time.size() == 1) value->binaryValue.at(13) = time.at(0);
			else
			{
				value->binaryValue.at(12) = time.at(0);
				value->binaryValue.at(13) = time.at(1);
			}
		}
		else value->binaryValue.at(i - 1) = Math::getNumber(element);
	}
	value->type = VariableType::tBinary;
}

StringReplace::StringReplace(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

StringReplace::StringReplace(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"booleanString\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "search") search = Http::decodeURL(value);
		else if(name == "replace") replace = Http::decodeURL(value);
		else _bl->out.printWarning("Warning: Unknown node in \"stringReplace\": " + name);
	}
}

void StringReplace::fromPacket(PVariable value)
{
	if(!value) return;
	HelperFunctions::stringReplace(value->stringValue, replace, search);
}

void StringReplace::toPacket(PVariable value)
{
	if(!value) return;
	HelperFunctions::stringReplace(value->stringValue, search, replace);
}

HexStringByteArray::HexStringByteArray(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

HexStringByteArray::HexStringByteArray(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"hexStringByteArray\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"hexStringByteArray\": " + std::string(node->name()));
	}
}

void HexStringByteArray::fromPacket(PVariable value)
{
	if(!value) return;
	value->stringValue = _bl->hf.getHexString(value->stringValue);
}

void HexStringByteArray::toPacket(PVariable value)
{
	if(!value) return;
	if(value->stringValue.find(',') != std::string::npos)
	{
		std::vector<std::string> bytes = _bl->hf.splitAll(value->stringValue, ',');
		value->stringValue = "";
		value->stringValue.reserve(bytes.size() * 2);
		for(auto byte : bytes)
		{
			_bl->hf.trim(byte);
			if(byte.size() > 2) byte = byte.substr(2);
			if(byte.size() > 2) byte = byte.substr(0, 2);
			if(byte.size() == 1) value->stringValue.append("0" + byte);
			else value->stringValue.append(byte);
		}
	}
	value->stringValue = _bl->hf.getBinaryString(value->stringValue);
}

TimeStringSeconds::TimeStringSeconds(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

TimeStringSeconds::TimeStringSeconds(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"timestringDuration\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"timestringDuration\": " + std::string(subNode->name()));
	}
}

void TimeStringSeconds::fromPacket(PVariable value)
{
	if(!value) return;
	std::vector<std::string> parts = BaseLib::HelperFunctions::splitAll(value->stringValue, ':');
	value->integerValue = 0;
	value->type = VariableType::tInteger;
	int32_t j = 0;
	for(std::vector<std::string>::reverse_iterator i = parts.rbegin(); i != parts.rend(); i++, j++)
	{
		if(j == 0) value->integerValue += Math::getNumber(*i);
		else if(j == 1) value->integerValue += Math::getNumber(*i) * 60;
		else if(j == 2) value->integerValue += Math::getNumber(*i) * 3600;
	}
	value->stringValue.clear();
}

void TimeStringSeconds::toPacket(PVariable value)
{
	if(!value) return;
	value->type = VariableType::tString;
	std::ostringstream timeStream;
	timeStream << (value->integerValue / 3600) << ':' << std::setw(2) << std::setfill('0') << ((value->integerValue % 3600) / 60) << ':' << std::setw(2) << (value->integerValue % 60);
	value->stringValue = timeStream.str();
	value->integerValue = 0;
}

Invert::Invert(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

Invert::Invert(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"invert\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"invert\": " + std::string(subNode->name()));
	}
}

void Invert::fromPacket(PVariable value)
{
	if(!value) return;
	if(_parameter->logical->type == ILogical::Type::Enum::tBoolean) value->booleanValue = !value->booleanValue;
	else if(_parameter->logical->type == ILogical::Type::Enum::tInteger)
	{
		LogicalInteger* logical = (LogicalInteger*)_parameter->logical.get();
		value->integerValue = logical->maximumValue - (value->integerValue - logical->minimumValue);
	}
	else if(_parameter->logical->type == ILogical::Type::Enum::tInteger64)
	{
		LogicalInteger64* logical = (LogicalInteger64*)_parameter->logical.get();
		value->integerValue64 = logical->maximumValue - (value->integerValue64 - logical->minimumValue);
	}
	else if(_parameter->logical->type == ILogical::Type::Enum::tFloat)
	{
		LogicalDecimal* logical = (LogicalDecimal*)_parameter->logical.get();
		value->floatValue = logical->maximumValue - (value->floatValue - logical->minimumValue);
	}
}

void Invert::toPacket(PVariable value)
{
	if(!value) return;
	if(_parameter->logical->type == ILogical::Type::Enum::tBoolean) value->booleanValue = !value->booleanValue;
	else if(_parameter->logical->type == ILogical::Type::Enum::tInteger)
	{
		LogicalInteger* logical = (LogicalInteger*)_parameter->logical.get();
		value->integerValue = logical->maximumValue - (value->integerValue - logical->minimumValue);
	}
	else if(_parameter->logical->type == ILogical::Type::Enum::tInteger64)
	{
		LogicalInteger64* logical = (LogicalInteger64*)_parameter->logical.get();
		value->integerValue64 = logical->maximumValue - (value->integerValue64 - logical->minimumValue);
	}
	else if(_parameter->logical->type == ILogical::Type::Enum::tFloat)
	{
		LogicalDecimal* logical = (LogicalDecimal*)_parameter->logical.get();
		value->floatValue = logical->maximumValue - (value->floatValue - logical->minimumValue);
	}
}

Round::Round(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

Round::Round(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		_bl->out.printWarning("Warning: Unknown attribute for \"decimalPlaces\": " + std::string(attr->name()));
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string name(subNode->name());
		std::string value(subNode->value());
		if(name == "decimalPlaces")
		{
			if(value == "0.5")
			{
				decimalPlaces = 1;
				roundToPoint5 = true;
			}
			else decimalPlaces = Math::getNumber(value);
		}
		else _bl->out.printWarning("Warning: Unknown node in \"decimalPlaces\": " + name);
	}
}

void Round::fromPacket(PVariable value)
{
	if(!value) return;
	value->floatValue = std::round(value->floatValue * (roundToPoint5 ? 2.0 : Math::Pow10(decimalPlaces))) / (roundToPoint5 ? 2.0 : Math::Pow10(decimalPlaces));
}

void Round::toPacket(PVariable value)
{
	if(!value) return;
	value->floatValue = std::round(value->floatValue * (roundToPoint5 ? 2.0 : Math::Pow10(decimalPlaces))) / (roundToPoint5 ? 2.0 : Math::Pow10(decimalPlaces));
}

Generic::Generic(BaseLib::SharedObjects* baseLib) : ICast(baseLib)
{
}

Generic::Generic(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter) : ICast(baseLib, node, parameter)
{
	for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		std::string name(attr->name());
		std::string value(attr->value());
		if(name == "type") type = value;
		else _bl->out.printWarning("Warning: Unknown attribute for \"generic\": " + name);
	}
	for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		_bl->out.printWarning("Warning: Unknown node in \"generic\": " + std::string(subNode->name()));
	}
}

void Generic::fromPacket(PVariable value)
{
}

void Generic::toPacket(PVariable value)
{
}

}
}
}
