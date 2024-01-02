#pragma once

#include <JuceHeader.h>

namespace GSVST {

enum class EProperty : uint8_t
{
    Volume,
    Pan,
    Reverb,
    Attack,
    Decay,
    Sustain,
    Release,
    InitDuty,
    DutyBase,
    DutyStep,
    Depth
};

class PropertyListener
{
public:
    virtual void setPropertyVal(EProperty prop, uint8_t val) = 0;
};

}
