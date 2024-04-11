# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

add_library(huawei_arengine_ndk SHARED IMPORTED)
set_target_properties(huawei_arengine_ndk PROPERTIES IMPORTED_LOCATION
        "${ARENGINE_LIBPATH}/${ANDROID_ABI}/libhuawei_arengine_ndk.so")

# Declares and names the project.

project("selfdemo")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_subdirectory(glm-1.0.1)

add_library( # Sets the name of the library.
        selfdemo

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        selfdemo.cpp)

target_include_directories(selfdemo PRIVATE glm-1.0.1/glm ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(selfdemo PRIVATE GLM_ENABLE_EXPERIMENTAL)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        selfdemo

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        android
        huawei_arengine_ndk
        GLESv3)