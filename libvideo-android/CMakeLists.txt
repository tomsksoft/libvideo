cmake_minimum_required(VERSION 3.16.0)

project(libvideo_android VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(libvideo/conan_build_${ANDROID_ABI}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(${PROJECT_NAME}_SRC libvideo/src/main/libvideo_cpp/native-libvideo.cpp)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC})

target_link_libraries(${PROJECT_NAME} PUBLIC CONAN_PKG::libvideo log)
target_include_directories(${PROJECT_NAME} PUBLIC ../Include)