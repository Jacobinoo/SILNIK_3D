#include "Engine.h"
#include <iostream>

int main(int argc, char** argv) {
    Engine engine;

    engine.init(argc, argv);
    engine.setWindowParams(800, 600, "Moj Silnik 3D - FreeGLUT");
    // Ustawienia: 60 FPS, bufor głębi włączony, podwójne buforowanie włączone
    engine.setGraphicsParams(60, true, true); 
    engine.setClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    engine.setProjection(ProjectionType::PERSPECTIVE);
    // Informacje o sterowaniu
    std::cout << "Sterowanie: WASD - przesuwanie celu kamery, Q/E - góra/dół\n";
    std::cout << "Mysz: lewy-drag obraca, scroll zoom. Klawisze 1=Cube, 2=Cylinder, +/- zmiana FPS, p/o zmiana rzutowania, ESC wyjscie\n";

    engine.run();

    return 0;
}