#pragma once

#include "Presets.h"

#include "Processor/Instrument.h"
#include <string>
#include <map>

struct tsf;
struct tsf_preset;
struct tsf_region;

namespace juce
{
    class AudioFormatManager;
}

namespace GSVST {

class Preset;

struct ProgramInfo
{
    int bankid;
    int programid;
    std::string name;
    std::string device;
    EDSPType type = EDSPType::PCM;
    bool bDisplay = true;
};

struct PresetsHandler
{
    PresetsHandler();
    virtual ~PresetsHandler();

    void clear();
    void sort();

    const std::list<ProgramInfo>& getProgramInfo() const;

    typedef std::map<std::string, std::list<ProgramInfo>> ProgramList;
    virtual const ProgramList& getGamesProgramList() const { return m_emptyGameList; }

    virtual void addSynthsPresets() {}
    virtual Preset* buildCustomSynthPreset(unsigned short, const std::string&, const ADSR&) { return nullptr; }

    void addSamplePreset(int bankid, int programid, std::string&& name, std::string&& filename, ADSR&& adsr, int pitch_correction, int original_pitch, int sample_rate);
    void addSamplePresetLooping(int bankid, int programid, std::string&& name, std::string&& filepath, ADSR&& adsr, int pitch_correction, int original_pitch, int sample_rate, uint32_t in_loopPos, uint32_t in_endPos);

    void setSoundfont(const std::string& path);
    void cleanupSoundfont();

    float* getSoundfontBuffer(unsigned int offset) const;

    const std::string& getSoundFontPath() const { return soundFontPath; }

    void setAutoReplaceGSSynths(bool bEnable);
    bool getAutoReplaceGSSynths() const { return m_bAutoReplaceGSSynthsEnabled; }
    void setAutoReplaceGBSynths(bool bEnable);
    bool getAutoReplaceGBSynths() const { return m_bAutoReplaceGBSynthsEnabled; }
    void setHideUnknownInstruments(bool bHide);
    bool getHideUnknownInstruments() const { return m_bHideUnknownInstruments; }

    void setSelectedGame(const std::string& gameName);
    const std::string& getSelectedGame() const { return m_selectedGame; }

    void clearPresetsOfType(EPresetType type);

    const ProgramInfo* findProgramInfo(const std::list<ProgramInfo>& list, int bankid, int programid) const;

    std::vector<Preset*> m_presets;

    std::string soundFontPath;
    const tsf* soundFont = nullptr;

    bool m_bAutoReplaceGSSynthsEnabled = true;
    bool m_bAutoReplaceGBSynthsEnabled = true;
    bool m_bHideUnknownInstruments = false;
    std::string m_selectedGame;

protected:
    int calculateMidCFreq(int pitch_correction, int original_pitch, int sample_rate);

    void addSoundFontPresets();
    Preset* buildSoundfontPreset(const tsf_preset& preset, const std::string& name, const std::string& friendlyName);

    bool isGSSynth(int bankId, const std::string& presetName);
    bool isGBSynth(const std::string& presetName);
    bool isSynth(const tsf_preset& preset);
    ADSR getSoundfontADSR(const tsf_region& region);

    const ProgramList m_emptyGameList;
    const std::list<ProgramInfo> m_emptyList;

    std::unique_ptr<juce::AudioFormatManager> m_formatManager;
};

}