#include "Engine/Engine.h"
#include "Demo/ShootingGallery.h"
#include <iostream>

int main(int argc, char** argv) {
    Engine engine;

    engine.init(argc, argv);
    engine.setWindowParams(1024, 768, "Strzelnica 3D - Silnik FreeGLUT");
    engine.setGraphicsParams(60, true, true);
    engine.setClearColor(0.05f, 0.07f, 0.12f, 1.0f);
    engine.setProjection(ProjectionType::PERSPECTIVE);

    ShootingGallery game(engine);

    engine.setUpdateCallback([&](float dt) { game.onUpdate(dt); });
    engine.setShootCallback ([&]()         { game.onShoot();   });
    engine.setHUDCallback   ([&]()         { game.onHUD();     });
    engine.setResetCallback ([&]()         { game.onReset();   });

    std::cout << "=== STRZELNICA 3D ===\n";
    std::cout << "Cel: trafic w pomaranczowa sfere zanim wygasnie (7 sek).\n";
    std::cout << "Bonus za serie: co 5 trafien +1 zycie (max 9).\n\n";
    std::cout << "Sterowanie:\n";
    std::cout << "  WASD       = ruch gracza\n";
    std::cout << "  mysz       = celowanie (free look)\n";
    std::cout << "  SPACJA     = strzal (raycast)\n";
    std::cout << "  R          = restart po Game Over\n";
    std::cout << "  L / G / M  = lighting / shading / wireframe\n";
    std::cout << "  P / O      = perspektywa / ortogonalne\n";
    std::cout << "  + / -      = zmiana FPS\n";
    std::cout << "  ESC        = wyjscie\n";

    engine.run();
    return 0;
}
