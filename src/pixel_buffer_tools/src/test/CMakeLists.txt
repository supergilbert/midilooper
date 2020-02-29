cmake_minimum_required(VERSION 3.6.2)

project(test_pbt)

set(CMAKE_C_FLAGS "-Wall -Werror -g")
set(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++11 -g")

set(APP_NAME test_pbt)

add_executable(${PROJECT_NAME}
  test_pbt.c)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../CMakeLists.txt)

pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

target_link_libraries(${PROJECT_NAME}
  ${pbt_lib_name}
  ${GLES_LIBRARIES}
  ${X11_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${FONTCONFIG_LIBRARIES}
  pthread
  m)

include_directories(${PBT_DIR}/src)
