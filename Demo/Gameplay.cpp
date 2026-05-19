// =============================================================================
// Gameplay.cpp - Osoba 4
// Integracja silnika z gra: konstruktor (setup swiatel, kamery, sceny),
// petla update (ruch gracza, animacje, timery), strzelanie z raycastem,
// reset gry, oraz pelny HUD (statystyki, celownik, pasek czasu, ekran pauzy
// i ekran Game Over).
// =============================================================================

#include "ShootingGallery.h"
#include "Demo/Constants.h"
#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdio>
#include <string>

using namespace sg;

// =====================================================================
// Konstruktor: tworzy obiekty sceny, ustawia oswietlenie, konfiguruje
// silnik dla rozgrywki (FP kamera, free mouse look, callbacki).
// =====================================================================
ShootingGallery::ShootingGallery(Engine& engine)
    : engine_(engine),
      playerEye_(0.0f, PLAYER_HEIGHT, 7.0f),
      playerEyeHome_(0.0f, PLAYER_HEIGHT, 7.0f),
      floor_    (std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_LENGTH)),
      ceiling_  (std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_LENGTH)),
      backWall_ (std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_HEIGHT)),
      frontWall_(std::make_shared<PlaneNode>(ROOM_WIDTH,  ROOM_HEIGHT)),
      leftWall_ (std::make_shared<PlaneNode>(ROOM_HEIGHT, ROOM_LENGTH)),
      rightWall_(std::make_shared<PlaneNode>(ROOM_HEIGHT, ROOM_LENGTH)),
      secondLight_(std::make_shared<PointLight>()),
      waveRespawnTimer_(0.0f), waveSize_(0),
      floorTex_     (std::make_shared<Texture>()),
      ceilingTex_   (std::make_shared<Texture>()),
      wallTex_      (std::make_shared<Texture>()),
      pillarTex_    (std::make_shared<Texture>()),
      coneTex_      (std::make_shared<Texture>()),
      boxTex_       (std::make_shared<Texture>()),
      targetTex_    (std::make_shared<Texture>()),
      decorationTex_(std::make_shared<Texture>()),
      torusTex_     (std::make_shared<Texture>()),
      score_(0), streak_(0), bestStreak_(0),
      lives_(START_LIVES), totalTime_(0.0f),
      gameOver_(false),
      hitFlashTime_(0.0f), missFlashTime_(0.0f),
      crosshairFlash_(0.0f), bonusFlashTime_(0.0f),
      rng_((unsigned)std::chrono::steady_clock::now().time_since_epoch().count())
{
    // ---- Globalny ambient (silny, zeby ciemne katy nie byly czarne) ----
    engine_.setGlobalAmbient(0.35f, 0.35f, 0.38f);

    // ---- Tekstura celu: probujemy z pliku BMP, fallback do szachownicy ----
    if (!targetTex_->loadBMP("assets/textures/target.bmp")) {
        targetTex_->generateCheckerboard(128,
            1.00f, 0.30f, 0.10f,
            1.00f, 0.95f, 0.50f, 6);
    }

    // ---- Pierwsza lampa (z silnika) - frontowa strefa ----
    auto& l0 = *engine_.getPointLight();
    l0.setPosition(Vec3(0.0f, ROOM_HEIGHT - 0.5f, -2.0f));
    l0.setAmbient(Vec3(0.18f, 0.18f, 0.18f));
    l0.setDiffuse(Vec3(1.05f, 0.95f, 0.85f));
    l0.setSpecular(Vec3(1.0f, 1.0f, 1.0f));
    l0.setAttenuation(1.0f, 0.005f, 0.0005f);

    // ---- Druga lampa - tylna strefa, lekko chlodna ----
    secondLight_->setLightIndex(1);
    secondLight_->setPosition(Vec3(0.0f, ROOM_HEIGHT - 0.5f, -15.0f));
    secondLight_->setAmbient(Vec3(0.15f, 0.15f, 0.18f));
    secondLight_->setDiffuse(Vec3(0.85f, 0.92f, 1.05f));
    secondLight_->setSpecular(Vec3(0.9f, 0.95f, 1.0f));
    secondLight_->setAttenuation(1.0f, 0.005f, 0.0005f);
    // Dodajemy druga lampe PRZED scianami/przeszkodami zeby renderer ustawil
    // obie pozycje swiatla zanim zacznie rysowac geometrie.
    engine_.getSceneRoot()->addChild(secondLight_);

    // Budowa sceny - z 4 plikow (Room/Obstacles/Targets).
    buildRoom();
    buildObstacles();
    buildDecorations();
    initTargetPool();

    // ---- Konfiguracja silnika dla gry ----
    engine_.setCameraControl(CameraControlMode::GAME_CONTROLLED);
    engine_.setCameraYawPitch(0.0f, 0.0f);
    engine_.setMouseSensitivity(0.0030f);
    engine_.setFreeMouseLook(true);
    engine_.getCamera()->setFirstPerson(playerEye_, 0.0f, 0.0f);

    spawnWave();
}

void ShootingGallery::resetGame() {
    score_ = streak_ = 0;
    lives_ = START_LIVES;
    totalTime_ = 0.0f;
    gameOver_ = false;
    hitFlashTime_ = missFlashTime_ = crosshairFlash_ = bonusFlashTime_ = 0.0f;
    playerEye_ = playerEyeHome_;
    engine_.setCameraYawPitch(0.0f, 0.0f);
    spawnWave();
}

// =====================================================================
// Petla update wywolana co klatke. Aktualizuje: ruch gracza WASD,
// kolizje, kamere FP, timery flashy, animacje celow, wygasanie celow.
// =====================================================================
void ShootingGallery::onUpdate(float dt) {
    if (!gameOver_) {
        float yaw = engine_.getCameraYaw();
        Vec3 fwd(std::sin(yaw), 0.0f, -std::cos(yaw));
        Vec3 rgt(std::cos(yaw), 0.0f,  std::sin(yaw));
        float speed = PLAYER_MOVE_SPEED * dt;
        if (engine_.isKeyDown('w') || engine_.isKeyDown('W')) playerEye_ += fwd * speed;
        if (engine_.isKeyDown('s') || engine_.isKeyDown('S')) playerEye_ -= fwd * speed;
        if (engine_.isKeyDown('a') || engine_.isKeyDown('A')) playerEye_ -= rgt * speed;
        if (engine_.isKeyDown('d') || engine_.isKeyDown('D')) playerEye_ += rgt * speed;

        applyObstacleCollision();

        playerEye_.x = std::max(-ROOM_X_HALF + WALL_BUFFER,
                       std::min( ROOM_X_HALF - WALL_BUFFER, playerEye_.x));
        playerEye_.z = std::max( ROOM_Z_MIN  + WALL_BUFFER,
                       std::min( ROOM_Z_MAX  - WALL_BUFFER, playerEye_.z));
        playerEye_.y = PLAYER_HEIGHT;
    }

    engine_.getCamera()->setFirstPerson(
        playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());

    if (gameOver_) return;

    totalTime_     += dt;
    hitFlashTime_   = std::max(0.0f, hitFlashTime_   - dt);
    missFlashTime_  = std::max(0.0f, missFlashTime_  - dt);
    crosshairFlash_ = std::max(0.0f, crosshairFlash_ - dt);
    bonusFlashTime_ = std::max(0.0f, bonusFlashTime_ - dt);

    // Respawn fali po jej wyczyszczeniu/wygasnieciu.
    if (targets_.empty()) {
        if (waveRespawnTimer_ > 0.0f) {
            waveRespawnTimer_ -= dt;
            if (waveRespawnTimer_ <= 0.0f) spawnWave();
        } else {
            waveRespawnTimer_ = WAVE_RESPAWN;
        }
        return;
    }

    // Animacja celow + wygasanie czasu.
    for (size_t i = 0; i < targets_.size(); ) {
        Target& t = targets_[i];
        t.bobPhase  += dt;
        t.spinAngle += dt * 1.4f;

        auto& node = targetNodePool_[t.nodeIndex];
        node->setPosition(currentTargetPos(t));
        node->setRotation(Vec3(0, t.spinAngle, 0));

        if (totalTime_ - t.spawnTime > TARGET_LIFETIME) {
            --lives_;
            streak_ = 0;
            missFlashTime_ = FLASH_DURATION;
            node->setVisible(false);
            targets_.erase(targets_.begin() + i);
            if (lives_ <= 0) {
                gameOver_ = true;
                for (auto& n : targetNodePool_) n->setVisible(false);
                targets_.clear();
                return;
            }
            continue;
        }
        ++i;
    }
}

// =====================================================================
// Obsluga strzalu (SPACJA / LPM). Raycast z kamery, znajduje najblizszy
// cel z tolerancja, sprawdza czy przeszkoda blokuje, przyznaje punkty
// lub odbiera zycie. Bonus +2s dla pozostalych celow w fali.
// =====================================================================
void ShootingGallery::onShoot() {
    if (gameOver_ || targets_.empty()) return;

    // Odswiez kamere najnowszym yaw/pitch (mysz mogla sie poruszyc miedzy klatkami).
    engine_.getCamera()->setFirstPerson(
        playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());

    Vec3 origin = engine_.getCamera()->eyePosition();
    Vec3 dir    = engine_.getCamera()->lookDirection();

    crosshairFlash_ = 0.20f;

    // Znajdz najblizszy trafiony cel.
    float closestT = MAX_SHOOT_DIST;
    int   hitIdx = -1;
    for (size_t i = 0; i < targets_.size(); ++i) {
        Vec3 pos = currentTargetPos(targets_[i]);
        float t;
        if (raySphereIntersect(origin, dir, pos,
                               TARGET_RADIUS * HIT_TOLERANCE, t)
            && t > 0.0f && t < closestT) {
            closestT = t;
            hitIdx = (int)i;
        }
    }

    // Czy przeszkoda blokuje promien przed celem?
    if (hitIdx >= 0 && obstacleBlocksRay(origin, dir, closestT)) {
        hitIdx = -1;
    }

    if (hitIdx >= 0) {
        ++score_;
        ++streak_;
        if (streak_ > bestStreak_) bestStreak_ = streak_;
        if (streak_ % 5 == 0 && lives_ < 9) ++lives_;
        hitFlashTime_ = FLASH_DURATION;
        targetNodePool_[targets_[hitIdx].nodeIndex]->setVisible(false);
        targets_.erase(targets_.begin() + hitIdx);

        // BONUS: jezeli zostaly cele w fali, daj kazdemu +WAVE_HIT_BONUS sek.
        if (!targets_.empty()) {
            for (auto& remaining : targets_) {
                remaining.spawnTime += WAVE_HIT_BONUS;
            }
            bonusFlashTime_ = 1.5f;
        }
    } else {
        --lives_;
        streak_ = 0;
        missFlashTime_ = FLASH_DURATION;
        if (lives_ <= 0) {
            gameOver_ = true;
            for (auto& n : targetNodePool_) n->setVisible(false);
            targets_.clear();
        }
    }
}

void ShootingGallery::onReset() {
    if (gameOver_) resetGame();
}

// =====================================================================
// HUD: nakladki ekranowe (winieta hit/miss), celownik, statystyki gry,
// pasek czasu celu, wskaznik bonusu, ekran Game Over, ekran pauzy.
// Rysowane w 2D (engine konfiguruje ortho przed wywolaniem).
// =====================================================================
void ShootingGallery::onHUD() {
    int W = engine_.windowW();
    int H = engine_.windowH();
    int cx = W / 2;
    int cy = H / 2;

    // Czerwony blysk po pudle
    if (missFlashTime_ > 0.0f) {
        float a = missFlashTime_ / FLASH_DURATION;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.10f, 0.10f, 0.30f * a);
        glBegin(GL_QUADS);
        glVertex2i(0,0); glVertex2i(W,0); glVertex2i(W,H); glVertex2i(0,H);
        glEnd();
        glDisable(GL_BLEND);
    }
    // Zielony przyblysk po trafieniu
    if (hitFlashTime_ > 0.0f) {
        float a = hitFlashTime_ / FLASH_DURATION;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.20f, 1.00f, 0.30f, 0.18f * a);
        glBegin(GL_QUADS);
        glVertex2i(0,0); glVertex2i(W,0); glVertex2i(W,H); glVertex2i(0,H);
        glEnd();
        glDisable(GL_BLEND);
    }

    // Celownik (krzyzyk z odstepem + kropka srodek)
    bool flash = crosshairFlash_ > 0.0f;
    glColor3f(flash ? 1.0f : 1.0f, flash ? 0.8f : 1.0f, flash ? 0.2f : 1.0f);
    glLineWidth(flash ? 3.0f : 2.0f);
    int gap = flash ? 8 : 5, len = flash ? 18 : 12;
    glBegin(GL_LINES);
    glVertex2i(cx-gap-len, cy); glVertex2i(cx-gap, cy);
    glVertex2i(cx+gap,     cy); glVertex2i(cx+gap+len, cy);
    glVertex2i(cx, cy-gap-len); glVertex2i(cx, cy-gap);
    glVertex2i(cx, cy+gap);     glVertex2i(cx, cy+gap+len);
    glEnd();
    glLineWidth(1.0f);
    glColor3f(1.0f, 0.25f, 0.25f);
    glPointSize(3.0f);
    glBegin(GL_POINTS); glVertex2i(cx, cy); glEnd();
    glPointSize(1.0f);

    // Statystyki (prawa gora)
    int rx = W - 230;
    int ry = H - 22;

    int alive = (int)targets_.size();
    glColor3f(1.00f, 0.80f, 0.40f);
    engine_.drawStringLarge(rx, ry, "CELE: " + std::to_string(alive) + " / " + std::to_string(waveSize_));
    ry -= 28;
    glColor3f(0.55f, 1.00f, 0.55f);
    engine_.drawStringLarge(rx, ry, "WYNIK: " + std::to_string(score_));
    ry -= 28;
    glColor3f(1.00f, 0.55f, 0.55f);
    engine_.drawStringLarge(rx, ry, "ZYCIA: " + std::to_string(lives_));
    ry -= 26;
    glColor3f(0.85f, 0.85f, 0.45f);
    int sec = (int)totalTime_;
    int csec = (int)((totalTime_ - sec) * 100);
    char buf[64];
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

    // Pasek czasu (najkrotsza zycie ze wszystkich celow w fali)
    if (!targets_.empty() && !gameOver_) {
        float minLifeLeft = 1.0f;
        for (const auto& t : targets_) {
            float ll = 1.0f - (totalTime_ - t.spawnTime) / TARGET_LIFETIME;
            if (ll < minLifeLeft) minLifeLeft = ll;
        }
        if (minLifeLeft < 0.0f) minLifeLeft = 0.0f;
        int barX = cx - 80, barY = 36, barW = 160, barH = 10;
        glColor3f(0.15f,0.15f,0.15f);
        glBegin(GL_QUADS);
        glVertex2i(barX-1,barY-1); glVertex2i(barX+barW+1,barY-1);
        glVertex2i(barX+barW+1,barY+barH+1); glVertex2i(barX-1,barY+barH+1);
        glEnd();
        float fr = (minLifeLeft < 0.5f) ? 1.0f : (1.0f - minLifeLeft) * 2.0f;
        float fg = (minLifeLeft > 0.5f) ? 1.0f : minLifeLeft * 2.0f;
        glColor3f(fr, fg, 0.10f);
        glBegin(GL_QUADS);
        glVertex2i(barX,barY); glVertex2i(barX+(int)(barW*minLifeLeft),barY);
        glVertex2i(barX+(int)(barW*minLifeLeft),barY+barH); glVertex2i(barX,barY+barH);
        glEnd();

        // Wskaznik bonusu czasowego
        if (bonusFlashTime_ > 0.0f) {
            float a = bonusFlashTime_ / 1.5f;
            glColor3f(0.30f, 1.00f, 0.40f * a + 0.40f);
            engine_.drawString(cx - 50, barY + barH + 6, "+2s do pozostalych celow!");
        }
    }

    // Naglowek
    glColor3f(0.75f, 0.85f, 1.00f);
    engine_.drawString(10, 60, "STRZELNICA 3D");
    glColor3f(0.55f, 0.65f, 0.85f);
    engine_.drawString(10, 42, "WASD = ruch  |  mysz = celowanie  |  SPACJA = strzal  |  R = reset");

    // Ekran Game Over
    if (gameOver_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0,0,0, 0.70f);
        glBegin(GL_QUADS);
        glVertex2i(0,0); glVertex2i(W,0); glVertex2i(W,H); glVertex2i(0,H);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(1.0f, 0.30f, 0.30f);
        engine_.drawStringLarge(cx - 100, cy + 50, "KONIEC GRY");
        glColor3f(1.0f, 1.0f, 1.0f);
        engine_.drawStringLarge(cx - 90, cy + 10, "Wynik: " + std::to_string(score_));
        glColor3f(0.85f, 0.85f, 0.50f);
        engine_.drawString(cx - 100, cy - 20, "Najlepsza seria: " + std::to_string(bestStreak_));
        glColor3f(0.80f, 0.80f, 0.40f);
        engine_.drawString(cx - 115, cy - 50, "Nacisnij R aby zagrac ponownie");
    } else if (engine_.isPaused()) {
        // Ekran pauzy
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
        glBegin(GL_QUADS);
        glVertex2i(0,0); glVertex2i(W,0); glVertex2i(W,H); glVertex2i(0,H);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.85f, 0.45f);
        engine_.drawStringLarge(cx - 50, cy + 30, "PAUZA");
        glColor3f(0.90f, 0.90f, 0.90f);
        engine_.drawString(cx - 80, cy - 5,  "ESC = wznow gre");
        engine_.drawString(cx - 80, cy - 25, "Q   = wyjscie z gry");
    }
}
