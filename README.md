# MIDILOOPER

This code is in development.

## Build instructions

    git submodule update --recursive --init
    mkdir <BUILD_DIR>
    cd <BUILD_DIR>
    cmake <MIDILOOPER_DIR>
    make

## Features

* Loop Midi tracks over "Alsa sequencer" or "Jack"
* Midi track editor
* Midifile reader and writer
* Remote keyboard and midi to mute unmute tracks
* Record midi events

## Roadmap (todo)

* Add New Session Manager support (jack session now unsupported)
* Sync
  * Improve jack transport
  * Add midi sync (In Out All)
  * Tap tempo (MIDI and Interface)
* Improve graphic interface
  * Imgui implementation (fonts, update)
  * Main window
    * Colored track
  * Editor
    * Add keyboard binding to piano in editor
* User helper and documentation
* ???
  * OSC
  * plugin (lv2, dssi, vst???)
* Clean
  * Remove all hacks
  * Factorize
