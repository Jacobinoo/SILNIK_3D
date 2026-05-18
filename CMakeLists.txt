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

# Windows: vcpkg (ręczne wskazanie ścieżek jeśli nie użyto toolchain)
if(WIN32)
    set(VCPKG_DIR "C:/vcpkg/installed/x64-windows")
    include_directories("${VCPKG_DIR}/include")
    link_directories("${VCPKG_DIR}/lib")
endif()

find_package(OpenGL REQUIRED)

set(SOURCES
    main.cpp
    Engine.cpp
    Math3D.cpp
    SceneNode.cpp
    Camera.cpp
    Light.cpp
    PrimitiveNode.cpp
    Texture.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL" glut)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenGL::GL
        "${VCPKG_DIR}/lib/freeglut.lib"
        "${VCPKG_DIR}/lib/GlU32.Lib"
    )
    # Skopiuj freeglut.dll obok pliku wykonywalnego
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VCPKG_DIR}/bin/freeglut.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    find_package(GLUT REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLUT::GLUT)
endif()
