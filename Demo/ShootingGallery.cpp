#include "ShootingGallery.h"
#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdio>
#include <string>

namespace {
    const float PI = 3.14159265358979323846f;

    // Wymiary pokoju (przestrzen wewnetrzna miedzy scianami)
    const float ROOM_X_HALF = 10.0f;        // X: [-10, +10]
    const float ROOM_HEIGHT =  6.0f;        // Y: [0, 6]
    const float ROOM_Z_MIN  = -20.0f;       // Z: [-20, +10]
    const float ROOM_Z_MAX  =  10.0f;
    const float ROOM_LENGTH = ROOM_Z_MAX - ROOM_Z_MIN;   // 30
    const float ROOM_WIDTH  = 2.0f * ROOM_X_HALF;        // 20
    const float ROOM_Z_CENTER = (ROOM_Z_MIN + ROOM_Z_MAX) * 0.5f; // -5

    // Bufor kolizji - jak blisko sciany moze podejsc gracz
    const float WALL_BUFFER = 0.5f;

    // Strefa pojawiania celow (w tylnej polowie pokoju, z dala od bocznych scian)
    const float SPAWN_X_HALF = 8.0f;        // X: [-8, +8]
    const float SPAWN_Y_MIN  = 1.0f;
    const float SPAWN_Y_MAX  = 4.0f;
    const float SPAWN_Z_MIN  = -18.0f;
    const float SPAWN_Z_MAX  = -4.0f;
    const float MIN_SPAWN_DIST_FROM_PLAYER = 3.0f;

    // Parametry gry
    const float TARGET_RADIUS    = 0.50f;
    const float HIT_TOLERANCE    = 1.50f;
    const float TARGET_LIFETIME  = 8.0f;
    const float RESPAWN_PAUSE    = 0.40f;
    const int   START_LIVES      = 5;
    const float FLASH_DURATION   = 0.35f;
    const float MAX_SHOOT_DIST   = 100.0f;
    const float PLAYER_HEIGHT    = 1.75f;
    const float PLAYER_MOVE_SPEED = 4.5f;

    Material defaultTargetMaterial() {
        return Material(
            Vec3(0.22f, 0.10f, 0.02f),
            Vec3(0.98f, 0.50f, 0.08f),
            Vec3(1.00f, 1.00f, 1.00f), 48.0f);
    }
}

ShootingGallery::ShootingGallery(Engine& engine)
    : engine_(engine),
      playerEye_(0.0f, PLAYER_HEIGHT, 7.0f),
      playerEyeHome_(0.0f, PLAYER_HEIGHT, 7.0f),
      // Sciany - rozmiary dopasowane do wymiarow pokoju
      floor_    (std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_LENGTH)),
      ceiling_  (std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_LENGTH)),
      backWall_ (std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_HEIGHT)),
      frontWall_(std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_HEIGHT)),
      leftWall_ (std::make_shared<PlaneNode>(ROOM_HEIGHT, ROOM_LENGTH)),
      rightWall_(std::make_shared<PlaneNode>(ROOM_HEIGHT, ROOM_LENGTH)),
      target_   (std::make_shared<SphereNode>(TARGET_RADIUS, 20, 32)),
      floorTex_  (std::make_shared<Texture>()),
      ceilingTex_(std::make_shared<Texture>()),
      wallTex_   (std::make_shared<Texture>()),
      backWallTex_(std::make_shared<Texture>()),
      targetTex_ (std::make_shared<Texture>()),
      targetPos_(0.0f, 2.0f, -8.0f),
      targetRadius_(TARGET_RADIUS),
      targetSpawnTime_(0.0f),
      targetBobPhase_(0.0f),
      targetSpinAngle_(0.0f),
      targetAlive_(true),
      respawnTimer_(0.0f),
      score_(0), streak_(0), bestStreak_(0),
      lives_(START_LIVES), totalTime_(0.0f),
      gameOver_(false),
      hitFlashTime_(0.0f), missFlashTime_(0.0f), crosshairFlash_(0.0f),
      rng_((unsigned)std::chrono::steady_clock::now().time_since_epoch().count())
{
    buildRoom();

    // Cel
    target_->setMaterial(defaultTargetMaterial());
    target_->setTexture(targetTex_);
    target_->setPosition(targetPos_);
    engine_.getSceneRoot()->addChild(target_);

    // Swiatlo: lampa pod sufitem, mocno oswietla caly pokoj
    engine_.getPointLight()->setPosition(Vec3(0.0f, ROOM_HEIGHT - 0.5f, ROOM_Z_CENTER));
    engine_.getPointLight()->setAmbient(Vec3(0.20f, 0.20f, 0.22f));
    engine_.getPointLight()->setDiffuse(Vec3(1.00f, 0.95f, 0.85f));
    engine_.getPointLight()->setAttenuation(1.0f, 0.014f, 0.0014f);

    // Konfiguracja silnika dla gry
    engine_.setCameraControl(CameraControlMode::GAME_CONTROLLED);
    engine_.setCameraYawPitch(0.0f, 0.0f);
    engine_.setMouseSensitivity(0.0030f);
    engine_.setFreeMouseLook(true);
    engine_.getCamera()->setFirstPerson(playerEye_, 0.0f, 0.0f);

    spawnTarget();
}

void ShootingGallery::buildRoom() {
    // ---- Tekstury ----
    floorTex_->generateCheckerboard(256,
        0.50f, 0.50f, 0.55f,
        0.22f, 0.22f, 0.26f, 8);
    ceilingTex_->generateCheckerboard(256,
        0.20f, 0.20f, 0.26f,
        0.10f, 0.10f, 0.14f, 6);
    wallTex_->generateStripes(256,
        0.42f, 0.35f, 0.28f,
        0.30f, 0.22f, 0.18f, 16);
    backWallTex_->generateCheckerboard(256,
        0.55f, 0.30f, 0.20f,
        0.30f, 0.15f, 0.10f, 4);

    // Material dla scian / sufitu (Phong)
    Material wallMat(
        Vec3(0.10f, 0.08f, 0.06f),
        Vec3(0.65f, 0.55f, 0.45f),
        Vec3(0.05f, 0.05f, 0.05f), 8.0f);

    Material floorMat(
        Vec3(0.10f, 0.10f, 0.12f),
        Vec3(0.65f, 0.65f, 0.70f),
        Vec3(0.05f, 0.05f, 0.05f), 4.0f);

    Material ceilingMat(
        Vec3(0.05f, 0.05f, 0.06f),
        Vec3(0.30f, 0.30f, 0.38f),
        Vec3(0.05f, 0.05f, 0.05f), 4.0f);

    Material backMat(
        Vec3(0.12f, 0.06f, 0.04f),
        Vec3(0.70f, 0.45f, 0.30f),
        Vec3(0.10f, 0.05f, 0.05f), 12.0f);

    // ---- Podloga (normal +Y, bez rotacji) ----
    floor_->setPosition(Vec3(0.0f, 0.0f, ROOM_Z_CENTER));
    floor_->setMaterial(floorMat);
    floor_->setTexture(floorTex_);

    // ---- Sufit (rotacja 180 wokol X -> normal -Y, patrzy w dol) ----
    ceiling_->setPosition(Vec3(0.0f, ROOM_HEIGHT, ROOM_Z_CENTER));
    ceiling_->setRotation(Vec3(PI, 0.0f, 0.0f));
    ceiling_->setMaterial(ceilingMat);
    ceiling_->setTexture(ceilingTex_);

    // ---- Sciana tylna (Z = ROOM_Z_MIN, rotacja +90 wokol X -> normal +Z) ----
    backWall_->setPosition(Vec3(0.0f, ROOM_HEIGHT * 0.5f, ROOM_Z_MIN));
    backWall_->setRotation(Vec3(0.5f * PI, 0.0f, 0.0f));
    backWall_->setMaterial(backMat);
    backWall_->setTexture(backWallTex_);

    // ---- Sciana frontowa (Z = ROOM_Z_MAX, rotacja -90 wokol X -> normal -Z) ----
    frontWall_->setPosition(Vec3(0.0f, ROOM_HEIGHT * 0.5f, ROOM_Z_MAX));
    frontWall_->setRotation(Vec3(-0.5f * PI, 0.0f, 0.0f));
    frontWall_->setMaterial(wallMat);
    frontWall_->setTexture(wallTex_);

    // ---- Sciana lewa (X = -ROOM_X_HALF, rotacja -90 wokol Z -> normal +X) ----
    leftWall_->setPosition(Vec3(-ROOM_X_HALF, ROOM_HEIGHT * 0.5f, ROOM_Z_CENTER));
    leftWall_->setRotation(Vec3(0.0f, 0.0f, -0.5f * PI));
    leftWall_->setMaterial(wallMat);
    leftWall_->setTexture(wallTex_);

    // ---- Sciana prawa (X = +ROOM_X_HALF, rotacja +90 wokol Z -> normal -X) ----
    rightWall_->setPosition(Vec3(ROOM_X_HALF, ROOM_HEIGHT * 0.5f, ROOM_Z_CENTER));
    rightWall_->setRotation(Vec3(0.0f, 0.0f, 0.5f * PI));
    rightWall_->setMaterial(wallMat);
    rightWall_->setTexture(wallTex_);

    auto root = engine_.getSceneRoot();
    root->addChild(floor_);
    root->addChild(ceiling_);
    root->addChild(backWall_);
    root->addChild(frontWall_);
    root->addChild(leftWall_);
    root->addChild(rightWall_);
}

Vec3 ShootingGallery::currentTargetWorldPos() const {
    Vec3 pos = targetPos_;
    pos.y += 0.18f * std::sin(targetBobPhase_ * 2.2f);
    return pos;
}

void ShootingGallery::restoreTargetMaterial() {
    target_->setMaterial(defaultTargetMaterial());
}

void ShootingGallery::spawnTarget() {
    std::uniform_real_distribution<float> distX(-SPAWN_X_HALF, SPAWN_X_HALF);
    std::uniform_real_distribution<float> distY(SPAWN_Y_MIN, SPAWN_Y_MAX);
    std::uniform_real_distribution<float> distZ(SPAWN_Z_MIN, SPAWN_Z_MAX);

    // Probujemy znalezc pozycje co najmniej MIN_SPAWN_DIST_FROM_PLAYER od gracza
    for (int attempt = 0; attempt < 30; ++attempt) {
        Vec3 candidate(distX(rng_), distY(rng_), distZ(rng_));
        if (length(candidate - playerEye_) >= MIN_SPAWN_DIST_FROM_PLAYER) {
            targetPos_ = candidate;
            break;
        }
        if (attempt == 29) {
            targetPos_ = candidate;  // fallback
        }
    }

    target_->setPosition(targetPos_);
    targetSpawnTime_ = totalTime_;
    targetBobPhase_  = 0.0f;
    targetAlive_     = true;
    respawnTimer_    = 0.0f;
    restoreTargetMaterial();
}

void ShootingGallery::resetGame() {
    score_         = 0;
    streak_        = 0;
    lives_         = START_LIVES;
    totalTime_     = 0.0f;
    gameOver_      = false;
    hitFlashTime_  = 0.0f;
    missFlashTime_ = 0.0f;
    crosshairFlash_ = 0.0f;
    playerEye_     = playerEyeHome_;
    engine_.setCameraYawPitch(0.0f, 0.0f);
    spawnTarget();
}

void ShootingGallery::onUpdate(float dt) {
    if (!gameOver_) {
        // Ruch gracza WASD na plaszczyznie XZ, sztywno na wysokosci PLAYER_HEIGHT
        float yaw = engine_.getCameraYaw();
        Vec3 fwd(std::sin(yaw), 0.0f, -std::cos(yaw));
        Vec3 rgt(std::cos(yaw), 0.0f,  std::sin(yaw));

        float speed = PLAYER_MOVE_SPEED * dt;
        if (engine_.isKeyDown('w') || engine_.isKeyDown('W')) playerEye_ += fwd * speed;
        if (engine_.isKeyDown('s') || engine_.isKeyDown('S')) playerEye_ -= fwd * speed;
        if (engine_.isKeyDown('a') || engine_.isKeyDown('A')) playerEye_ -= rgt * speed;
        if (engine_.isKeyDown('d') || engine_.isKeyDown('D')) playerEye_ += rgt * speed;

        // Kolizje ze scianami pokoju
        const float xMin = -ROOM_X_HALF + WALL_BUFFER;
        const float xMax =  ROOM_X_HALF - WALL_BUFFER;
        const float zMin =  ROOM_Z_MIN  + WALL_BUFFER;
        const float zMax =  ROOM_Z_MAX  - WALL_BUFFER;

        playerEye_.x = std::max(xMin, std::min(xMax, playerEye_.x));
        playerEye_.z = std::max(zMin, std::min(zMax, playerEye_.z));
        playerEye_.y = PLAYER_HEIGHT;
    }

    engine_.getCamera()->setFirstPerson(
        playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());

    if (gameOver_) return;

    totalTime_       += dt;
    targetBobPhase_  += dt;
    targetSpinAngle_ += dt * 1.4f;
    hitFlashTime_     = std::max(0.0f, hitFlashTime_   - dt);
    missFlashTime_    = std::max(0.0f, missFlashTime_  - dt);
    crosshairFlash_   = std::max(0.0f, crosshairFlash_ - dt);

    if (!targetAlive_ && respawnTimer_ > 0.0f) {
        respawnTimer_ -= dt;
        if (respawnTimer_ <= 0.0f) spawnTarget();
        return;
    }

    if (targetAlive_) {
        Vec3 displayPos = currentTargetWorldPos();
        target_->setPosition(displayPos);
        target_->setRotation(Vec3(0.0f, targetSpinAngle_, 0.0f));

        float elapsed = totalTime_ - targetSpawnTime_;
        if (elapsed > TARGET_LIFETIME) {
            --lives_;
            streak_ = 0;
            missFlashTime_ = FLASH_DURATION;
            if (lives_ <= 0) {
                gameOver_ = true;
                targetAlive_ = false;
            } else {
                spawnTarget();
            }
        }
    }
}

void ShootingGallery::onShoot() {
    if (gameOver_ || !targetAlive_) return;

    // Odswiez kamere najnowszym yaw/pitch (mysz mogla sie poruszyc miedzy klatkami)
    engine_.getCamera()->setFirstPerson(
        playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());

    Vec3 origin = engine_.getCamera()->eyePosition();
    Vec3 dir    = engine_.getCamera()->lookDirection();

    crosshairFlash_ = 0.20f;

    float t = 0.0f;
    Vec3 spherePos = currentTargetWorldPos();

    if (raySphereIntersect(origin, dir, spherePos,
                           targetRadius_ * HIT_TOLERANCE, t)
        && t < MAX_SHOOT_DIST) {
        ++score_;
        ++streak_;
        if (streak_ > bestStreak_) bestStreak_ = streak_;
        if (streak_ % 5 == 0 && lives_ < 9) ++lives_;

        hitFlashTime_ = FLASH_DURATION;
        targetAlive_  = false;
        respawnTimer_ = RESPAWN_PAUSE;

        target_->setMaterial(Material(
            Vec3(0.05f, 0.25f, 0.05f),
            Vec3(0.15f, 1.00f, 0.20f),
            Vec3(1.00f, 1.00f, 1.00f), 96.0f));
    } else {
        --lives_;
        streak_ = 0;
        missFlashTime_ = FLASH_DURATION;
        if (lives_ <= 0) gameOver_ = true;
    }
}

void ShootingGallery::onReset() {
    if (gameOver_) resetGame();
}

void ShootingGallery::onHUD() {
    int W = engine_.windowW();
    int H = engine_.windowH();
    int cx = W / 2;
    int cy = H / 2;

    if (missFlashTime_ > 0.0f) {
        float a = missFlashTime_ / FLASH_DURATION;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.10f, 0.10f, 0.30f * a);
        glBegin(GL_QUADS);
        glVertex2i(0, 0); glVertex2i(W, 0); glVertex2i(W, H); glVertex2i(0, H);
        glEnd();
        glDisable(GL_BLEND);
    }
    if (hitFlashTime_ > 0.0f) {
        float a = hitFlashTime_ / FLASH_DURATION;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.20f, 1.00f, 0.30f, 0.18f * a);
        glBegin(GL_QUADS);
        glVertex2i(0, 0); glVertex2i(W, 0); glVertex2i(W, H); glVertex2i(0, H);
        glEnd();
        glDisable(GL_BLEND);
    }

    // Celownik
    bool flash = crosshairFlash_ > 0.0f;
    glColor3f(flash ? 1.0f : 1.0f, flash ? 0.8f : 1.0f, flash ? 0.2f : 1.0f);
    glLineWidth(flash ? 3.0f : 2.0f);
    int gap = flash ? 8 : 5;
    int len = flash ? 18 : 12;
    glBegin(GL_LINES);
    glVertex2i(cx - gap - len, cy); glVertex2i(cx - gap, cy);
    glVertex2i(cx + gap,       cy); glVertex2i(cx + gap + len, cy);
    glVertex2i(cx, cy - gap - len); glVertex2i(cx, cy - gap);
    glVertex2i(cx, cy + gap);       glVertex2i(cx, cy + gap + len);
    glEnd();
    glLineWidth(1.0f);
    glColor3f(1.0f, 0.25f, 0.25f);
    glPointSize(3.0f);
    glBegin(GL_POINTS); glVertex2i(cx, cy); glEnd();
    glPointSize(1.0f);

    // Statystyki
    int rx = W - 220;
    int ry = H - 22;
    glColor3f(0.55f, 1.00f, 0.55f);
    engine_.drawStringLarge(rx, ry, "WYNIK: " + std::to_string(score_));
    ry -= 28;
    glColor3f(1.00f, 0.55f, 0.55f);
    engine_.drawStringLarge(rx, ry, "ZYCIA: " + std::to_string(lives_));
    ry -= 28;
    glColor3f(0.85f, 0.85f, 0.45f);
    int   sec  = (int)totalTime_;
    int   csec = (int)((totalTime_ - sec) * 100);
    char  buf[64];
    std::snprintf(buf, sizeof(buf), "CZAS: %02d:%02d", sec, csec);
    engine_.drawString(rx, ry, buf);
    ry -= 18;
    if (streak_ >= 2) {
        glColor3f(1.00f, 0.65f, 0.30f);
        engine_.drawString(rx, ry, "SERIA: " + std::to_string(streak_)
                                   + " (rekord: " + std::to_string(bestStreak_) + ")");
    } else if (bestStreak_ > 0) {
        glColor3f(0.60f, 0.60f, 0.60f);
        engine_.drawString(rx, ry, "Rekord serii: " + std::to_string(bestStreak_));
    }

    // Pasek czasu zycia celu
    if (targetAlive_ && !gameOver_) {
        float lifeLeft = 1.0f - (totalTime_ - targetSpawnTime_) / TARGET_LIFETIME;
        if (lifeLeft < 0.0f) lifeLeft = 0.0f;
        int barX = cx - 80, barY = 36, barW = 160, barH = 10;
        glColor3f(0.15f, 0.15f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2i(barX-1, barY-1); glVertex2i(barX+barW+1, barY-1);
        glVertex2i(barX+barW+1, barY+barH+1); glVertex2i(barX-1, barY+barH+1);
        glEnd();
        float fr = (lifeLeft < 0.5f) ? 1.0f : (1.0f - lifeLeft) * 2.0f;
        float fg = (lifeLeft > 0.5f) ? 1.0f : lifeLeft * 2.0f;
        glColor3f(fr, fg, 0.10f);
        glBegin(GL_QUADS);
        glVertex2i(barX, barY); glVertex2i(barX + (int)(barW * lifeLeft), barY);
        glVertex2i(barX + (int)(barW * lifeLeft), barY + barH); glVertex2i(barX, barY + barH);
        glEnd();
    }

    // Naglowek
    glColor3f(0.75f, 0.85f, 1.00f);
    engine_.drawString(10, 60, "STRZELNICA 3D");
    glColor3f(0.55f, 0.65f, 0.85f);
    engine_.drawString(10, 42, "WASD = ruch  |  mysz = celowanie  |  SPACJA = strzal  |  R = reset");

    if (gameOver_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.70f);
        glBegin(GL_QUADS);
        glVertex2i(0, 0); glVertex2i(W, 0); glVertex2i(W, H); glVertex2i(0, H);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(1.0f, 0.30f, 0.30f);
        engine_.drawStringLarge(cx - 100, cy + 50, "KONIEC GRY");
        glColor3f(1.0f, 1.0f, 1.0f);
        engine_.drawStringLarge(cx - 90, cy + 10, "Wynik: " + std::to_string(score_));
        glColor3f(0.85f, 0.85f, 0.50f);
        engine_.drawString(cx - 100, cy - 20,
            "Najlepsza seria: " + std::to_string(bestStreak_));
        glColor3f(0.80f, 0.80f, 0.40f);
        engine_.drawString(cx - 115, cy - 50, "Nacisnij R aby zagrac ponownie");
    }
}
