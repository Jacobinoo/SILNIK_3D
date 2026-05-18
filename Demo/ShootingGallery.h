#ifndef SHOOTING_GALLERY_H
#define SHOOTING_GALLERY_H

#include "Engine/Engine.h"
#include <memory>
#include <random>

// Gra: Strzelnica 3D z perspektywy pierwszej osoby.
// Cel-sfera pojawia sie losowo, obraca i lekko koluje. Gracz porusza sie WASD
// (na plaszczyznie XZ, stala wysokosc), celuje myszka (free look),
// strzela SPACJA. Trafienie = +1 pkt (z bonusem za serie), pudlo/timeout = -1 zycie.
class ShootingGallery {
public:
    explicit ShootingGallery(Engine& engine);

    void onUpdate(float dt);
    void onShoot();
    void onHUD();
    void onReset();

private:
    void spawnTarget();
    void resetGame();
    void restoreTargetMaterial();
    Vec3 currentTargetWorldPos() const;

    Engine& engine_;

    // Pozycja gracza (FP eye height)
    Vec3 playerEye_;
    Vec3 playerEyeHome_;  // pozycja startowa do resetu

    // Scena
    std::shared_ptr<PlaneNode>  ground_;
    std::shared_ptr<PlaneNode>  ceiling_;
    std::shared_ptr<PlaneNode>  backWall_;
    std::shared_ptr<SphereNode> target_;
    std::shared_ptr<CubeNode>   marker_;     // dekoracyjne slupki na podlodze
    std::shared_ptr<Texture>    groundTex_;
    std::shared_ptr<Texture>    wallTex_;
    std::shared_ptr<Texture>    ceilingTex_;
    std::shared_ptr<Texture>    targetTex_;

    // Stan celu
    Vec3  targetPos_;
    float targetRadius_;
    float targetSpawnTime_;
    float targetBobPhase_;
    float targetSpinAngle_;
    bool  targetAlive_;
    float respawnTimer_;  // > 0 = krotka pauza przed nowym celem

    // Stan gry
    int   score_;
    int   streak_;
    int   bestStreak_;
    int   lives_;
    float totalTime_;
    bool  gameOver_;
    float hitFlashTime_;    // zielony blysk po trafieniu
    float missFlashTime_;   // czerwony blysk pudla
    float crosshairFlash_;  // krotki rozblysk celownika na strzal

    std::mt19937 rng_;
};

#endif
