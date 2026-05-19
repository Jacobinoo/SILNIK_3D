#ifndef SHOOTING_GALLERY_H
#define SHOOTING_GALLERY_H

#include "Engine/Engine.h"
#include <memory>
#include <random>

// Gra: Strzelnica 3D z perspektywy pierwszej osoby w zamknietym pokoju.
// Pokoj o wymiarach 20 x 6 x 30 (X x Y x Z), gracz porusza sie WASD po calej
// powierzchni podlogi (ze sciennymi kolizjami). Cel-sfera pojawia sie
// losowo w tylnej czesci pokoju, obraca i kolysze.
class ShootingGallery {
public:
    explicit ShootingGallery(Engine& engine);

    void onUpdate(float dt);
    void onShoot();
    void onHUD();
    void onReset();

private:
    void buildRoom();
    void spawnTarget();
    void resetGame();
    void restoreTargetMaterial();
    Vec3 currentTargetWorldPos() const;

    Engine& engine_;

    Vec3 playerEye_;
    Vec3 playerEyeHome_;

    // Sciany pokoju (6 plaszczyzn)
    std::shared_ptr<PlaneNode>  floor_;
    std::shared_ptr<PlaneNode>  ceiling_;
    std::shared_ptr<PlaneNode>  backWall_;
    std::shared_ptr<PlaneNode>  frontWall_;
    std::shared_ptr<PlaneNode>  leftWall_;
    std::shared_ptr<PlaneNode>  rightWall_;

    // Cel
    std::shared_ptr<SphereNode> target_;

    // Tekstury
    std::shared_ptr<Texture>    floorTex_;
    std::shared_ptr<Texture>    ceilingTex_;
    std::shared_ptr<Texture>    wallTex_;
    std::shared_ptr<Texture>    backWallTex_;
    std::shared_ptr<Texture>    targetTex_;

    // Stan celu
    Vec3  targetPos_;
    float targetRadius_;
    float targetSpawnTime_;
    float targetBobPhase_;
    float targetSpinAngle_;
    bool  targetAlive_;
    float respawnTimer_;

    // Stan gry
    int   score_;
    int   streak_;
    int   bestStreak_;
    int   lives_;
    float totalTime_;
    bool  gameOver_;
    float hitFlashTime_;
    float missFlashTime_;
    float crosshairFlash_;

    std::mt19937 rng_;
};

#endif
