cmake_minimum_required(VERSION 3.22.1)

project(HelloQuest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(FetchContent)

# Fetch OpenXR-SDK
FetchContent_Declare(OpenXR-SDK
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
        GIT_TAG release-1.0.33
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(OpenXR-SDK)

# Fetch stb
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# Expose stb headers via an INTERFACE target so subdirectories can link it
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${stb_SOURCE_DIR})

add_library(meta_quest_openxr_loader SHARED IMPORTED GLOBAL)

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set_target_properties(meta_quest_openxr_loader PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/libs/debug/arm64-v8a/libopenxr_loader.so")
elseif (${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo) #android always generates debug symbols and the gradle plugin does the stripping part
    set_target_properties(meta_quest_openxr_loader PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/libs/release/arm64-v8a/libopenxr_loader.so")
else ()
    message(FATAL_ERROR "unknown build type")
endif ()


add_subdirectory(cpp)

