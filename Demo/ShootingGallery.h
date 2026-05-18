#ifndef SHOOTING_GALLERY_H
#define SHOOTING_GALLERY_H

#include "Engine/Engine.h"
#include <memory>
#include <random>

// Gra: Strzelnica 3D.
// Cel-sfera pojawia sie w losowym miejscu w przestrzeni przed graczem,
// powoli sie obraca i kolysze. Gracz celuje kamera (LPM + mysz),
// strzela spacja - test ray-sphere intersection.
// Trafienie: cel zmienia kolor na zielony, znika, +1 punkt.
// Pudlo: -1 zycie. Cel zostaje.
// Czas zycia celu: 5 sekund. Wygasniecie = -1 zycie i nowy cel.
class ShootingGallery {
public:
    explicit ShootingGallery(Engine& engine);

    // Callbacki rejestrowane w silniku
    void onUpdate(float dt);  // logika gry, animacje
    void onShoot();           // spacja: raycast
    void onHUD();             // celownik, statystyki
    void onReset();           // klawisz R

private:
    void spawnTarget();
    void resetGame();

    Engine& engine_;

    // Pozycja staloczasowa gracza (FP camera eye)
    Vec3 playerEye_;

    // Scena
    std::shared_ptr<PlaneNode>  ground_;
    std::shared_ptr<PlaneNode>  backWall_;
    std::shared_ptr<SphereNode> target_;
    std::shared_ptr<Texture>    groundTex_;
    std::shared_ptr<Texture>    wallTex_;
    std::shared_ptr<Texture>    targetTex_;

    // Stan celu
    Vec3  targetPos_;
    float targetRadius_;
    float targetSpawnTime_;
    float targetBobPhase_;
    float targetSpinAngle_;
    bool  targetAlive_;

    // Stan gry
    int   score_;
    int   lives_;
    float totalTime_;
    bool  gameOver_;
    float hitFlashTime_;   // zielony blysk po trafieniu (sekundy)
    float missFlashTime_;  // czerwony blysk po pudle

    // RNG
    std::mt19937 rng_;
};

#endif
