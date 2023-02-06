cmake_minimum_required(VERSION 3.16.0)

project(libvideo_test VERSION 1.0.0 LANGUAGES CXX)

if (${LIBVIDEO_USE_CLANGTIDY})
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)
endif()

add_executable(${PROJECT_NAME} LibvideoExample.cpp TestWindow.cpp TestWindow.h)

add_executable(${PROJECT_NAME}_qa TestLibvideo.cpp TestLibvideoFilter.cpp)

include(GoogleTest)

gtest_discover_tests(${PROJECT_NAME}_qa)

set(WIN_LIBS ws2_32 d3d11.lib)

if (TARGET CONAN_PKG::libvideo)
    target_link_libraries(${PROJECT_NAME} CONAN_PKG::libvideo ${WIN_LIBS})
    target_link_libraries(${PROJECT_NAME}_qa CONAN_PKG::libvideo ${WIN_LIBS})
else()
    # LibvideoExample.cpp
    target_link_libraries(${PROJECT_NAME} PRIVATE libvideo ${CONAN_TARGETS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${WIN_LIBS})

    # TestLibVideo.cpp TestLibVideoFilter.cpp
    target_link_libraries(${PROJECT_NAME}_qa PRIVATE libvideo ${CONAN_TARGETS})
    target_link_libraries(${PROJECT_NAME}_qa PRIVATE ${WIN_LIBS})
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    target_compile_options(${PROJECT_NAME}_qa PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(${PROJECT_NAME}_qa PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if (${LIBVIDEO_USE_CLANGTIDY})
    if(MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_GLOBAL_RunCodeAnalysis true
            VS_GLOBAL_EnableClangTidyCodeAnalysis true
        )
        set_target_properties(${PROJECT_NAME}_qa PROPERTIES
            VS_GLOBAL_RunCodeAnalysis true
            VS_GLOBAL_EnableClangTidyCodeAnalysis true
        )
    endif()
endif()