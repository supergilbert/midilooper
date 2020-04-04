cmake_minimum_required(VERSION 3.6.2)

if("${PBT_DIR}" STREQUAL "")
  set(PBT_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()

set(pbt_default_font "pbt_default_font")
set(PBT_DEFAULT_FONT_PATH "${PBT_DIR}/src/externals/DejaVuSansMono.ttf")

set(PBT_DEFAULT_FONT_HDR "${CMAKE_BINARY_DIR}/${pbt_default_font}.h")
set(PBT_DEFAULT_FONT_SRC "${CMAKE_BINARY_DIR}/${pbt_default_font}.c")
set(PBT_DEFAULT_FONT_LDS "${CMAKE_BINARY_DIR}/${pbt_default_font}.lds")
set(PBT_DEFAULT_FONT_DATA_PATH "${CMAKE_BINARY_DIR}/${pbt_default_font}_data")
set(PBT_DEFAULT_FONT_DATA_SRC "${PBT_DEFAULT_FONT_DATA_PATH}.c")
set(PBT_DEFAULT_FONT_DATA_OBJ "${PBT_DEFAULT_FONT_DATA_PATH}.o")
set(PBT_DEFAULT_FONT_DATA_LDS_OBJ "${PBT_DEFAULT_FONT_DATA_PATH}_lds.o")

set(PBT_DEFAULT_FONT_SRCS
  "${PBT_DEFAULT_FONT_HDR}"
  "${PBT_DEFAULT_FONT_SRC}"
  "${PBT_DEFAULT_FONT_LDS}")

add_custom_command(OUTPUT ${PBT_DEFAULT_FONT_SRCS}
  COMMAND ${PBT_DIR}/src/data_tools/gen_archive_src.sh ${pbt_default_font} ${PBT_DEFAULT_FONT_DATA_OBJ}
  DEPENDS ${PBT_DIR}/src/data_tools/gen_archive_src.sh)

set(PBT_OBJCOPY "objcopy")
set(PBT_LINKER "ld")

add_custom_command(OUTPUT ${PBT_DEFAULT_FONT_DATA_OBJ}
  COMMAND echo > ${PBT_DEFAULT_FONT_DATA_SRC}
  COMMAND ${CMAKE_C_COMPILER} -c ${PBT_DEFAULT_FONT_DATA_SRC}
  COMMAND ${PBT_OBJCOPY} --add-section .${pbt_default_font}=${PBT_DEFAULT_FONT_PATH} ${PBT_DEFAULT_FONT_DATA_OBJ}
  DEPENDS ${PBT_DEFAULT_FONT_PATH})

add_custom_command(OUTPUT ${PBT_DEFAULT_FONT_DATA_LDS_OBJ}
  COMMAND ${PBT_LINKER} -r -o ${PBT_DEFAULT_FONT_DATA_LDS_OBJ} ${PBT_DEFAULT_FONT_LDS}
  DEPENDS ${PBT_DEFAULT_FONT_LDS} ${PBT_DEFAULT_FONT_DATA_OBJ})

add_library(${pbt_default_font}
  EXCLUDE_FROM_ALL
  ${PBT_DEFAULT_FONT_SRC}
  ${PBT_DEFAULT_FONT_DATA_LDS_OBJ})

set(pbt_source_files
  ${PBT_DIR}/src/pbt_draw.c
  ${PBT_DIR}/src/pbt_draw_font.c
  ${PBT_DIR}/src/pbt_event_handler.c
  ${PBT_DIR}/src/pbt_font.c
  ${PBT_DIR}/src/pbt_gadget.c
  ${PBT_DIR}/src/pbt_gadget_window.c
  ${PBT_DIR}/src/pbt_pixel_buffer.c
  ${PBT_DIR}/src/pbt_tools.c
  ${PBT_DIR}/src/pbt_window_gadget.c
  ${PBT_DIR}/src/wbe_gl.c
  ${PBT_DIR}/src/wbe_glfw.c
  ${PBT_DIR}/src/wbe_pbw.c)

find_package(PkgConfig)

set(pbt_lib_name "pbt")

add_library(${pbt_lib_name} ${pbt_source_files})

pkg_check_modules(FREETYPE2 REQUIRED freetype2)
pkg_check_modules(GLES      REQUIRED glesv2)
pkg_check_modules(GLFW      REQUIRED glfw3)

target_include_directories(${pbt_lib_name}
  PUBLIC ${FREETYPE2_INCLUDE_DIRS}
  PUBLIC ${PBT_DIR}/src)
