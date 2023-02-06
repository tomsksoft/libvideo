cmake_minimum_required(VERSION 3.16.0)

project(libvideo VERSION 1.0.0 LANGUAGES CXX)

if (${LIBVIDEO_USE_CLANGTIDY})
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

if (WIN32)
    # Compile shaders. Ensure that fxc.exe is inside in PATH environment variable
    add_custom_command(
      OUTPUT src/Shaders/VertexShader.h
      PRE_BUILD
      COMMAND fxc.exe ${CMAKE_SOURCE_DIR}/src/Shaders/VertexShader.hlsl /Fh /E vs_main /T vs_4_0
      MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/Shaders/VertexShader.hlsl
      VERBATIM)

    add_custom_command(
      OUTPUT src/Shaders/PixelShader.h
      PRE_BUILD
      COMMAND fxc.exe ${CMAKE_SOURCE_DIR}/src/Shaders/PixelShader.hlsl /Fh /E ps_main /T ps_4_0
      MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/Shaders/PixelShader.hlsl
      VERBATIM)

    set(SHADERS_SRC src/Shaders/VertexShader.h src/Shaders/PixelShader.h)
endif()

set(${PROJECT_NAME}_SRC src/GeneralUtilities.cpp
                        src/SWEncoder.h src/SWEncoder.cpp
                        src/SWDecoder.h src/SWDecoder.cpp
                        src/Filters/IFilter.h
                        src/Filters/ScaleFilter.h src/Filters/ScaleFilter.cpp
                        src/Filters/CropFilter.h src/Filters/CropFilter.cpp
                        src/Logs.h src/Logs.cpp
                        src/FormatConvertors/RGBAtoYUV.h src/FormatConvertors/RGBAtoYUV.cpp
                        src/FormatConvertors/YUVtoNV12.h src/FormatConvertors/YUVtoNV12.cpp
                        src/FormatConvertors/YUVtoRGBA.h src/FormatConvertors/YUVtoRGBA.cpp
                        src/libvideo.cpp
                        )

set(${PROJECT_NAME}_IFACE Include/DecoderParams.h Include/EncoderParams.h Include/FilterParams.h 
                          Include/GeneralUtilities.h Include/IEncoder.h Include/IDecoder.h Include/libvideo.h)

if (WIN32)
    set(${PROJECT_NAME}_PLATFORM_SRC src/WinHWEncoder.h src/WinHWEncoder.cpp
                                     src/WinHWDecoder.h src/WinHWDecoder.cpp
                                     src/WindowsManager/WindowOutputManagerSW.h src/WindowsManager/WindowOutputManagerSW.cpp
                                     src/WindowsManager/WindowOutputManagerHW.h src/WindowsManager/WindowOutputManagerHW.cpp)

    set(${PROJECT_NAME}_PLATFORM_INCLUDE_DIRS src/WindowsManager src/Shaders)
endif()

if (ANDROID)
    set(${PROJECT_NAME}_PLATFORM_SRC src/AndroidHWDecoder.h src/AndroidHWDecoder.cpp)
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC} ${${PROJECT_NAME}_PLATFORM_SRC} ${${PROJECT_NAME}_IFACE} ${SHADERS_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::ffmpeg)
target_include_directories(${PROJECT_NAME} PRIVATE src src/Filters src/FormatConvertors ${${PROJECT_NAME}_PLATFORM_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC Include)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()

if (WIN32)
    add_subdirectory(test_package)
endif()

if (${LIBVIDEO_USE_CLANGTIDY})
    if(MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_GLOBAL_RunCodeAnalysis true
            VS_GLOBAL_EnableClangTidyCodeAnalysis true
        )
    endif()
endif()