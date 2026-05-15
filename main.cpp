#include "Engine.h"

int main(int argc, char** argv) {
    Engine engine;

    engine.init(argc, argv);
    engine.setWindowParams(800, 600, "Moj Silnik 3D - FreeGLUT");
    // Ustawienia: 60 FPS, bufor głębi włączony, podwójne buforowanie włączone
    engine.setGraphicsParams(60, true, true); 
    engine.setClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    engine.setProjection(ProjectionType::PERSPECTIVE);
    
    engine.run();

    return 0;
}