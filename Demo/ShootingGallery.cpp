#include "ShootingGallery.h"
#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdio>
#include <string>

namespace {
    const float TARGET_RADIUS    = 0.50f;
    const float HIT_TOLERANCE    = 1.35f;   // mnoznik promienia dla wykrywania trafienia
    const float TARGET_LIFETIME  = 7.0f;
    const float RESPAWN_PAUSE    = 0.40f;
    const int   START_LIVES      = 5;
    const float FLASH_DURATION   = 0.35f;
    const float MAX_SHOOT_DIST   = 100.0f;
    const float PLAYER_HEIGHT    = 1.75f;
    const float PLAYER_MOVE_SPEED = 4.0f;   // m/s

    // Granice areny (dla ruchu gracza)
    const float ARENA_X_MIN = -8.0f;
    const float ARENA_X_MAX =  8.0f;
    const float ARENA_Z_MIN = -2.0f;        // gracz nie wchodzi w strefe celow
    const float ARENA_Z_MAX =  9.5f;

    // Strefa pojawiania celow
    const float SPAWN_X_MIN = -7.0f;
    const float SPAWN_X_MAX =  7.0f;
    const float SPAWN_Y_MIN =  1.0f;
    const float SPAWN_Y_MAX =  3.5f;
    const float SPAWN_Z_MIN = -14.0f;
    const float SPAWN_Z_MAX =  -3.0f;

    Material defaultTargetMaterial() {
        return Material(
            Vec3(0.22f, 0.10f, 0.02f),
            Vec3(0.98f, 0.50f, 0.08f),
            Vec3(1.00f, 1.00f, 1.00f), 48.0f);
    }
}

ShootingGallery::ShootingGallery(Engine& engine)
    : engine_(engine),
      playerEye_(0.0f, PLAYER_HEIGHT, 6.0f),
      playerEyeHome_(0.0f, PLAYER_HEIGHT, 6.0f),
      ground_  (std::make_shared<PlaneNode>(40.0f, 40.0f)),
      ceiling_ (std::make_shared<PlaneNode>(40.0f, 40.0f)),
      backWall_(std::make_shared<PlaneNode>(30.0f, 12.0f)),
      target_  (std::make_shared<SphereNode>(TARGET_RADIUS, 20, 32)),
      marker_  (std::make_shared<CubeNode>(0.3f)),
      groundTex_ (std::make_shared<Texture>()),
      wallTex_   (std::make_shared<Texture>()),
      ceilingTex_(std::make_shared<Texture>()),
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
    // ---- Tekstury proceduralne ----
    groundTex_->generateCheckerboard(256,
        0.45f, 0.45f, 0.48f,
        0.20f, 0.20f, 0.24f, 10);
    wallTex_->generateGradient(256,
        0.18f, 0.14f, 0.12f,
        0.40f, 0.30f, 0.22f, false);
    ceilingTex_->generateCheckerboard(256,
        0.22f, 0.22f, 0.28f,
        0.12f, 0.12f, 0.16f, 8);
    targetTex_->generateCheckerboard(128,
        1.00f, 0.30f, 0.10f,
        1.00f, 0.95f, 0.50f, 6);

    // ---- Podloga ----
    ground_->setPosition(Vec3(0.0f, 0.0f, -5.0f));
    ground_->setMaterial(Material(
        Vec3(0.10f, 0.10f, 0.10f),
        Vec3(0.55f, 0.55f, 0.58f),
        Vec3(0.05f, 0.05f, 0.05f), 4.0f));
    ground_->setTexture(groundTex_);

    // ---- Sufit (obrocony aby normal pokazywal w dol) ----
    ceiling_->setPosition(Vec3(0.0f, 6.0f, -5.0f));
    ceiling_->setRotation(Vec3(3.14159265f, 0.0f, 0.0f)); // 180 deg around X -> normal w dol
    ceiling_->setMaterial(Material(
        Vec3(0.05f, 0.05f, 0.08f),
        Vec3(0.30f, 0.30f, 0.38f),
        Vec3(0.05f, 0.05f, 0.05f), 4.0f));
    ceiling_->setTexture(ceilingTex_);

    // ---- Sciana tylna ----
    backWall_->setPosition(Vec3(0.0f, 3.0f, -18.0f));
    backWall_->setRotation(Vec3(1.5707963f, 0.0f, 0.0f)); // 90 deg around X -> pionowa, normal w +Z
    backWall_->setMaterial(Material(
        Vec3(0.10f, 0.08f, 0.06f),
        Vec3(0.55f, 0.42f, 0.30f),
        Vec3(0.05f, 0.05f, 0.05f), 6.0f));
    backWall_->setTexture(wallTex_);

    // ---- Cel ----
    target_->setMaterial(defaultTargetMaterial());
    target_->setTexture(targetTex_);
    target_->setPosition(targetPos_);

    // ---- Swiatlo: centralna lampa wysoko ----
    engine_.getPointLight()->setPosition(Vec3(0.0f, 5.5f, -5.0f));
    engine_.getPointLight()->setAttenuation(1.0f, 0.018f, 0.0025f);
    engine_.getPointLight()->setDiffuse(Vec3(1.0f, 0.95f, 0.85f));

    // ---- Dodanie do grafu sceny ----
    auto root = engine_.getSceneRoot();
    root->addChild(ground_);
    root->addChild(ceiling_);
    root->addChild(backWall_);
    root->addChild(target_);

    // ---- Konfiguracja silnika dla gry ----
    engine_.setCameraControl(CameraControlMode::GAME_CONTROLLED);
    engine_.setCameraYawPitch(0.0f, 0.0f);
    engine_.setMouseSensitivity(0.0030f);
    engine_.setFreeMouseLook(true);
    engine_.getCamera()->setFirstPerson(playerEye_, 0.0f, 0.0f);

    spawnTarget();
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
    std::uniform_real_distribution<float> distX(SPAWN_X_MIN, SPAWN_X_MAX);
    std::uniform_real_distribution<float> distY(SPAWN_Y_MIN, SPAWN_Y_MAX);
    std::uniform_real_distribution<float> distZ(SPAWN_Z_MIN, SPAWN_Z_MAX);

    targetPos_ = Vec3(distX(rng_), distY(rng_), distZ(rng_));
    target_->setPosition(targetPos_);
    targetSpawnTime_ = totalTime_;
    targetBobPhase_  = 0.0f;
    targetAlive_     = true;
    respawnTimer_    = 0.0f;
    restoreTargetMaterial();
}

void ShootingGallery::resetGame() {
    score_        = 0;
    streak_       = 0;
    lives_        = START_LIVES;
    totalTime_    = 0.0f;
    gameOver_     = false;
    hitFlashTime_ = 0.0f;
    missFlashTime_ = 0.0f;
    crosshairFlash_ = 0.0f;
    playerEye_    = playerEyeHome_;
    engine_.setCameraYawPitch(0.0f, 0.0f);
    spawnTarget();
}

void ShootingGallery::onUpdate(float dt) {
    // Aktualizacja kamery zawsze (zeby widok byl plynny nawet po game over)
    if (!gameOver_) {
        // Ruch gracza WASD na plaszczyznie XZ
        float yaw = engine_.getCameraYaw();
        Vec3 fwd(std::sin(yaw), 0.0f, -std::cos(yaw));
        Vec3 rgt(std::cos(yaw), 0.0f,  std::sin(yaw));

        float speed = PLAYER_MOVE_SPEED * dt;
        if (engine_.isKeyDown('w') || engine_.isKeyDown('W')) playerEye_ += fwd * speed;
        if (engine_.isKeyDown('s') || engine_.isKeyDown('S')) playerEye_ -= fwd * speed;
        if (engine_.isKeyDown('a') || engine_.isKeyDown('A')) playerEye_ -= rgt * speed;
        if (engine_.isKeyDown('d') || engine_.isKeyDown('D')) playerEye_ += rgt * speed;

        playerEye_.x = std::max(ARENA_X_MIN, std::min(ARENA_X_MAX, playerEye_.x));
        playerEye_.z = std::max(ARENA_Z_MIN, std::min(ARENA_Z_MAX, playerEye_.z));
        playerEye_.y = PLAYER_HEIGHT;
    }

    engine_.getCamera()->setFirstPerson(
        playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());

    if (gameOver_) return;

    // Animacje i czas
    totalTime_       += dt;
    targetBobPhase_  += dt;
    targetSpinAngle_ += dt * 1.4f;
    hitFlashTime_     = std::max(0.0f, hitFlashTime_   - dt);
    missFlashTime_    = std::max(0.0f, missFlashTime_  - dt);
    crosshairFlash_   = std::max(0.0f, crosshairFlash_ - dt);

    // Po trafieniu: krotka pauza zanim spawnujemy nowy cel
    if (!targetAlive_ && respawnTimer_ > 0.0f) {
        respawnTimer_ -= dt;
        if (respawnTimer_ <= 0.0f) {
            spawnTarget();
        }
        return;
    }

    if (targetAlive_) {
        // Animacja: obrot + kolysanie pionowe
        Vec3 displayPos = currentTargetWorldPos();
        target_->setPosition(displayPos);
        target_->setRotation(Vec3(0.0f, targetSpinAngle_, 0.0f));

        // Wygasniecie celu = pudlo
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

    // KRYTYCZNE: odswiez kamere najnowszym yaw/pitch z silnika,
    // poniewaz mysz mogla sie poruszyc miedzy ostatnia klatka a klawiszem.
    engine_.getCamera()->setFirstPerson(
        playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());

    Vec3 origin = engine_.getCamera()->eyePosition();
    Vec3 dir    = engine_.getCamera()->lookDirection();

    crosshairFlash_ = 0.20f;  // wizualne potwierdzenie strzalu

    float t = 0.0f;
    Vec3 spherePos = currentTargetWorldPos();

    if (raySphereIntersect(origin, dir, spherePos,
                           targetRadius_ * HIT_TOLERANCE, t)
        && t < MAX_SHOOT_DIST) {
        // TRAFIENIE
        ++score_;
        ++streak_;
        if (streak_ > bestStreak_) bestStreak_ = streak_;
        // Bonus za serie co 5 trafien -> +1 zycie (max 9)
        if (streak_ % 5 == 0 && lives_ < 9) ++lives_;

        hitFlashTime_ = FLASH_DURATION;
        targetAlive_  = false;
        respawnTimer_ = RESPAWN_PAUSE;

        // Wizualnie: zielony blysk na chwile (bez respawnu)
        target_->setMaterial(Material(
            Vec3(0.05f, 0.25f, 0.05f),
            Vec3(0.15f, 1.00f, 0.20f),
            Vec3(1.00f, 1.00f, 1.00f), 96.0f));
    } else {
        // PUDLO
        --lives_;
        streak_ = 0;
        missFlashTime_ = FLASH_DURATION;
        if (lives_ <= 0) {
            gameOver_ = true;
        }
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

    // ---- Czerwony blysk pudla ----
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

    // ---- Zielony przyblysk trafienia (winieta) ----
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

    // ---- Celownik (krzyzyk + ramka) ----
    bool flash = crosshairFlash_ > 0.0f;
    float r = flash ? 1.0f : 1.0f;
    float g = flash ? 0.8f : 1.0f;
    float b = flash ? 0.2f : 1.0f;
    glColor3f(r, g, b);
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

    // Centralna kropka
    glColor3f(1.0f, 0.25f, 0.25f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2i(cx, cy);
    glEnd();
    glPointSize(1.0f);

    // ---- Statystyki (prawa gora) ----
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

    // ---- Pasek czasu zycia celu ----
    if (targetAlive_ && !gameOver_) {
        float lifeLeft = 1.0f - (totalTime_ - targetSpawnTime_) / TARGET_LIFETIME;
        if (lifeLeft < 0.0f) lifeLeft = 0.0f;

        int barX = cx - 80;
        int barY = 36;
        int barW = 160;
        int barH = 10;

        glColor3f(0.15f, 0.15f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2i(barX-1,        barY-1);
        glVertex2i(barX+barW+1,   barY-1);
        glVertex2i(barX+barW+1,   barY+barH+1);
        glVertex2i(barX-1,        barY+barH+1);
        glEnd();

        float fillR = (lifeLeft < 0.5f) ? 1.0f : (1.0f - lifeLeft) * 2.0f;
        float fillG = (lifeLeft > 0.5f) ? 1.0f : lifeLeft * 2.0f;
        glColor3f(fillR, fillG, 0.10f);
        glBegin(GL_QUADS);
        glVertex2i(barX,                          barY);
        glVertex2i(barX + (int)(barW * lifeLeft), barY);
        glVertex2i(barX + (int)(barW * lifeLeft), barY + barH);
        glVertex2i(barX,                          barY + barH);
        glEnd();
    }

    // ---- Naglowek i sterowanie (lewy dol) ----
    glColor3f(0.75f, 0.85f, 1.00f);
    engine_.drawString(10, 60, "STRZELNICA 3D");
    glColor3f(0.55f, 0.65f, 0.85f);
    engine_.drawString(10, 42, "WASD = ruch  |  mysz = celowanie  |  SPACJA = strzal  |  R = reset");

    // ---- Ekran "Game Over" ----
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
