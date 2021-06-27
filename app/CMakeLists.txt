# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

file(GLOB CPP_FILES "src/main/cpp/*.cpp", "src/main/cpp/android_player/audio_player/*.cpp", "src/main/cpp/android_player/video_player/*.cpp")

include_directories("src/main/cpp/FFmpeg/")
include_directories("src/main/cpp/")
include_directories("src/main/cpp/iconv/")
include_directories("src/main/cpp/android_player/video_player/")
include_directories("src/main/cpp/android_player/audio_player/")



add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${CPP_FILES})

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
        native-lib
        android
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        OpenSLES
        GLESv2
        EGL
        z
#        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libcharset.so
#        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libiconv.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libfdk-aac.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libmp3lame.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libx264.x.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavformat.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavfilter.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavcodec.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavutil.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libswresample.so
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libswscale.so
        )
