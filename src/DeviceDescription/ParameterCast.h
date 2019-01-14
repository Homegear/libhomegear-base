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

#ifndef DEVICEPARAMETERCAST_H_
#define DEVICEPARAMETERCAST_H_

#include "../Variable.h"
#include "../Encoding/RapidXml/rapidxml.hpp"
#include <string>

using namespace rapidxml;

namespace BaseLib
{

namespace Rpc
{
	class RpcEncoder;
	class RpcDecoder;
}

namespace DeviceDescription
{

class Parameter;

namespace ParameterCast
{

class ICast
{
public:
	ICast(BaseLib::SharedObjects* baseLib);
	ICast(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~ICast() {}

	virtual bool needsBinaryPacketData() { return false; }
	virtual void fromPacket(PVariable value);
	virtual void toPacket(PVariable value);
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	Parameter* _parameter = nullptr;
};

class DecimalIntegerScale : public ICast
{
public:
	DecimalIntegerScale(BaseLib::SharedObjects* baseLib);
	DecimalIntegerScale(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~DecimalIntegerScale() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	double factor = 1.0;
	double offset = 0;
};

class IntegerIntegerScale : public ICast
{
public:
	struct Operation
	{
		enum Enum { none = 0, division = 1, multiplication = 2 };
	};

	IntegerIntegerScale(BaseLib::SharedObjects* baseLib);
	IntegerIntegerScale(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~IntegerIntegerScale() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	Operation::Enum operation = Operation::none;
	double factor = 10;
	int32_t offset = 0;
};

class IntegerOffset : public ICast
{
public:
	IntegerOffset(BaseLib::SharedObjects* baseLib);
	IntegerOffset(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~IntegerOffset() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	bool directionToPacket = true;
	bool addOffset = false;
	int32_t offset = 0;
};

class DecimalOffset : public ICast
{
public:
	DecimalOffset(BaseLib::SharedObjects* baseLib);
	DecimalOffset(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~DecimalOffset() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	bool directionToPacket = true;
	bool addOffset = false;
	double offset = 0;
};

class IntegerIntegerMap : public ICast
{
public:
	struct Direction
	{
		enum Enum { none = 0, fromDevice = 1, toDevice = 2, both = 3 };
	};

	IntegerIntegerMap(BaseLib::SharedObjects* baseLib);
	IntegerIntegerMap(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~IntegerIntegerMap() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	Direction::Enum direction = Direction::none;
	std::map<int32_t, int32_t> integerValueMapFromDevice;
	std::map<int32_t, int32_t> integerValueMapToDevice;
};

class BooleanInteger : public ICast
{
public:
	BooleanInteger(BaseLib::SharedObjects* baseLib);
	BooleanInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~BooleanInteger() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	int32_t trueValue = 0;
	int32_t falseValue = 0;
	bool invert = false;
	int32_t threshold = 1;
};

class BooleanString : public ICast
{
public:
	BooleanString(BaseLib::SharedObjects* baseLib);
	BooleanString(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~BooleanString() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	std::string trueValue;
	std::string falseValue;
	bool invert = false;
};

class DecimalConfigTime : public ICast
{
public:
	DecimalConfigTime(BaseLib::SharedObjects* baseLib);
	DecimalConfigTime(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~DecimalConfigTime() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	std::vector<double> factors;
	double valueSize = 0;
};

class IntegerTinyFloat : public ICast
{
public:
	IntegerTinyFloat(BaseLib::SharedObjects* baseLib);
	IntegerTinyFloat(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~IntegerTinyFloat() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	int32_t mantissaStart = 5;
	int32_t mantissaSize = 11;
	int32_t exponentStart = 0;
	int32_t exponentSize = 5;
};

class StringUnsignedInteger : public ICast
{
public:
	StringUnsignedInteger(BaseLib::SharedObjects* baseLib);
	StringUnsignedInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~StringUnsignedInteger() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class BlindTest : public ICast
{
public:
	BlindTest(BaseLib::SharedObjects* baseLib);
	BlindTest(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~BlindTest() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	int32_t value = 0;
};

class OptionString : public ICast
{
public:
	OptionString(BaseLib::SharedObjects* baseLib);
	OptionString(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~OptionString() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class OptionInteger : public ICast
{
public:
	OptionInteger(BaseLib::SharedObjects* baseLib);
	OptionInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~OptionInteger() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	std::map<int32_t, int32_t> valueMapFromDevice;
	std::map<int32_t, int32_t> valueMapToDevice;
};

class StringJsonArrayDecimal : public ICast
{
public:
	StringJsonArrayDecimal(BaseLib::SharedObjects* baseLib);
	StringJsonArrayDecimal(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~StringJsonArrayDecimal() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class RpcBinary : public ICast
{
public:
	RpcBinary(BaseLib::SharedObjects* baseLib);
	RpcBinary(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~RpcBinary() {}

	bool needsBinaryPacketData() { return true; }
	void fromPacket(PVariable value);
	void toPacket(PVariable value);
private:
	//Helpers
	std::shared_ptr<BaseLib::Rpc::RpcDecoder> _binaryDecoder;
	std::shared_ptr<BaseLib::Rpc::RpcEncoder> _binaryEncoder;
};

class Toggle : public ICast
{
public:
	Toggle(BaseLib::SharedObjects* baseLib);
	Toggle(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~Toggle() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	std::string parameter;
	int32_t on = 200;
	int32_t off = 0;
};

class CcrtdnParty : public ICast
{
public:
	CcrtdnParty(BaseLib::SharedObjects* baseLib);
	CcrtdnParty(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~CcrtdnParty() {}

	bool needsBinaryPacketData() { return true; }
	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class Cfm : public ICast
{
public:
	Cfm(BaseLib::SharedObjects* baseLib);
	Cfm(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~Cfm() {}

	bool needsBinaryPacketData() { return true; }
	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class StringReplace : public ICast
{
public:
	StringReplace(BaseLib::SharedObjects* baseLib);
	StringReplace(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~StringReplace() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	std::string search;
	std::string replace;
};

class HexStringByteArray : public ICast
{
public:
	HexStringByteArray(BaseLib::SharedObjects* baseLib);
	HexStringByteArray(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~HexStringByteArray() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class TimeStringSeconds : public ICast
{
public:
	TimeStringSeconds(BaseLib::SharedObjects* baseLib);
	TimeStringSeconds(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~TimeStringSeconds() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class Invert : public ICast
{
public:
	Invert(BaseLib::SharedObjects* baseLib);
	Invert(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~Invert() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);
};

class Round : public ICast
{
public:
	Round(BaseLib::SharedObjects* baseLib);
	Round(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~Round() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	bool roundToPoint5 = false;
	int32_t decimalPlaces = 1;
};

class Generic : public ICast
{
public:
	Generic(BaseLib::SharedObjects* baseLib);
	Generic(BaseLib::SharedObjects* baseLib, xml_node<>* node, Parameter* parameter);
	virtual ~Generic() {}

	void fromPacket(PVariable value);
	void toPacket(PVariable value);

	//Elements
	std::string type;
};

typedef std::shared_ptr<ICast> PICast;
typedef std::vector<PICast> Casts;
typedef std::shared_ptr<BlindTest> PBlindTest;
typedef std::shared_ptr<BooleanInteger> PBooleanInteger;
typedef std::shared_ptr<BooleanString> PBooleanString;
typedef std::shared_ptr<CcrtdnParty> PCcrtdnParty;
typedef std::shared_ptr<Cfm> PCfm;
typedef std::shared_ptr<DecimalConfigTime> PDecimalConfigTime;
typedef std::shared_ptr<DecimalIntegerScale> PDecimalIntegerScale;
typedef std::shared_ptr<IntegerIntegerMap> PIntegerIntegerMap;
typedef std::shared_ptr<IntegerIntegerScale> PIntegerIntegerScale;
typedef std::shared_ptr<IntegerOffset> PIntegerOffset;
typedef std::shared_ptr<DecimalOffset> PDecimalOffset;
typedef std::shared_ptr<IntegerTinyFloat> PIntegerTinyFloat;
typedef std::shared_ptr<OptionString> POptionString;
typedef std::shared_ptr<OptionInteger> POptionInteger;
typedef std::shared_ptr<RpcBinary> PRpcBinary;
typedef std::shared_ptr<StringJsonArrayDecimal> PStringJsonArrayDecimal;
typedef std::shared_ptr<StringUnsignedInteger> PStringUnsignedInteger;
typedef std::shared_ptr<Toggle> PToggle;
typedef std::shared_ptr<StringReplace> PStringReplace;
typedef std::shared_ptr<HexStringByteArray> PHexStringByteArray;
typedef std::shared_ptr<TimeStringSeconds> PTimeStringSeconds;
typedef std::shared_ptr<Invert> PInvert;
typedef std::shared_ptr<Round> PRound;
typedef std::shared_ptr<Generic> PGeneric;

}
}
}
#endif
