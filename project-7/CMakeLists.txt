cmake_minimum_required(VERSION 3.16)


project(BreadFinal LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#cpp standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#qt
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

include_directories(src)


add_executable(${PROJECT_NAME}
    src/main.cpp
    src/realtime.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp
    src/utils/uvmapper.cpp

    src/camera/Camera.cpp

    src/shapes/Cube.cpp
    src/shapes/Sphere.cpp
    src/shapes/Cone.cpp
    src/shapes/Cylinder.cpp
    src/shapes/ShapeManager.cpp
    src/shapes/Baguette.cpp
    src/shapes/Loaf.cpp

    src/rendering/ShaderManager.cpp
    src/rendering/TextureManager.cpp
    src/rendering/InstanceManager.cpp
    src/rendering/ShadowManager.cpp

    src/terrain/TerrainGenerator.cpp

    src/mainwindow.h
    src/realtime.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/uvmapper.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp

    src/camera/Camera.h

    src/shapes/Cube.h
    src/shapes/Sphere.h
    src/shapes/Cone.h
    src/shapes/Cylinder.h
    src/shapes/ShapeManager.h
    src/shapes/Baguette.h
    src/shapes/Loaf.h

    src/rendering/ShaderManager.h
    src/rendering/TextureManager.h
    src/rendering/InstanceManager.h
    src/rendering/ShadowManager.h

    src/terrain/TerrainGenerator.h
)


add_subdirectory(glm)


add_library(StaticGLEW STATIC glew/src/glew.c)
include_directories(${PROJECT_NAME} PRIVATE glew/include)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
        resources/shaders/default.frag
        resources/shaders/default.vert
        resources/shaders/shadow.frag
        resources/shaders/shadow.vert
        resources/shaders/bloom.vert
        resources/shaders/bloom_bright.frag
        resources/shaders/bloom_blur.frag
        resources/shaders/bloom_composite.frag
        resources/shaders/color_grading.frag
)

# test executables (optional - only built if explicitly requested)
# test 1: tangent/bitangent mathematical correctness
add_executable(test_tangent_bitangent
    tests/test_tangent_bitangent.cpp
    src/shapes/Cube.cpp
    src/shapes/Sphere.cpp
    src/shapes/Cylinder.cpp
    src/shapes/Cone.cpp
    src/utils/uvmapper.cpp
)

target_link_libraries(test_tangent_bitangent PRIVATE
    Qt::Core
)

# test 2: texture manager functionality
add_executable(test_texture_manager
    tests/test_texture_manager.cpp
    src/rendering/TextureManager.cpp
)

target_link_libraries(test_texture_manager PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    StaticGLEW
)

# enable ctest
enable_testing()
add_test(NAME TangentBitangentTest COMMAND test_tangent_bitangent)
add_test(NAME TextureManagerTest COMMAND test_texture_manager)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
  target_link_libraries(test_texture_manager PRIVATE
    opengl32
    glu32
  )
endif()

#flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()

#flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
