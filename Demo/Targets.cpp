// =============================================================================
// Targets.cpp - Osoba 3
// Zarzadzanie celami: pula sfer, spawnowanie fal (1-3 cele), animacja ruchu
// (statyczny / ruch po osi X / ruch po osi Z) z faza i amplituda.
// =============================================================================

#include "ShootingGallery.h"
#include "Demo/Constants.h"
#include <cmath>

using namespace sg;

Material ShootingGallery::defaultTargetMaterial() const {
    return Material(
        Vec3(0.30f, 0.15f, 0.05f),
        Vec3(0.98f, 0.50f, 0.08f),
        Vec3(1.00f, 1.00f, 1.00f), 48.0f);
}

void ShootingGallery::initTargetPool() {
    for (int i = 0; i < MAX_TARGETS; ++i) {
        auto node = std::make_shared<SphereNode>(TARGET_RADIUS, 20, 32);
        node->setMaterial(defaultTargetMaterial());
        node->setTexture(targetTex_);
        node->setVisible(false);
        engine_.getSceneRoot()->addChild(node);
        targetNodePool_.push_back(node);
    }
}

// Aktualna pozycja celu z uwzglednieniem ruchu liniowego ping-pong i kolysania.
Vec3 ShootingGallery::currentTargetPos(const Target& t) const {
    Vec3 pos = t.basePos;
    if (t.moveRange > 0.01f) {
        pos += t.moveAxis * (t.moveRange * std::sin(totalTime_ * t.moveSpeed + t.movePhase));
    }
    pos.y += 0.18f * std::sin(t.bobPhase * 2.2f);
    return pos;
}

void ShootingGallery::spawnWave() {
    std::uniform_real_distribution<float> distR(0.0f, 1.0f);
    float r = distR(rng_);
    // Rozmiar fali: czesciej 1, czasem 2, rzadziej 3.
    waveSize_ = (r < 0.50f) ? 1 : (r < 0.85f) ? 2 : 3;

    // Schowaj wszystkie sfery z puli i wyczysc liste aktywnych celow.
    for (auto& node : targetNodePool_) node->setVisible(false);
    targets_.clear();

    std::uniform_real_distribution<float> distX(-SPAWN_X_HALF, SPAWN_X_HALF);
    std::uniform_real_distribution<float> distY(SPAWN_Y_MIN, SPAWN_Y_MAX);
    std::uniform_real_distribution<float> distZ(SPAWN_Z_MIN, SPAWN_Z_MAX);

    for (int i = 0; i < waveSize_; ++i) {
        Target t;
        t.nodeIndex = i;
        t.spawnTime = totalTime_;
        t.bobPhase  = 0.0f;
        t.spinAngle = 0.0f;
        t.movePhase = distR(rng_) * 6.28f;

        // Wybor trybu ruchu: 30% static, 40% X axis, 30% Z axis.
        float mr = distR(rng_);
        if (mr < 0.30f) {
            t.moveAxis = Vec3(0,0,0); t.moveRange = 0; t.moveSpeed = 0;
        } else if (mr < 0.70f) {
            // Os X: prawie 2x szybszy niz wczesniej (1.7..2.8 rad/s).
            t.moveAxis = Vec3(1,0,0);
            t.moveRange = MAX_MOVE_RANGE;
            t.moveSpeed = 1.7f + distR(rng_) * 1.1f;
        } else {
            // Os Z: szybszy, mniejsza amplituda (1.7..2.9 rad/s).
            t.moveAxis = Vec3(0,0,1);
            t.moveRange = MAX_MOVE_RANGE * 0.8f;
            t.moveSpeed = 1.7f + distR(rng_) * 1.2f;
        }

        // Znajdz valid pozycje: dostatecznie daleko od gracza, nie w przeszkodzie,
        // nie za blisko innych celow w tej fali.
        bool found = false;
        for (int attempt = 0; attempt < 60; ++attempt) {
            Vec3 candidate(distX(rng_), distY(rng_), distZ(rng_));
            if (length(candidate - playerEye_) < MIN_SPAWN_DIST_FROM_PLAYER) continue;

            // Margines obejmuje promien celu, pelen zakres ruchu i rezerwe.
            float margin = TARGET_RADIUS + t.moveRange + 0.25f;
            if (isInsideObstacle(candidate, margin)) continue;

            bool tooClose = false;
            for (const auto& other : targets_) {
                if (length(candidate - other.basePos) < MIN_SPAWN_DIST_FROM_OTHER) {
                    tooClose = true; break;
                }
            }
            if (tooClose) continue;

            t.basePos = candidate;
            found = true;
            break;
        }

        if (!found) {
            // Fallback: stacjonarny cel w bezpiecznym miejscu.
            t.moveAxis = Vec3(0,0,0); t.moveRange = 0;
            t.basePos = Vec3(0, 2.5f, -6.0f);
        }

        auto& node = targetNodePool_[t.nodeIndex];
        node->setVisible(true);
        node->setMaterial(defaultTargetMaterial());
        node->setPosition(currentTargetPos(t));
        node->setRotation(Vec3(0, 0, 0));

        targets_.push_back(t);
    }

    waveRespawnTimer_ = 0.0f;
}
