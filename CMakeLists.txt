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
)

add_executable(${PROJECT_NAME} ${SOURCES})

# 4. Linkowanie bibliotek (zależnie od systemu)
if(APPLE)
    # Dla wersji Cocoa linkujemy natywny framework OpenGL i skompilowany freeglut
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL" glut)
else()
    # Na Windows/Linux używamy standardowego wyszukiwania
    find_package(GLUT REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLUT::GLUT)
endif()