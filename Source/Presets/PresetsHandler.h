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

enum class EProgramNameMode : uint8_t { Sf2, GS };

struct ProgramInfo
{
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

    const std::map<int, ProgramInfo>& getProgramInfo() const;
    virtual const std::map<int, ProgramInfo>& getCustomProgramList() const { return m_emptyMap; }

    virtual void addSynthsPresets() {}
    virtual Preset* buildCustomSynthPreset(unsigned short, const std::string&, const ADSR&) { return nullptr; }

    void addSamplePreset(int id, std::string&& name, std::string&& filename, ADSR&& adsr, int pitch_correction, int original_pitch, int sample_rate);
    void addSamplePresetLooping(int id, std::string&& name, std::string&& filepath, ADSR&& adsr, int pitch_correction, int original_pitch, int sample_rate, uint32_t in_loopPos, uint32_t in_endPos);

    void setSoundfont(const std::string& path);
    void cleanupSoundfont();

    float* getSoundfontBuffer(unsigned int offset) const;

    const std::string& getSoundFontPath() const { return soundFontPath; }

    void setAutoReplaceGSSynths(bool bEnable);
    bool getAutoReplaceGSSynths() const { return m_bAutoReplaceGSSynthsEnabled; }
    void setAutoReplaceGBSynths(bool bEnable);
    bool getAutoReplaceGBSynths() const { return m_bAutoReplaceGBSynthsEnabled; }

    void setProgramNameMode(EProgramNameMode mode);
    EProgramNameMode getProgramNameMode() const { return m_programNameMode; }

    void clearPresetsOfType(EPresetType type);

    std::vector<Preset*> m_presets;

    std::string soundFontPath;
    const tsf* soundFont = nullptr;

    bool m_bAutoReplaceGSSynthsEnabled = true;
    bool m_bAutoReplaceGBSynthsEnabled = true;
    EProgramNameMode m_programNameMode = EProgramNameMode::GS;

protected:
    int calculateMidCFreq(int pitch_correction, int original_pitch, int sample_rate);

    void addSoundFontPresets();
    Preset* buildSoundfontPreset(const tsf_preset& preset, const std::string& name, const std::string& friendlyName);

    bool isGSSynth(const std::string& presetName);
    bool isGBSynth(const std::string& presetName);
    bool isSynth(const tsf_preset& preset);
    ADSR getSoundfontADSR(const tsf_region& region);

    const std::map<int, ProgramInfo> m_emptyMap;

    std::unique_ptr<juce::AudioFormatManager> m_formatManager;
};

}