cmake_minimum_required(VERSION 3.22.1)
include(FetchContent)

project(HelloQuest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

add_subdirectory(meta_quest_openxr_loader)

FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        )
FetchContent_MakeAvailable(glm)

FetchContent_Declare(OpenXR-SDK
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
        GIT_TAG release-1.0.33 #must match the meta quest loader OpenXR version
        #https://developer.oculus.com/downloads/package/oculus-openxr-mobile-sdk#current-openxr-version
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(OpenXR-SDK)

add_subdirectory(cpp)
