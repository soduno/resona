# Resona DAW

Native C++/JUCE prototype for the simple Resona music workstation.

## What works in the first milestone

- Native resizable desktop window
- Timeline, mixer and MIDI piano-roll views
- Double-click from the timeline into the MIDI editor
- Transport controls and movable playhead
- A small built-in synth preview of the demo MIDI notes
- Keyboard shortcuts: `Enter`/`Space` play/pause, `0` return to start, `C` click track,
  `1` timeline, `2` mixer, `3` MIDI
- JUCE is configured for VST3 hosting; the scanner and plugin instances are the next engine milestone

## Architecture

Resona is split into small static libraries with one responsibility each:

- `resona_core` — project data and shared musical types
- `resona_transport` — playback, playhead and click-track state
- `resona_timeline` — timeline viewport, grid snapping and zoom behaviour
- `resona_midi_editor` — piano-roll viewport, zoom and note selection
- `resona_audio` — real-time synth and metronome rendering
- `resona_ui` — Windows/JUCE presentation and input routing

The executable in `src/app` only owns application startup. This keeps real-time audio,
editing rules and presentation independent so they can be tested and extended without
growing another monolithic application file.

## Build on Windows

Install Visual Studio 2022 with **Desktop development with C++**, plus CMake. Then run from a Developer PowerShell:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

The executable is generated below `build/Resona_artefacts/Release/`.

Install it for the current Windows user with:

```powershell
cmake --install build --config Release --component ResonaApp --prefix "$env:LOCALAPPDATA\Programs\Resona"
```

## Build on macOS

```bash
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

JUCE is fetched at the pinned `9.0.2` tag during configuration. Distribution must comply with the selected JUCE licence.
