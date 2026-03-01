cmake_minimum_required(VERSION 3.22)

project(Resona VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 9.0.2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(resona_material_icons
  NAMESPACE ResonaMaterialIcons
  SOURCES
    assets/material_symbols/add_24px.svg
    assets/material_symbols/arrow_drop_down_24px.svg
    assets/material_symbols/close_24px.svg
    assets/material_symbols/crop_square_24px.svg
    assets/material_symbols/fiber_manual_record_24px.svg
    assets/material_symbols/keyboard_arrow_left_24px.svg
    assets/material_symbols/keyboard_arrow_right_24px.svg
    assets/material_symbols/minimize_24px.svg
    assets/material_symbols/music_note_24px.svg
    assets/material_symbols/piano_24px.svg
    assets/material_symbols/pause_24px.svg
    assets/material_symbols/play_arrow_24px.svg
    assets/material_symbols/settings_24px.svg
    assets/material_symbols/stop_24px.svg
    assets/material_symbols/tune_24px.svg
    assets/material_symbols/view_timeline_24px.svg
    assets/material_symbols/volume_up_24px.svg
    assets/material_symbols/zoom_in_24px.svg
    assets/material_symbols/zoom_out_24px.svg
)

juce_add_gui_app(Resona
  PRODUCT_NAME "Resona"
  COMPANY_NAME "Resona Audio"
  BUNDLE_ID "audio.resona.daw"
  MICROPHONE_PERMISSION_ENABLED TRUE
  MICROPHONE_PERMISSION_TEXT "Resona uses microphone access to record audio tracks."
)

juce_generate_juce_header(Resona)

add_library(resona_framework STATIC)
target_link_libraries(resona_framework PRIVATE
  juce::juce_audio_devices
  juce::juce_audio_formats
  juce::juce_audio_processors
  juce::juce_audio_utils
  juce::juce_dsp
  juce::juce_gui_extra
)
target_include_directories(resona_framework PUBLIC "${juce_SOURCE_DIR}/modules")
target_compile_definitions(resona_framework PUBLIC
  JUCE_APPLICATION_NAME_STRING="Resona"
  JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}"
  JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
  JUCE_STANDALONE_APPLICATION=1
  JUCE_REPORT_APP_USAGE=0
  JUCE_USE_CURL=0
  JUCE_WEB_BROWSER=0
  JUCE_PLUGINHOST_VST3=1
  JUCE_PLUGINHOST_AU=0
)

add_library(resona_core STATIC
  src/core/ProjectModel.cpp
  src/core/ProjectModel.h
)
target_include_directories(resona_core PUBLIC src)
target_link_libraries(resona_core PUBLIC resona_framework)
add_library(resona::core ALIAS resona_core)

add_library(resona_transport STATIC
  src/transport/TransportController.cpp
  src/transport/TransportController.h
)
target_include_directories(resona_transport PUBLIC src)
target_link_libraries(resona_transport PUBLIC resona_core)
add_library(resona::transport ALIAS resona_transport)

add_library(resona_timeline STATIC
  src/timeline/TimelineController.cpp
  src/timeline/TimelineController.h
)
target_include_directories(resona_timeline PUBLIC src)
target_link_libraries(resona_timeline PUBLIC resona_core)
add_library(resona::timeline ALIAS resona_timeline)

add_library(resona_midi_editor STATIC
  src/midi_editor/MidiEditorController.cpp
  src/midi_editor/MidiEditorController.h
)
target_include_directories(resona_midi_editor PUBLIC src)
target_link_libraries(resona_midi_editor PUBLIC resona_core)
add_library(resona::midi_editor ALIAS resona_midi_editor)

add_library(resona_mixer STATIC
  src/mixer/MixerController.cpp
  src/mixer/MixerController.h
)
target_include_directories(resona_mixer PUBLIC src)
target_link_libraries(resona_mixer PUBLIC resona_core resona_framework)
add_library(resona::mixer ALIAS resona_mixer)

add_library(resona_audio STATIC
  src/audio/AudioEngine.cpp
  src/audio/AudioEngine.h
)
target_include_directories(resona_audio PUBLIC src)
target_link_libraries(resona_audio PUBLIC resona_core resona_transport)
add_library(resona::audio ALIAS resona_audio)

add_library(resona_ui STATIC
  src/ui/chrome/ChromePresenter.cpp
  src/ui/chrome/ChromePresenter.h
  src/ui/common/Drawing.cpp
  src/ui/common/Drawing.h
  src/ui/common/MaterialIcons.cpp
  src/ui/common/MaterialIcons.h
  src/ui/timeline/TimelinePresenter.cpp
  src/ui/timeline/TimelinePresenter.h
  src/ui/midi_editor/MidiEditorPresenter.cpp
  src/ui/midi_editor/MidiEditorPresenter.h
  src/ui/inspector/InspectorPresenter.cpp
  src/ui/inspector/InspectorPresenter.h
  src/ui/mixer/MixerPresenter.cpp
  src/ui/mixer/MixerPresenter.h
  src/ui/MainComponent.cpp
  src/ui/MainComponentFactory.h
  src/ui/MainWindow.cpp
  src/ui/MainWindow.h
  src/ui/common/Theme.cpp
  src/ui/common/Theme.h
)
target_include_directories(resona_ui PUBLIC src)
target_link_libraries(resona_ui PUBLIC
  resona_material_icons
  resona_audio
  resona_timeline
  resona_midi_editor
  resona_mixer
)
add_library(resona::ui ALIAS resona_ui)

target_sources(Resona PRIVATE src/app/Main.cpp)
target_include_directories(Resona PRIVATE src)
target_link_libraries(Resona PRIVATE resona_ui)

foreach(target
    resona_framework
    resona_core
    resona_transport
    resona_timeline
    resona_midi_editor
    resona_audio
    resona_ui
    Resona)
  if(MSVC)
    target_compile_options(${target} PRIVATE /utf-8)
  endif()
  target_link_libraries(${target} PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
  )
endforeach()

install(TARGETS Resona
  RUNTIME DESTINATION . COMPONENT ResonaApp
  BUNDLE DESTINATION . COMPONENT ResonaApp
)
