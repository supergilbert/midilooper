cmake_minimum_required(VERSION 3.6.2)

project(midilooper)

set(PBT_DIR "${CMAKE_SOURCE_DIR}/src/pixel_buffer_tools")
set(MSQ_DIR "${CMAKE_SOURCE_DIR}/src/midiseq")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/src/externals/imgui")
set(MIDILOOPER_DIR "${CMAKE_SOURCE_DIR}/src/midilooper")

set(IMGUI_SRC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/examples/imgui_impl_glfw.cpp)

###

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
  message("Debug build as default")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

# TODO add a debug flag
set(CMAKE_C_FLAGS_DEBUG "-Wall -Werror -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Werror -std=c++11 -g")

set(CMAKE_C_FLAGS_RELEASE "-g0 -s")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++11 -g0 -s")

set(MIDISEQ_SRC
  ${MSQ_DIR}/midi/midifile.c
  ${MSQ_DIR}/seqtool/seqtool.c
  ${MSQ_DIR}/tool/tool.c
  ${MSQ_DIR}/asound/aseq.c
  ${MSQ_DIR}/asound/aseq_tool.c
  ${MSQ_DIR}/clock/clock.c
  ${MSQ_DIR}/jack/jack_backend.c
  ${MSQ_DIR}/loop_engine/engine.c
  ${MSQ_DIR}/loop_engine/track_ctx.c
  ${MSQ_DIR}/loop_engine/engine_binding.c
  ${MSQ_DIR}/loop_engine/output_req.c
  ${MSQ_DIR}/loop_engine/midi_ring_buffer.c
  ${MSQ_DIR}/loop_engine/engine_nanosleep.c
  ${MSQ_DIR}/loop_engine/engine_jack.c
  ${MSQ_DIR}/loop_engine/engine_midisave.c
  ${MSQ_DIR}/debug_tool/debug_tool.c
  ${MSQ_DIR}/midi/midifile_get_varlen.c
  ${MSQ_DIR}/midi/midifile_tool.c
  ${MSQ_DIR}/midi/midi_meta_ev.c
  ${MSQ_DIR}/midi/midi_channel_ev.c
  ${MSQ_DIR}/midi/midi_tool.c
  ${MSQ_DIR}/seqtool/ev_iterator.c)

add_executable(${PROJECT_NAME}
  ${MIDILOOPER_DIR}/msq_gui.c
  ${MIDILOOPER_DIR}/msq_track_editor.c
  ${MIDILOOPER_DIR}/msq_imgui.cxx
  ${MIDILOOPER_DIR}/msq_main_gui.cxx
  ${MIDISEQ_SRC}
  ${IMGUI_SRC})

include(${PBT_DIR}/CMakeLists.txt)

find_package(PkgConfig)

# pkg_check_modules(GLES      REQUIRED glesv2)
# pkg_check_modules(GLFW      REQUIRED glfw3)
# pkg_check_modules(FREETYPE2 REQUIRED freetype2)
pkg_check_modules(ALSA      REQUIRED alsa)
pkg_check_modules(JACK      REQUIRED jack)
pkg_check_modules(LIBCONFIG REQUIRED libconfig)

target_link_libraries(${PROJECT_NAME}
  ${wbe_lib_name}
  ${pbt_lib_name}
  ${pbt_default_font}
  ${GLES_LIBRARIES}
  ${X11_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${ALSA_LIBRARIES}
  ${JACK_LIBRARIES}
  ${FREETYPE2_LIBRARIES}
  ${LIBCONFIG_LIBRARIES}
  pthread)

include_directories(${CMAKE_BINARY_DIR}
  ${GLFW_INCLUDE_DIRS}
  ${FREETYPE2_INCLUDE_DIRS}
  ${PBT_DIR}/src
  ${IMGUI_DIR}
  ${MSQ_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  include(ProcessorCount)
  ProcessorCount(PROC_COUNT)
  math(EXPR JOBS_COUNT "${PROC_COUNT} + 1")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/debian ${CMAKE_BINARY_DIR}/midilooper.1
    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/debian ${CMAKE_BINARY_DIR}/debian
    COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/src/midilooper.1 ${CMAKE_BINARY_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/src/debian)

  add_custom_target(debsrc
    DEPENDS ${CMAKE_BINARY_DIR}/debian ${CMAKE_BINARY_DIR}/midilooper.1)

  add_custom_target(debpkg
    COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/src/debian/changelog ${CMAKE_BINARY_DIR}/debian/changelog
    COMMAND ${CMAKE_SOURCE_DIR}/src/update_debchangelog.sh ${CMAKE_SOURCE_DIR}/src
    COMMAND debuild -nc -j${JOBS_COUNT} -b -us -uc
    DEPENDS
    ${CMAKE_BINARY_DIR}/debian
    ${CMAKE_SOURCE_DIR}/src/midilooper.1
    ${CMAKE_SOURCE_DIR}/src/update_debchangelog.sh)
endif()
