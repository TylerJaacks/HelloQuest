# cmake
add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# collect sources (combined globs)
file(GLOB ALL_SRC
        "*.cpp" "*.c"
        "common/*.cpp" "common/*.c"
        "common/winsys/*.cpp" "common/winsys/*.c"
)

# platform libraries
find_library(GLESV2_LIB GLESv2)
find_library(EGL_LIBRARY EGL)

# build shared library
add_library(HelloQuest SHARED ${ALL_SRC})
target_compile_definitions(HelloQuest PRIVATE XR_USE_PLATFORM_ANDROID XR_USE_GRAPHICS_API_OPENGL_ES)

# Note: `common/util_texture.c` defines `STB_IMAGE_IMPLEMENTATION` — do not add another implementation file.
target_link_libraries(HelloQuest
        PRIVATE
        stb_headers
        android
        native_app_glue
        meta_quest_openxr_loader
        OpenXR::headers
        log
        ${GLESV2_LIB}
        ${EGL_LIBRARY}
)
