cmake_minimum_required(VERSION 3.10)
project(raviz C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
include(FetchContent)

find_package(PkgConfig REQUIRED)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PULSE libpulse-simple)
    pkg_check_modules(FFTW fftw3)
    pkg_check_modules(GLFW glfw3)
endif()

# Fetch CGLM if not found
if (NOT CGLM_FOUND)
    message(STATUS "cglm not found via pkg-config, fetching...")
    FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG        v0.9.1
    )
    FetchContent_MakeAvailable(cglm)
endif()

# Fetch GLEW if not found (Skipped: We will use local gl_loader)
# if (NOT GLEW_FOUND)
# ...
# endif()


# Fallback logic
if (NOT PULSE_FOUND)
    message(WARNING "PulseAudio not found. Using local stubs for compilation.")
    set(PULSE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/external/include")
    set(USE_STUBS TRUE)
endif()

if (NOT FFTW_FOUND)
    message(WARNING "FFTW3 not found. Using local stubs for compilation.")
    set(FFTW_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/external/include")
    set(USE_STUBS TRUE)
endif()

if (NOT GLFW_FOUND)
    # Try find_package config mode
    find_package(glfw3 CONFIG)
endif()

if (NOT GLFW_FOUND)
    message(FATAL_ERROR "GLFW3 not found. Please install libglfw3-dev.")
endif()

# If GLEW still not found
if (NOT GLEW_FOUND AND NOT TARGET libglew_static)
     # Just warn for now, might link error
     message(WARNING "GLEW not found. Linking might fail.")
endif()

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

include_directories(include src)
include_directories(external/include)
include_directories(${PULSE_INCLUDE_DIRS})
include_directories(${FFTW_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${CGLM_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.c
    src/audio/audio.c
    src/fft/fft.c
    src/render/render.c
    src/render/gl_loader.c
    src/utils/config.c
    external/src/toml.c
)

if (USE_STUBS)
    list(APPEND SOURCES external/src/stubs.c)
    add_definitions(-DUSE_STUBS)
endif()

add_executable(raviz ${SOURCES})

# Link libraries
if (PULSE_FOUND)
    target_link_libraries(raviz ${PULSE_LIBRARIES})
endif()

if (FFTW_FOUND)
    target_link_libraries(raviz ${FFTW_LIBRARIES})
endif()

target_link_libraries(raviz ${GLFW_LIBRARIES} OpenGL::GL Threads::Threads m)

if (CGLM_FOUND)
    target_link_libraries(raviz ${CGLM_LIBRARIES})
else()
    target_link_libraries(raviz cglm)
endif()

# GLEW removed. Using local loader.