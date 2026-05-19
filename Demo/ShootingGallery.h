#ifndef SHOOTING_GALLERY_H
#define SHOOTING_GALLERY_H

#include "Engine/Engine.h"
#include <memory>
#include <random>
#include <vector>

// Gra: Strzelnica 3D z perspektywy pierwszej osoby.
// Pokoj 20 x 6 x 30 z kolumnami-przeszkodami i ruchomymi celami.
// Fale 1-3 celow naraz. Przeszkody blokuja zarowno ruch gracza jak i strzaly.
class ShootingGallery {
public:
    explicit ShootingGallery(Engine& engine);

    void onUpdate(float dt);
    void onShoot();
    void onHUD();
    void onReset();

private:
    // ---------- Wewnetrzne struktury ----------

    // Pojedynczy cel: pozycja bazowa + opcjonalny ruch liniowy ping-pong.
    struct Target {
        Vec3  basePos;
        Vec3  moveAxis;      // (0,0,0) = statyczny
        float moveRange;     // amplituda (m)
        float moveSpeed;     // czestosc (rad/s)
        float movePhase;     // przesuniecie fazy (rad)
        float bobPhase;
        float spinAngle;
        float spawnTime;
        int   nodeIndex;     // indeks w targetNodePool_
    };

    // Przeszkoda: pionowy walec stojacy na podlodze.
    struct Obstacle {
        Vec3  base;          // dolny srodek (na podlodze)
        float radius;
        float height;
    };

    // ---------- Pomocnicze metody ----------

    void buildRoom();
    void buildObstacles();
    void initTargetPool();
    void spawnWave();
    void resetGame();

    Vec3 currentTargetPos(const Target& t) const;
    bool isInsideObstacle(const Vec3& pos, float margin) const;
    bool obstacleBlocksRay(const Vec3& origin, const Vec3& dir, float maxT) const;
    void applyObstacleCollision();
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

    // Przeszkody (kolumny)
    std::vector<std::shared_ptr<CylinderNode>> obstacleNodes_;
    std::vector<Obstacle> obstacles_;

    // Pula sfer dla celow (rezerwujemy MAX_TARGETS sfer raz, ukrywamy nieuzywane)
    std::vector<std::shared_ptr<SphereNode>> targetNodePool_;

    // Aktywne cele (bieżąca fala)
    std::vector<Target> targets_;

    // Stan fali
    float waveRespawnTimer_;
    int   waveSize_;             // ile celow bylo w aktualnej fali (dla HUD)

    // Tekstury
    std::shared_ptr<Texture> floorTex_;
    std::shared_ptr<Texture> ceilingTex_;
    std::shared_ptr<Texture> wallTex_;
    std::shared_ptr<Texture> backWallTex_;
    std::shared_ptr<Texture> obstacleTex_;
    std::shared_ptr<Texture> targetTex_;

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
