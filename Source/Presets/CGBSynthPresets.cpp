#include "CGBSynthPresets.h"
#include "Processor/CGBChannel.h"

namespace GSVST {

//-----------------------------------------------------------------------------
Instrument* SquareSynthPreset::createPlayingInstance(const Note& note) const
{
    return new SquareChannel(dutyCycle, note, 0);
}

}
