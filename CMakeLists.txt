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

set(DEMO_SOURCES
    Demo/Room.cpp         # Osoba 1
    Demo/Obstacles.cpp    # Osoba 2
    Demo/Targets.cpp      # Osoba 3
    Demo/Gameplay.cpp     # Osoba 4
)

set(SOURCES
    main.cpp
    ${ENGINE_SOURCES}
    ${DEMO_SOURCES}
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
    # If MSYS2 UCRT64 is present, prefer its toolchain (ucrt64).
    if(EXISTS "/ucrt64")
        set(UCRT_DIR "/ucrt64")
        include_directories("${UCRT_DIR}/include")
        link_directories("${UCRT_DIR}/lib")
        find_library(FREEGLUT_LIB NAMES freeglut libfreeglut PATHS "${UCRT_DIR}/lib")
        find_library(GLU_LIB NAMES glu32 libGLU32 PATHS "${UCRT_DIR}/lib")
        target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL ${FREEGLUT_LIB} ${GLU_LIB})
        if(FREEGLUT_LIB)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${UCRT_DIR}/bin/freeglut.dll"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    else()
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
    endif()
else()
    find_package(GLUT REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLUT::GLUT)
endif()

# Kopiowanie folderu assets/ (BMP textures) obok pliku wykonywalnego.
# Uzywamy osobnego custom target z ALL - dzieki temu komenda kopiowania
# uruchamia sie ZAWSZE przy `cmake --build`, niezaleznie od tego czy
# Silnik3D.exe byl przebudowywany. Dorzucenie nowego pliku BMP do
# assets/textures/ wystarczy potwierdzic przez kolejny `cmake --build build`.
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Kopiowanie assets/ do katalogu z exe"
    )
    add_dependencies(copy_assets ${PROJECT_NAME})
endif()
