#pragma once

#include "Types.h"
#include "Resampler.h"
#include <memory>

namespace GSVST {

struct ProcArgs
{
    float lVol = 0.0f;
    float rVol = 0.0f;
    float lVolStep = 0.0f;
    float rVolStep = 0.0f;
    float interStep = 0.0f;
    bool bInitialized = false;
};

enum class EDSPType : uint8_t
{
    PCM = 0,
    PCMFixed,
    ModPulse,
    Saw,
    Tri,
    Square
};
class Instrument
{
public:
    Instrument(const Note& in_note)
        : note(in_note)
    {}
    virtual ~Instrument() {}

    Instrument(const Instrument&) = delete;
    Instrument& operator=(const Instrument&) = delete;

    virtual void processCommon(sample* buffer, size_t numSamples, const MixingArgs& args);

    virtual EDSPType getType() const = 0;
    virtual void process(sample* buffer, size_t numSamples, const MixingArgs& args) = 0;
    virtual void updateArgs(const MixingArgs& args);
    virtual int getMidCFreq() const = 0;
    int8_t getMidiKeyPitch() const { return note.midiKeyPitch; }
    uint8_t getMidiNote() const { return note.midiKeyTrackData; }

    virtual void processStart(const MixingArgs& args, size_t currentSample, size_t numSamples);
    virtual void processEnd(const MixingArgs& args, size_t currentSample);

    struct VolumeFade
    {
        float fromVolLeft;
        float fromVolRight;
        float toVolLeft;
        float toVolRight;
    };

    virtual VolumeFade getVol() const;
    void setVol(uint8_t in_vol);
    uint8_t getLfoVol() const;
    int16_t getPitch() const;
    int8_t getPan() const;
    void setPan(int8_t in_pan);
    void updateVolAndPan();

    void useTrackADSR(bool bUse) { bUseTrackADSR = bUse; }
    bool shouldUseTrackADSR() const { return bUseTrackADSR; }

    const auto& getADSR() const { return env; }
    void updateADSR(const ADSR& in_adsr);

    virtual void updatePWMData(const PWMData&) {}

    void setLfoType(ELfoType type);
    void setLfoSpeed(uint8_t speed);
    void setLfoDepth(uint8_t depth);
    void setPitchWheel(int16_t pitch);
    void setPitchBendRange(int16_t range);
    void setTune(int16_t in_tune);
    virtual void updatePitch();

    void setBPM(int bpm) { bpmRefresh = bpm; }

    virtual void release();
    bool isStopping() const;
    void kill();

    void tickLfo();

    enum class EnvState : int { INIT = 0, ATK, DEC, SUS, REL, PSEUDO_ECHO, DIE, DEAD };
    bool isDead() const { return envState == EnvState::DEAD; }

    void addPendingVolChange(int timestamp, int volume);
    bool hasPendingNoteOff() const;
    void addPendingNoteOff(int timestamp);
protected:
    void handlePendingVolumeChanges(int sampleId);
    void handlePendingNoteOff(int sampleId);

    void updateIncrements(const MixingArgs& args);
    void updateBPMStack();

    virtual void stepEnvelope();
    virtual void updateVolFade();

    /* all of these values have pairs of new and old value to allow smooth fades */
    EnvState envState = EnvState::INIT;
    uint8_t envInterStep = 0;
    uint8_t envLevelCur;
    uint8_t envLevelPrev;
    uint8_t leftVolCur = 0;
    uint8_t leftVolPrev = 0;
    uint8_t rightVolCur = 0;
    uint8_t rightVolPrev = 0;

    ELfoType lfoType = ELfoType::Pitch;
    uint8_t lfoPhase = 0;
    int8_t lfoSpeed = 0;
    int8_t lfoValue = 0;

    int envSampleCount = 0;
    uint32_t bpmStack = 0;
    ProcArgs cargs;

    int bpmRefresh = 120;

    uint8_t vol = 0;
    int8_t pan = 0;

    int16_t pitchWheel = 0;
    int16_t pitchBendRange = 2;
    int16_t modWheel = 0;

    int16_t tune = 0;
    float freq = 0.0f;

    bool stop = false;

    ADSR env;
    Note note;

    bool bUseTrackADSR = true;

    std::vector<VolChange> pendingVolChanges;
    std::vector<double> pendingNoteOff;
};

}