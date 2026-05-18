#include "Engine/Engine.h"
#include <iostream>

int main(int argc, char** argv) {
    Engine engine;

    engine.init(argc, argv);
    engine.setWindowParams(800, 600, "Silnik 3D - FreeGLUT");
    engine.setGraphicsParams(60, true, true);
    engine.setClearColor(0.12f, 0.16f, 0.22f, 1.0f);
    engine.setProjection(ProjectionType::PERSPECTIVE);

    std::cout << "=== Silnik 3D ===\n";
    std::cout << "Kamera:      WASD = ruch, Q/E = góra/dół\n";
    std::cout << "             LPM + mysz = obrót, scroll = zoom\n";
    std::cout << "Rendering:   M = siatka,  L = oswietlenie, G = cieniowanie\n";
    std::cout << "Rzutowanie:  P = perspektywa, O = ortogonalne\n";
    std::cout << "FPS:         + / -\n";
    std::cout << "Wyjscie:     ESC\n";

    engine.run();
    return 0;
}
