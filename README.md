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
* Remote keyboard and midi to mute un-mute tracks
* Record midi events
* NSM Compatible

## Roadmap (todo)

* Sync
  * Improve jack transport
  * Add midi sync (In Out All)
  * Tap tempo (MIDI and Interface)
* Improve graphic interface
  * Main window
    * Colored track
  * Editor
    * Improve mouse interaction
      * Add touchscreen capabilities
        * Add modal button (select, write, ...)
    * Add scale selector
    * Add keyboard binding to piano in editor
  * Imgui implementation (fonts, update)
* User helper and documentation
* Clean
  * Remove all hacks
  * Factorize
* Compatibility ???
  * OSC
  * plugin (lv2, dssi, vst???)
