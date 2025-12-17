#add_subdirectory(shaders)

add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(HelloQuest SHARED
        main.cpp
)

target_compile_definitions(HelloQuest PRIVATE XR_USE_PLATFORM_ANDROID
        XR_USE_GRAPHICS_API_OPENGL_ES)

find_library(EGL_LIBRARY EGL)

target_link_libraries(
        HelloQuest
        android
        glm
        native_app_glue
        meta_quest_openxr_loader
        OpenXR::headers
        log
        ${EGL_LIBRARY}
        #shaders
)
