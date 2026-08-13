cmake_minimum_required(VERSION 3.21)

project(SmartAccessWelcome
    VERSION 0.1.0
    DESCRIPTION "Smart access control terminal with a real-time web welcome flow"
    LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SAW_BUILD_APPS "Build desktop and server applications" ON)
option(SAW_BUILD_TESTS "Build automated tests" ON)
option(SAW_ENABLE_LEGACY_OPENCV "Allow the legacy OpenCV 3.4 migration environment" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(SAW_BUILD_APPS)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network Sql Test WebSockets TextToSpeech)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network Sql Test WebSockets TextToSpeech)
    find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs videoio objdetect face)

    if(OpenCV_VERSION VERSION_LESS 4.0 AND NOT SAW_ENABLE_LEGACY_OPENCV)
        message(FATAL_ERROR "OpenCV 4.x is required. Set SAW_ENABLE_LEGACY_OPENCV=ON only during migration.")
    endif()

    add_subdirectory(modules)
    add_subdirectory(apps)
endif()

include(CTest)
if(BUILD_TESTING AND SAW_BUILD_TESTS)
    add_subdirectory(tests)
endif()

message(STATUS "Smart Access Welcome ${PROJECT_VERSION}")
message(STATUS "Build applications: ${SAW_BUILD_APPS}")
message(STATUS "Build tests: ${SAW_BUILD_TESTS}")

