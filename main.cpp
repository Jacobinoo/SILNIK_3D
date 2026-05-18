#include "Engine/Engine.h"
#include "Demo/ShootingGallery.h"
#include <iostream>

int main(int argc, char** argv) {
    Engine engine;

    engine.init(argc, argv);
    engine.setWindowParams(1024, 768, "Strzelnica 3D - Silnik FreeGLUT");
    // 60 FPS, depth buffer, double buffering
    engine.setGraphicsParams(60, true, true);
    engine.setClearColor(0.08f, 0.10f, 0.16f, 1.0f);
    engine.setProjection(ProjectionType::PERSPECTIVE);

    // Gra musi byc skonstruowana PO setGraphicsParams (potrzeba kontekstu GL
    // do generowania tekstur proceduralnych).
    ShootingGallery game(engine);

    // Rejestracja callbackow gry w silniku
    engine.setUpdateCallback([&](float dt) { game.onUpdate(dt); });
    engine.setShootCallback ([&]()         { game.onShoot();   });
    engine.setHUDCallback   ([&]()         { game.onHUD();     });
    engine.setResetCallback ([&]()         { game.onReset();   });

    std::cout << "=== STRZELNICA 3D ===\n";
    std::cout << "Cel: trafic w pomaranczowy cel zanim wygasnie.\n";
    std::cout << "Sterowanie:\n";
    std::cout << "  LPM + mysz   = celowanie (rozglad)\n";
    std::cout << "  SPACJA       = strzal (raycast)\n";
    std::cout << "  R            = restart (po koncu gry)\n";
    std::cout << "  L / G / M    = lighting / shading / wireframe\n";
    std::cout << "  P / O        = perspektywa / ortogonalne\n";
    std::cout << "  + / -        = zmiana FPS\n";
    std::cout << "  ESC          = wyjscie\n";

    engine.run();
    return 0;
}
