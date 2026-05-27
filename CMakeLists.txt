cmake_minimum_required(VERSION 3.10)
project(Silnik3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Konfiguracja ścieżek do Homebrew na macOS
if(APPLE)
    # Ścieżki dla procesorów Apple Silicon (M1/M2/M3)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
    
    # Ścieżki dla starszych procesorów Intel
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
endif()

# 2. Szukanie OpenGL
find_package(OpenGL REQUIRED)

# 3. Zdefiniowanie plików źródłowych
set(SOURCES 
    main.cpp 
    Engine.cpp
    Math3D.cpp
    SceneNode.cpp
    Camera.cpp
    Light.cpp
    PrimitiveNode.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# 4. Linkowanie bibliotek (zależnie od systemu)
if(APPLE)
    # Dla wersji Cocoa linkujemy natywny framework OpenGL i skompilowany freeglut
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL" glut)
elseif(WIN32)
    # Na Windows preferujemy statyczne biblioteki, żeby EXE nie wymagał DLL
    set(GLUT_USE_STATIC_LIBS ON)

    find_package(GLUT REQUIRED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FREEGLUT_STATIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-static -static-libgcc -static-libstdc++")
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLUT::GLUT)

    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(runtime_dll IN ITEMS libfreeglut.dll libwinpthread-1.dll libgcc_s_seh-1.dll libstdc++-6.dll)
        if(EXISTS "${MINGW_BIN_DIR}/${runtime_dll}")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/${runtime_dll}"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endforeach()
else()
    # Na Windows/Linux używamy standardowego wyszukiwania
    find_package(GLUT REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLUT::GLUT)
endif()