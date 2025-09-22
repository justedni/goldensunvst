<p align="left">
<a href="https://github.com/justedni/goldensunvst/actions/workflows/build-windows.yml?query=event%3Apush"><img src="https://github.com/justedni/goldensunvst/actions/workflows/build-windows.yml/badge.svg" /></a>
<a href="https://github.com/justedni/goldensunvst/actions/workflows/build-macos.yml?query=event%3Apush"><img src="https://github.com/justedni/goldensunvst/actions/workflows/build-macos.yml/badge.svg" /></a>
</p>

# goldensunvst

## goldensunvst
__goldensunvst__ is a VST plugin to play and experiment with music from the two Golden Sun games on the GBA.
It is fully compatible with MIDI and SF2 files exported with GBA Mus Ripper.
The low-level algorithms are based on [agbplay](https://github.com/ipatix/agbplay) by ipatix.

The goal of this plugin is to offer an alternative to general soundfont plugins with the following features:
- most accurate versions of the Golden Sun specific synthesizers (thanks to ipatix disassembly work)
- Golden Sun 1&2 specific reverb effects (again, thanks to ipatix)
- Proper ADSR factors
- GBA-specific LFO: its speed is linked to the song BPM
- Possibility to use custom-made soundfonts with improved HQ samples

### Screenshot
![Screenshot](https://github.com/justedni/goldensunvst/assets/155494991/225c319b-8da4-4bbd-a306-17494d0550dc)

### Build
- Download latest [Juce framework](https://github.com/juce-framework/JUCE)
- Launch Projucer.exe and open GoldenSunVST.jucer. Generate the required Exporters (default provided one is for Visual Studio 2022). Compile.

### Quick Start
- Copy the GoldenSunVST.dll in your VST folder. Typically this can be C:\Program Files\Steinberg\VSTPlugins
- Open your DAW (I like to use Reaper). Import a midi file (it is recommended to use a file exported with [GBA Mus Ripper](https://github.com/CaptainSwag101/gba-mus-ripper))
- Create an instance of the VST and route the midi tracks to it.
- Click on the "Settings" button and load a Soundfont file. Again, it is recommended to use a sf2 generated with GBA Mus Ripper.
- If your midi file contains standard program change events (and volume, pan, pitch etc), the correct instruments will be set on each channel and you should immediately hear sound!

### Dependencies
I'm using the following library, just slightly modified to remove some compiler warnings:
 - [TinySoundFont](https://github.com/schellingb/TinySoundFont)


### Current limitations and TODO list
- for now, the VST only includes a partial port of agbplay's code. I only took what I needed for Golden Sun 1 & 2. The VST was only tested on those two games (which is why it's called "Golden Sun VST" and not "GBA VST", I plan to make it work with other GBA games in the future.
- some midi events are not handled yet
- parameter automation is not implemented yet
- soundfont samples are only "mono" for now. This is not a limitation if you use GBA Mus Ripper soundfonts. Stereo samples (basically, two regions, one with 100% left pan and another with 100% right) will be handled later.
- you can experiment with ADSR and PWM sliders but I haven't implemented a "save new preset" feature yet. All included presets are read-only and your changes won't be saved when you close your DAW.

### Thanks
Massive thanks to __ipatix__ for his incredible work on agbplay.

Special thanks to __FreeJusticeHere__ for testing and giving me feedback on the plugin. And for his invaluable knowledge on Golden Sun samples!
