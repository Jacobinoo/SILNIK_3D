cmake_minimum_required(VERSION 3.10)
project(Silnik3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Szukamy pakietów. Dzięki integracji vcpkg (toolchain) Windows 
# bez problemu sam znajdzie te paczki tam, gdzie zostały zainstalowane.
find_package(OpenGL REQUIRED)

if(WIN32)
    # Szukamy freeglut pobranego przez vcpkg
    find_package(freeglut CONFIG REQUIRED)
endif()

# macOS (Homebrew)
if(APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
endif()

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
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Engine
)

# LINKOWANIE Z BIBLIOTEKAMI
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
        # WŁAŚCIWE PODEJŚCIE DLA VCPKG
        # Łączymy projekt z celem freeglut::freeglut. CMake załatwia całą resztę.
        target_link_libraries(${PROJECT_NAME} PRIVATE
            OpenGL::GL
            FreeGLUT::freeglut
        )
    endif()
else()
    find_package(GLUT REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLUT::GLUT)
endif()

# Kopiowanie folderu assets/ (BMP textures) obok pliku wykonywalnego.
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Kopiowanie assets/ do katalogu z exe"
    )
    add_dependencies(copy_assets ${PROJECT_NAME})
endif()