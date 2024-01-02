#pragma once

#include <vector>

#define AGB_FPS 60
// for increased quality we process in subframes (including the base frame)
#define INTERFRAMES 4
#define MAX_MIDI_CHANNELS 16

namespace GSVST {

enum class EReverbType : uint8_t { None, Default, GS1, GS2 };

struct ADSR
{
    ADSR(uint8_t att, uint8_t dec, uint8_t sus, uint8_t rel)
    {
        this->att = att;
        this->dec = dec;
        this->sus = sus;
        this->rel = rel;
    }

    ADSR()
    {
        this->att = 0xFF;
        this->dec = 0x00;
        this->sus = 0xFF;
        this->rel = 0x00;
    }

    uint8_t att;
    uint8_t dec;
    uint8_t sus;
    uint8_t rel;
};

struct PWMData
{
    PWMData() {}

    PWMData(uint8_t in_initDuty, uint8_t in_dutybase, int32_t in_dutyStep, uint8_t in_depth)
    {
        this->initDuty = in_initDuty;
        this->dutyBase = in_dutybase;
        this->dutyStep = in_dutyStep;
        this->depth    = in_depth;
    }

    PWMData(const PWMData& other)
    {
        this->initDuty = other.initDuty;
        this->dutyBase = other.dutyBase;
        this->dutyStep = other.dutyStep;
        this->depth = other.depth;
    }

    uint8_t initDuty = 0;
    uint8_t dutyBase = 0;
    int32_t dutyStep = 0;
    uint8_t depth = 0;
};

struct Note
{
    //uint8_t length = 0;
    uint8_t midiKeyTrackData; // Midi note
    uint8_t midiKeyPitch; // Note pitch in the data
    uint8_t velocity = 100;
    //uint8_t priority;
    int8_t rhythmPan = 0;
    uint8_t pseudoEchoVol = 0;
    uint8_t pseudoEchoLen = 0;
    uint8_t trackIdx = 0;
};

struct sample {
    float left = 0.0f;
    float right = 0.0f;
};

struct MixingArgs
{
    float vol;
    float sampleRateInv;
    float samplesPerBufferInv;
    int samplesPerBufferForComputation;
};

}