# MIDILOOPER

This code is in development.
It is a midi looper inspired of seq24 and ableton/live.

## Build instruction example

    git submodule update --recursive --init
    mkdir build
    cd build
    cmake ..
    make

## Features

* Loop Midi tracks
  * High precision engine
    * clock_nanosleep + alsa sequence
    * Jack
* Midi track editor
* Midifile reader and writer
* Remote keyboard and midi to mute unmute tracks
* Record midi events
* NSM Compatible

## Road-map (to-do)

* Prevent all kind of error
* Sync
  * Improve jack transport
  * Add midi sync (In Out All)
  * Tap tempo (MIDI and Interface)
* Improve graphic interface
  * Check and modify all labels (ex: channels label 1-16)
  * Main window
    * Button show editor
    * Column handling
    * Colored track
  * Editor
    * Improve mouse interaction
      * Add touchscreen capabilities
        * Add modal button (select, write, move, resize, ...)
    * Add scale selector
    * Add keyboard binding to piano in editor
  * Improve interface API
    * Improve dialog interface
    * Fully functional on RPI3
  * Add multiple rec mode (overwrite, last loop, inc loop size, ...)
* Add midi effects
* User helper and documentation
* Clean
  * Remove all hacks
  * Factorize
* Compatibility ???
  * OSC
  * plugin (lv2, dssi, vst???)
