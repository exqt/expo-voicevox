cmake_minimum_required(VERSION 3.18)
project(voicevox_jni)

# Prebuilt libvoicevox_core.so
add_library(voicevox_core SHARED IMPORTED)
set_target_properties(voicevox_core PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libvoicevox_core.so
    IMPORTED_NO_SONAME TRUE
)

# Prebuilt libvoicevox_onnxruntime.so
add_library(voicevox_onnxruntime SHARED IMPORTED)
set_target_properties(voicevox_onnxruntime PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libvoicevox_onnxruntime.so
    IMPORTED_NO_SONAME TRUE
)

# JNI bridge library
add_library(voicevox_jni SHARED voicevox_jni.cpp)

target_include_directories(voicevox_jni PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(voicevox_jni
    voicevox_core
    voicevox_onnxruntime
    log
)
