cmake_minimum_required(VERSION 3.10)
project(Silnik3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS (Homebrew)
if(APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
endif()

# Windows: vcpkg
if(WIN32)
    set(VCPKG_DIR "C:/vcpkg/installed/x64-windows")
    include_directories("${VCPKG_DIR}/include")
    link_directories("${VCPKG_DIR}/lib")
endif()

find_package(OpenGL REQUIRED)

set(ENGINE_SOURCES
    Engine/Engine.cpp
    Engine/Math3D.cpp
    Engine/SceneNode.cpp
    Engine/Camera.cpp
    Engine/Light.cpp
    Engine/PrimitiveNode.cpp
    Engine/Texture.cpp
)

set(SOURCES
    main.cpp
    ${ENGINE_SOURCES}
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Ścieżki include:
#   ${CMAKE_SOURCE_DIR}        -> main.cpp używa #include "Engine/Engine.h"
#   ${CMAKE_SOURCE_DIR}/Engine -> pliki silnika używają #include "Math3D.h" etc.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Engine
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL" glut)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenGL::GL
        "${VCPKG_DIR}/lib/freeglut.lib"
        "${VCPKG_DIR}/lib/GlU32.Lib"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VCPKG_DIR}/bin/freeglut.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    find_package(GLUT REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLUT::GLUT)
endif()
