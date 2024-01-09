#include "RPNHandler.h"

#include "assert.h"

namespace GSVST {

RPNHandler::RPNHandler()
{
    rpns.emplace_back(RPN::Type::RPN,  RPN::Param::Detune, 0, 1, 0);
    rpns.emplace_back(RPN::Type::RPN,  RPN::Param::PitchBendRange, 0, 0, 0);
    rpns.emplace_back(RPN::Type::NRPN, RPN::Param::LfoSpeed, 1, 8, 40);
}

RPNHandler::~RPNHandler()
{
    rpns.clear();
}

void RPNHandler::setRPN_MSB(RPN::Type type, int msb, double timestamp)
{
    for (auto& rpn : rpns)
    {
        if (type == rpn.type && msb == rpn.msb)
            rpn.setMSB(timestamp);
        else
            rpn.resetMSB();
    }
}

void RPNHandler::setRPN_LSB(RPN::Type type, int lsb, double timestamp)
{
    for (auto& rpn : rpns)
    {
        if (type == rpn.type && lsb == rpn.lsb)
            rpn.setLSB(timestamp);
        else
            rpn.resetLSB();
    }
}


RPN::Param RPNHandler::setDataEntry(int controllerVal, double timestamp)
{
    auto paramSet = RPN::Param::None;

    for (auto& rpn : rpns)
    {
        if (rpn.isValid(timestamp))
        {
            switch (rpn.param)
            {
            case RPN::Param::Detune:
            {
                int16_t val = static_cast<int16_t>(controllerVal - 0x40);
                rpn.setValue(val);
                break;
            }
            case RPN::Param::PitchBendRange:
            case RPN::Param::LfoSpeed:
            {
                int16_t val = static_cast<int16_t>(controllerVal);
                rpn.setValue(val);
                break;
            }
            }

            assert(paramSet == RPN::Param::None);
            paramSet = rpn.param;
        }

    }

    return paramSet;
}

bool RPNHandler::hasValue(RPN::Param param) const
{
    for (auto& rpn : rpns)
    {
        if (rpn.param == param)
            return rpn.hasValue();
    }

    return false;
}

int16_t RPNHandler::getValue(RPN::Param param) const
{
    for (auto& rpn : rpns)
    {
        if (rpn.param == param)
            return rpn.getValue();
    }

    return 0;
}

void RPNHandler::resetAllRPNs()
{
    for (auto& rpn : rpns)
    {
        rpn.reset();
    }
}

}