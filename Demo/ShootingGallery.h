#ifndef SHOOTING_GALLERY_H
#define SHOOTING_GALLERY_H

#include "Engine/Engine.h"
#include <memory>
#include <random>
#include <vector>

// Gra: Strzelnica 3D w zamknietym pokoju z roznymi przeszkodami.
// Przeszkody: kolumny (cylindry), stozki, skrzynie (AABB).
// Fale 1-3 ruchomych celow. Dwie lampy oswietlaja pokoj.
class ShootingGallery {
public:
    explicit ShootingGallery(Engine& engine);

    void onUpdate(float dt);
    void onShoot();
    void onHUD();
    void onReset();

private:
    // ---------- Struktury ----------

    struct Target {
        Vec3  basePos;
        Vec3  moveAxis;
        float moveRange;
        float moveSpeed;
        float movePhase;
        float bobPhase;
        float spinAngle;
        float spawnTime;
        int   nodeIndex;
    };

    struct ObstacleCyl  { Vec3 base;   float radius, height; };
    struct ObstacleCone { Vec3 base;   float radius, height; };
    struct ObstacleBox  { Vec3 boxMin; Vec3  boxMax; };

    // ---------- Metody ----------

    void buildRoom();
    void buildObstacles();
    void buildDecorations();
    void initTargetPool();
    void spawnWave();
    void resetGame();

    Vec3  currentTargetPos(const Target& t) const;
    bool  isInsideObstacle(const Vec3& pos, float margin) const;
    bool  obstacleBlocksRay(const Vec3& origin, const Vec3& dir, float maxT) const;
    void  applyObstacleCollision();
    Material defaultTargetMaterial() const;

    // ---------- Pola ----------

    Engine& engine_;

    Vec3 playerEye_;
    Vec3 playerEyeHome_;

    // Pokoj
    std::shared_ptr<PlaneNode>  floor_;
    std::shared_ptr<PlaneNode>  ceiling_;
    std::shared_ptr<PlaneNode>  backWall_;
    std::shared_ptr<PlaneNode>  frontWall_;
    std::shared_ptr<PlaneNode>  leftWall_;
    std::shared_ptr<PlaneNode>  rightWall_;

    // Druga lampa (pierwsza jest w silniku)
    std::shared_ptr<PointLight> secondLight_;

    // Przeszkody i ich wezly sceny
    std::vector<std::shared_ptr<CylinderNode>> cylinderNodes_;
    std::vector<std::shared_ptr<ConeNode>>     coneNodes_;
    std::vector<std::shared_ptr<CubeNode>>     boxNodes_;
    std::vector<ObstacleCyl>  cylinderObs_;
    std::vector<ObstacleCone> coneObs_;
    std::vector<ObstacleBox>  boxObs_;

    // Pula 3 sfer dla celow
    std::vector<std::shared_ptr<SphereNode>> targetNodePool_;

    // Aktywne cele (bieżąca fala)
    std::vector<Target> targets_;
    float waveRespawnTimer_;
    int   waveSize_;

    // Tekstury
    std::shared_ptr<Texture> floorTex_;
    std::shared_ptr<Texture> ceilingTex_;
    std::shared_ptr<Texture> wallTex_;
    std::shared_ptr<Texture> backWallTex_;
    std::shared_ptr<Texture> pillarTex_;
    std::shared_ptr<Texture> coneTex_;
    std::shared_ptr<Texture> boxTex_;
    std::shared_ptr<Texture> targetTex_;
    std::shared_ptr<Texture> decorationTex_;
    std::shared_ptr<Texture> torusTex_;

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
