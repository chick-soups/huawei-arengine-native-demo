

cmake_minimum_required(VERSION 3.4.1)
add_library(huawei_arengine_ndk SHARED IMPORTED)
set_target_properties(huawei_arengine_ndk PROPERTIES IMPORTED_LOCATION
        "${ARENGINE_LIBPATH}/${ANDROID_ABI}/libhuawei_arengine_ndk.so")

# This is the main app library.
add_library(worldAr_native SHARED
        src/main/cpp/rendering/world_background_renderer.cpp
        src/main/cpp/world_ar_application.cpp
        src/main/cpp/jni_interface.cpp
        src/main/cpp/rendering/world_point_cloud_renderer.cpp
        src/main/cpp/rendering/world_render_manager.cpp
        src/main/cpp/rendering/world_object_renderer.cpp
        src/main/cpp/rendering/world_plane_renderer.cpp
        src/main/cpp/utils/util.cpp)

target_include_directories(worldAr_native PRIVATE
        src/main/cpp
        ${ANDROID_NDK}/sources/third_party/vulkan/src/libs/glm)

target_link_libraries(worldAr_native
        android
        log
        GLESv2
        huawei_arengine_ndk
        mediandk
        )

