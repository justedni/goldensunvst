#pragma once

#include "Types.h"

namespace GSVST {

struct RPN
{
    enum class Type { RPN, NRPN };
    enum class Param { None, Detune, PitchBendRange, LfoSpeed };

    RPN(Type in_type, Param in_param, uint8_t in_msb, uint8_t in_lsb, int16_t in_defaultVal)
        : type(in_type)
        , param(in_param)
        , msb(in_msb)
        , lsb(in_lsb)
        , value(in_defaultVal)
        , bValueSet(true)
    {
    }

    void setMSB(double in_timestamp)
    {
        if (in_timestamp >= timestamp)
        {
            validMsb = true;
            timestamp = in_timestamp;
        }
        else
        {
            validMsb = false;
            timestamp = 0.0;
        }
    }

    void resetMSB()
    {
        validMsb = false;
        timestamp = 0.0;
    }

    void setLSB(double in_timestamp)
    {
        if (in_timestamp >= timestamp)
        {
            validLsb = true;
            timestamp = in_timestamp;
        }
        else
        {
            validLsb = false;
            timestamp = 0.0;
        }
    }

    void resetLSB()
    {
        validLsb = false;
        timestamp = 0.0;
    }

    bool hasValue() const { return bValueSet; }
    int16_t getValue() const { return value; }
    void setValue(int16_t in_val)
    {
        value = in_val;
        bValueSet = true;
    }

    bool isValid(double in_timestamp) const
    {
        return (validMsb && validLsb && in_timestamp >= timestamp);
    }

    void reset()
    {
        validMsb = false;
        validLsb = false;
    }

    const Param param = Param::Detune;

protected:
    friend struct RPNHandler;
    const Type type = Type::RPN;
    const uint8_t msb = 0;
    const uint8_t lsb = 0;

private:
    int16_t value = 0;
    bool bValueSet = false;

    bool validMsb = false;
    bool validLsb = false;
    double timestamp = 0.0;
};

struct RPNHandler
{
    RPNHandler();
    ~RPNHandler();

    void setRPN_MSB(RPN::Type type, int msb, double timestamp);
    void setRPN_LSB(RPN::Type type, int lsb, double timestamp);
    RPN::Param setDataEntry(int controllerVal, double timestamp);
    bool hasValue(RPN::Param param) const;
    int16_t getValue(RPN::Param param) const;
    void resetAllRPNs();

    std::vector<RPN> rpns;
};

}