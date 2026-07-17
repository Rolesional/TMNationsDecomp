cmake_minimum_required(VERSION 3.22.1)
project(tmnf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(game-activity REQUIRED CONFIG)

add_subdirectory(
    "${CMAKE_CURRENT_LIST_DIR}/../../../../core"
    "${CMAKE_CURRENT_BINARY_DIR}/tmnf-core"
)

add_library(tmnf SHARED
    content/reference_probe.cpp
    native_main.cpp
    platform/android/audio_output.cpp
    platform/android/egl_renderer.cpp
    platform/android/input_adapter.cpp
)

target_include_directories(tmnf PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

target_compile_options(tmnf PRIVATE
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
)

target_link_options(tmnf PRIVATE "-Wl,--gc-sections")

target_link_libraries(tmnf PRIVATE
    tmnf_core
    "-Wl,--whole-archive"
    game-activity::game-activity_static
    "-Wl,--no-whole-archive"
    android
    aaudio
    EGL
    GLESv3
    log
)
