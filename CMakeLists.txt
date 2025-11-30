cmake_minimum_required(VERSION 3.16)

project(BreadFinal)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Xml Widgets)

# Collect all source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Xml
    Qt6::Widgets
)

# Platform-specific OpenGL linking
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE GL)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)
