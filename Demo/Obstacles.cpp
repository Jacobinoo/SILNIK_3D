// =============================================================================
// Obstacles.cpp - Osoba 2
// Przeszkody w pokoju: cylindry, stozki, skrzynie.
// Wraz z kolizjami gracza i blokowaniem strzalow.
// =============================================================================

#include "ShootingGallery.h"
#include "Demo/Constants.h"
#include <algorithm>
#include <cmath>

using namespace sg;

void ShootingGallery::buildObstacles() {
    pillarTex_->generateMarble(256, 0.30f,0.30f,0.35f, 0.85f,0.85f,0.90f, 3.0f);

    // Stozki: probujemy z pliku BMP, fallback do paskow.
    if (!coneTex_->loadBMP("assets/textures/cone.bmp")) {
        coneTex_->generateStripes(128, 1.00f,0.55f,0.10f, 1.00f,0.95f,0.95f, 6);
    }
    // Skrzynie: probujemy z pliku BMP, fallback do proceduralnego drewna.
    if (!boxTex_->loadBMP("assets/textures/box.bmp")) {
        boxTex_->generateWood(256, 0.30f,0.18f,0.08f, 0.70f,0.50f,0.25f, 6);
    }

    Material pillarMat(Vec3(0.30f,0.30f,0.32f), Vec3(0.65f,0.65f,0.70f), Vec3(0.30f,0.30f,0.30f), 24.0f);
    Material coneMat  (Vec3(0.40f,0.20f,0.05f), Vec3(0.95f,0.55f,0.15f), Vec3(0.50f,0.50f,0.50f), 16.0f);
    Material boxMat   (Vec3(0.30f,0.20f,0.10f), Vec3(0.70f,0.50f,0.30f), Vec3(0.10f,0.10f,0.10f),  6.0f);

    // ---- Cylindryczne kolumny rozsiane po pokoju ----
    struct CylDef { Vec3 base; float r, h; };
    std::vector<CylDef> cyls = {
        { Vec3(-4.0f, 0.0f,  -7.5f), 0.55f, 4.0f },
        { Vec3( 4.5f, 0.0f, -10.5f), 0.55f, 4.0f },
        { Vec3( 0.0f, 0.0f, -14.0f), 0.65f, 4.5f },
        { Vec3(-6.5f, 0.0f, -16.0f), 0.50f, 3.5f },
        { Vec3( 5.0f, 0.0f,   4.0f), 0.50f, 4.0f },
        { Vec3(-3.0f, 0.0f,  -2.5f), 0.55f, 3.5f },
    };
    for (const auto& d : cyls) {
        auto node = std::make_shared<CylinderNode>(d.r, d.h, 28);
        node->setPosition(Vec3(d.base.x, d.base.y + d.h * 0.5f, d.base.z));
        node->setMaterial(pillarMat);
        node->setTexture(pillarTex_);
        engine_.getSceneRoot()->addChild(node);
        cylinderNodes_.push_back(node);
        cylinderObs_.push_back({ d.base, d.r, d.h });
    }

    // ---- Stozki rozsiane po pokoju ----
    struct ConeDef { Vec3 base; float r, h; };
    std::vector<ConeDef> cones = {
        { Vec3( 6.0f, 0.0f,  -7.0f), 0.85f, 3.2f },
        { Vec3(-5.5f, 0.0f,   4.0f), 0.70f, 3.0f },
        { Vec3( 2.5f, 0.0f,  -2.0f), 0.60f, 2.5f },
    };
    for (const auto& d : cones) {
        auto node = std::make_shared<ConeNode>(d.r, d.h, 28);
        node->setPosition(Vec3(d.base.x, d.base.y + d.h * 0.5f, d.base.z));
        node->setMaterial(coneMat);
        node->setTexture(coneTex_);
        engine_.getSceneRoot()->addChild(node);
        coneNodes_.push_back(node);
        coneObs_.push_back({ d.base, d.r, d.h });
    }

    // ---- Skrzynie ----
    struct BoxDef { Vec3 center; float w, h, d; };
    std::vector<BoxDef> boxes = {
        { Vec3( 5.5f, 1.0f, -15.5f), 1.6f, 2.0f, 1.6f },
        { Vec3(-7.0f, 1.0f,   1.0f), 1.8f, 2.0f, 1.5f },
    };
    for (const auto& d : boxes) {
        auto node = std::make_shared<CubeNode>(1.0f);
        node->setPosition(d.center);
        node->setScale(Vec3(d.w, d.h, d.d));
        node->setMaterial(boxMat);
        node->setTexture(boxTex_);
        engine_.getSceneRoot()->addChild(node);
        boxNodes_.push_back(node);

        Vec3 boxMin(d.center.x - d.w * 0.5f, d.center.y - d.h * 0.5f, d.center.z - d.d * 0.5f);
        Vec3 boxMax(d.center.x + d.w * 0.5f, d.center.y + d.h * 0.5f, d.center.z + d.d * 0.5f);
        boxObs_.push_back({ boxMin, boxMax });
    }
}

bool ShootingGallery::isInsideObstacle(const Vec3& pos, float margin) const {
    // Cylindry - sprawdz dystans XZ i zakres Y
    for (const auto& o : cylinderObs_) {
        float dx = pos.x - o.base.x;
        float dz = pos.z - o.base.z;
        float minR = o.radius + margin;
        if (dx*dx + dz*dz < minR*minR &&
            pos.y + margin >= o.base.y &&
            pos.y - margin <= o.base.y + o.height) {
            return true;
        }
    }
    // Stozki - efektywny promien zalezy od wysokosci nad podstawa
    for (const auto& o : coneObs_) {
        float yLocal = pos.y - o.base.y;
        if (yLocal < -margin || yLocal > o.height + margin) continue;
        float effR = o.radius * (1.0f - std::max(0.0f, yLocal) / o.height);
        if (effR < 0.0f) effR = 0.0f;
        float dx = pos.x - o.base.x;
        float dz = pos.z - o.base.z;
        float minR = effR + margin;
        if (dx*dx + dz*dz < minR*minR) return true;
    }
    // Skrzynie - rozszerzony AABB
    for (const auto& o : boxObs_) {
        if (pos.x > o.boxMin.x - margin && pos.x < o.boxMax.x + margin &&
            pos.y > o.boxMin.y - margin && pos.y < o.boxMax.y + margin &&
            pos.z > o.boxMin.z - margin && pos.z < o.boxMax.z + margin) {
            return true;
        }
    }
    return false;
}

bool ShootingGallery::obstacleBlocksRay(const Vec3& origin, const Vec3& dir, float maxT) const {
    float t;
    for (const auto& o : cylinderObs_) {
        if (rayCylinderIntersect(origin, dir, o.base, o.radius, o.height, t)
            && t > 0.0001f && t < maxT) return true;
    }
    for (const auto& o : coneObs_) {
        if (rayConeIntersect(origin, dir, o.base, o.radius, o.height, t)
            && t > 0.0001f && t < maxT) return true;
    }
    for (const auto& o : boxObs_) {
        if (rayAABBIntersect(origin, dir, o.boxMin, o.boxMax, t)
            && t > 0.0001f && t < maxT) return true;
    }
    return false;
}

void ShootingGallery::applyObstacleCollision() {
    // Cylindry - kolizja okragla (push out po stycznej)
    for (const auto& o : cylinderObs_) {
        float dx = playerEye_.x - o.base.x;
        float dz = playerEye_.z - o.base.z;
        float minR = o.radius + PLAYER_RADIUS;
        float distSq = dx*dx + dz*dz;
        if (distSq < minR*minR && distSq > 0.0001f) {
            float dist = std::sqrt(distSq);
            float push = (minR - dist) / dist;
            playerEye_.x += dx * push;
            playerEye_.z += dz * push;
        }
    }
    // Stozki - jak cylinder o promieniu podstawy (gracz nie wejdzie pod nawis)
    for (const auto& o : coneObs_) {
        float dx = playerEye_.x - o.base.x;
        float dz = playerEye_.z - o.base.z;
        float minR = o.radius + PLAYER_RADIUS;
        float distSq = dx*dx + dz*dz;
        if (distSq < minR*minR && distSq > 0.0001f) {
            float dist = std::sqrt(distSq);
            float push = (minR - dist) / dist;
            playerEye_.x += dx * push;
            playerEye_.z += dz * push;
        }
    }
    // Skrzynie - znajdz najblizszy punkt na AABB i wypchnij gracza
    for (const auto& o : boxObs_) {
        float cx = std::max(o.boxMin.x, std::min(playerEye_.x, o.boxMax.x));
        float cz = std::max(o.boxMin.z, std::min(playerEye_.z, o.boxMax.z));
        float dx = playerEye_.x - cx;
        float dz = playerEye_.z - cz;
        float distSq = dx*dx + dz*dz;
        if (distSq < PLAYER_RADIUS * PLAYER_RADIUS) {
            if (distSq < 0.0001f) {
                // Gracz wewnatrz AABB - wypchnij w najblizsza krawedz
                float distLeft  = playerEye_.x - o.boxMin.x;
                float distRight = o.boxMax.x - playerEye_.x;
                float distFront = playerEye_.z - o.boxMin.z;
                float distBack  = o.boxMax.z - playerEye_.z;
                float m = std::min(std::min(distLeft, distRight), std::min(distFront, distBack));
                if      (m == distLeft)  playerEye_.x = o.boxMin.x - PLAYER_RADIUS;
                else if (m == distRight) playerEye_.x = o.boxMax.x + PLAYER_RADIUS;
                else if (m == distFront) playerEye_.z = o.boxMin.z - PLAYER_RADIUS;
                else                     playerEye_.z = o.boxMax.z + PLAYER_RADIUS;
            } else {
                float dist = std::sqrt(distSq);
                float push = (PLAYER_RADIUS - dist) / dist;
                playerEye_.x += dx * push;
                playerEye_.z += dz * push;
            }
        }
    }
}
