#pragma once

#include "Presets.h"

#include <string>

namespace GSVST {

enum class WaveDuty : int;

class SquareSynthPreset : public Preset
{
public:
    SquareSynthPreset(int in_bankid, int in_programid, std::string&& in_name, ADSR&& in_adsr, WaveDuty in_dutyCycle)
        : Preset(in_bankid, in_programid, EPresetType::Synth, std::move(in_name))
        , adsr(std::move(in_adsr))
        , dutyCycle(in_dutyCycle)
    {}

    Instrument* createPlayingInstance(const Note& note) const final;
    EDSPType getDSPType() const final { return EDSPType::Square; }
    const ADSR& getADSR() const final { return adsr; }

private:
    const ADSR adsr;
    const WaveDuty dutyCycle;
};

}