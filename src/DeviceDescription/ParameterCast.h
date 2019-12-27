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
typedef std::shared_ptr<Parameter> PParameter;

namespace ParameterCast
{

class ICast
{
public:
	explicit ICast(BaseLib::SharedObjects* baseLib);
	explicit ICast(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	virtual ~ICast() = default;

	virtual bool needsBinaryPacketData() { return false; }
	virtual void fromPacket(PVariable& value);
	virtual void toPacket(PVariable& value);
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	const std::weak_ptr<Parameter> _parameter;
};

class DecimalIntegerScale : public ICast
{
public:
    explicit DecimalIntegerScale(BaseLib::SharedObjects* baseLib);
    explicit DecimalIntegerScale(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~DecimalIntegerScale() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	double factor = 1.0;
	double offset = 0;
};

class DecimalStringScale : public ICast
{
public:
    explicit DecimalStringScale(BaseLib::SharedObjects* baseLib);
    explicit DecimalStringScale(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
    ~DecimalStringScale() override = default;

    void fromPacket(PVariable& value) override;
    void toPacket(PVariable& value) override;

    //Elements
    double factor = 1.0;
};

class IntegerIntegerScale : public ICast
{
public:
	struct Operation
	{
		enum Enum { none = 0, division = 1, multiplication = 2 };
	};

    explicit IntegerIntegerScale(BaseLib::SharedObjects* baseLib);
    explicit IntegerIntegerScale(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~IntegerIntegerScale() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	Operation::Enum operation = Operation::none;
	double factor = 10;
	int32_t offset = 0;
};

class IntegerOffset : public ICast
{
public:
    explicit IntegerOffset(BaseLib::SharedObjects* baseLib);
    explicit IntegerOffset(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~IntegerOffset() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	bool directionToPacket = true;
	bool addOffset = false;
	int32_t offset = 0;
};

class DecimalOffset : public ICast
{
public:
    explicit DecimalOffset(BaseLib::SharedObjects* baseLib);
    explicit DecimalOffset(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~DecimalOffset() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

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

    explicit IntegerIntegerMap(BaseLib::SharedObjects* baseLib);
    explicit IntegerIntegerMap(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~IntegerIntegerMap() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	Direction::Enum direction = Direction::none;
	std::map<int32_t, int32_t> integerValueMapFromDevice;
	std::map<int32_t, int32_t> integerValueMapToDevice;
};

class BooleanInteger : public ICast
{
public:
    explicit BooleanInteger(BaseLib::SharedObjects* baseLib);
    explicit BooleanInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~BooleanInteger() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	int32_t trueValue = 0;
	int32_t falseValue = 0;
	bool invert = false;
	int32_t threshold = 1;
};

class BooleanString : public ICast
{
public:
    explicit BooleanString(BaseLib::SharedObjects* baseLib);
    explicit BooleanString(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~BooleanString() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	std::string trueValue;
	std::string falseValue;
	bool invert = false;
};

class DecimalConfigTime : public ICast
{
public:
    explicit DecimalConfigTime(BaseLib::SharedObjects* baseLib);
    explicit DecimalConfigTime(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~DecimalConfigTime() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	std::vector<double> factors;
	double valueSize = 0;
};

class IntegerTinyFloat : public ICast
{
public:
    explicit IntegerTinyFloat(BaseLib::SharedObjects* baseLib);
    explicit IntegerTinyFloat(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~IntegerTinyFloat() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	int32_t mantissaStart = 5;
	int32_t mantissaSize = 11;
	int32_t exponentStart = 0;
	int32_t exponentSize = 5;
};

class StringUnsignedInteger : public ICast
{
public:
    explicit StringUnsignedInteger(BaseLib::SharedObjects* baseLib);
    explicit StringUnsignedInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~StringUnsignedInteger() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class BlindTest : public ICast
{
public:
    explicit BlindTest(BaseLib::SharedObjects* baseLib);
    explicit BlindTest(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~BlindTest() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	int32_t value = 0;
};

class OptionString : public ICast
{
public:
    explicit OptionString(BaseLib::SharedObjects* baseLib);
    explicit OptionString(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~OptionString() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class OptionInteger : public ICast
{
public:
    explicit OptionInteger(BaseLib::SharedObjects* baseLib);
    explicit OptionInteger(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~OptionInteger() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	std::map<int32_t, int32_t> valueMapFromDevice;
	std::map<int32_t, int32_t> valueMapToDevice;
};

class StringJsonArrayDecimal : public ICast
{
public:
    explicit StringJsonArrayDecimal(BaseLib::SharedObjects* baseLib);
    explicit StringJsonArrayDecimal(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~StringJsonArrayDecimal() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class RpcBinary : public ICast
{
public:
    explicit RpcBinary(BaseLib::SharedObjects* baseLib);
    explicit RpcBinary(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~RpcBinary() override = default;

	bool needsBinaryPacketData() override { return true; }
	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
private:
	//Helpers
	std::shared_ptr<BaseLib::Rpc::RpcDecoder> _binaryDecoder;
	std::shared_ptr<BaseLib::Rpc::RpcEncoder> _binaryEncoder;
};

class Toggle : public ICast
{
public:
    explicit Toggle(BaseLib::SharedObjects* baseLib);
    explicit Toggle(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~Toggle() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	std::string parameter;
	int32_t on = 200;
	int32_t off = 0;
};

class CcrtdnParty : public ICast
{
public:
    explicit CcrtdnParty(BaseLib::SharedObjects* baseLib);
    explicit CcrtdnParty(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~CcrtdnParty() override = default;

	bool needsBinaryPacketData() override { return true; }
	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class Cfm : public ICast
{
public:
    explicit Cfm(BaseLib::SharedObjects* baseLib);
    explicit Cfm(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~Cfm() override = default;

	bool needsBinaryPacketData() override { return true; }
	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class StringReplace : public ICast
{
public:
    explicit StringReplace(BaseLib::SharedObjects* baseLib);
    explicit StringReplace(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~StringReplace() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	std::string search;
	std::string replace;
};

class HexStringByteArray : public ICast
{
public:
    explicit HexStringByteArray(BaseLib::SharedObjects* baseLib);
    explicit HexStringByteArray(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~HexStringByteArray() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class TimeStringSeconds : public ICast
{
public:
    explicit TimeStringSeconds(BaseLib::SharedObjects* baseLib);
    explicit TimeStringSeconds(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~TimeStringSeconds() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class Invert : public ICast
{
public:
    explicit Invert(BaseLib::SharedObjects* baseLib);
    explicit Invert(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~Invert() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;
};

class Round : public ICast
{
public:
    explicit Round(BaseLib::SharedObjects* baseLib);
    explicit Round(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~Round() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

	//Elements
	bool roundToPoint5 = false;
	int32_t decimalPlaces = 1;
};

class Generic : public ICast
{
public:
    explicit Generic(BaseLib::SharedObjects* baseLib);
    explicit Generic(BaseLib::SharedObjects* baseLib, xml_node<>* node, const PParameter& parameter);
	~Generic() override = default;

	void fromPacket(PVariable& value) override;
	void toPacket(PVariable& value) override;

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
typedef std::shared_ptr<DecimalStringScale> PDecimalStringScale;
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
