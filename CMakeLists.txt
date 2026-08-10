cmake_minimum_required(VERSION 3.24)

project(8Beat LANGUAGES CXX)

set(EIGHTBEAT_CORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Core"
    CACHE PATH "Path to the Core checkout")
set(EIGHTBEAT_AUDIOLIBSWITCHER_APPLAUDIO_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../AudioLibSwitcher_applaudio"
    CACHE PATH "Path to the AudioLibSwitcher_applaudio checkout")

if(NOT TARGET Core::Core)
  add_subdirectory("${EIGHTBEAT_CORE_SOURCE_DIR}"
                   "${CMAKE_CURRENT_BINARY_DIR}/Core"
                   EXCLUDE_FROM_ALL)
endif()
if(NOT TARGET AudioLibSwitcher_applaudio::AudioLibSwitcher_applaudio)
  add_subdirectory("${EIGHTBEAT_AUDIOLIBSWITCHER_APPLAUDIO_SOURCE_DIR}"
                   "${CMAKE_CURRENT_BINARY_DIR}/AudioLibSwitcher_applaudio"
                   EXCLUDE_FROM_ALL)
endif()

add_library(8Beat INTERFACE)
add_library(8Beat::8Beat ALIAS 8Beat)

target_compile_features(8Beat INTERFACE cxx_std_20)
target_include_directories(8Beat INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_definitions(8Beat INTERFACE USE_APPLAUDIO)
target_link_libraries(8Beat INTERFACE
  Core::Core
  AudioLibSwitcher_applaudio::AudioLibSwitcher_applaudio
)

