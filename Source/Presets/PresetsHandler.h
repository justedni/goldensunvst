#pragma once

#include "Presets.h"

#include "Processor/Instrument.h"
#include <string>
#include <map>

struct tsf;

namespace juce
{
    class AudioFormatManager;
}

namespace GSVST {

class Preset;

struct ProgramInfo
{
    std::string name;
    std::string device;
};

struct PresetsHandler
{
    PresetsHandler();
    ~PresetsHandler();

    void clear();
    void sort();

    virtual void addSynthsPresets() {}
    virtual const std::map<int, ProgramInfo>& getHandledProgramsList() const { return m_emptyMap; }

    void addSamplePreset(int id, std::string&& name, std::string&& filename, ADSR&& adsr, int pitch_correction, int original_pitch, int sample_rate);

    void setSoundfont(const std::string& path);
    void cleanupSoundfont();

    float* getSoundfontBuffer(unsigned int offset) const;

    const std::string& getSoundFontPath() const { return soundFontPath; }

    void setEnableGSMode(bool bEnable);
    bool getGSModeEnabled() const { return m_bGSModeEnabled; }

    void clearSynthPresets();
    void clearSoundfontPresets();

    std::vector<Preset*> m_presets;

    std::string soundFontPath;
    const tsf* soundFont = nullptr;

    bool m_bGSModeEnabled = true;

private:
    int calculateMidCFreq(int pitch_correction, int original_pitch, int sample_rate);

    void addSoundFontPresets();

    const std::map<int, ProgramInfo> m_emptyMap;

    std::unique_ptr<juce::AudioFormatManager> m_formatManager;
};

}