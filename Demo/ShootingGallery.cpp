#include "ShootingGallery.h"
#include <GL/freeglut.h>
#include <cmath>
#include <chrono>
#include <string>

namespace {
    const float TARGET_RADIUS    = 0.45f;
    const float TARGET_LIFETIME  = 5.0f;
    const int   START_LIVES      = 5;
    const float FLASH_DURATION   = 0.35f;
    const float MAX_SHOOT_DIST   = 100.0f;
}

ShootingGallery::ShootingGallery(Engine& engine)
    : engine_(engine),
      playerEye_(0.0f, 1.6f, 6.0f),
      ground_(std::make_shared<PlaneNode>(40.0f, 40.0f)),
      backWall_(std::make_shared<PlaneNode>(30.0f, 12.0f)),
      target_(std::make_shared<SphereNode>(TARGET_RADIUS, 18, 32)),
      groundTex_(std::make_shared<Texture>()),
      wallTex_(std::make_shared<Texture>()),
      targetTex_(std::make_shared<Texture>()),
      targetPos_(0.0f, 1.5f, -6.0f),
      targetRadius_(TARGET_RADIUS),
      targetSpawnTime_(0.0f),
      targetBobPhase_(0.0f),
      targetSpinAngle_(0.0f),
      targetAlive_(true),
      score_(0), lives_(START_LIVES), totalTime_(0.0f),
      gameOver_(false),
      hitFlashTime_(0.0f), missFlashTime_(0.0f),
      rng_((unsigned)std::chrono::steady_clock::now().time_since_epoch().count())
{
    // ---- Tekstury proceduralne (wymagaja kontekstu GL - dlatego po setGraphicsParams) ----
    groundTex_->generateCheckerboard(256,
        0.45f, 0.45f, 0.45f,
        0.20f, 0.20f, 0.22f,
        8);
    wallTex_->generateStripes(256,
        0.30f, 0.20f, 0.15f,
        0.45f, 0.30f, 0.20f,
        16);
    targetTex_->generateCheckerboard(128,
        1.0f, 0.30f, 0.10f,
        1.0f, 0.95f, 0.50f,
        6);

    // ---- Podloga ----
    ground_->setPosition(Vec3(0.0f, 0.0f, 0.0f));
    ground_->setMaterial(Material(
        Vec3(0.10f, 0.10f, 0.10f),
        Vec3(0.55f, 0.55f, 0.55f),
        Vec3(0.05f, 0.05f, 0.05f), 4.0f));
    ground_->setTexture(groundTex_);

    // ---- Tylna sciana (obrocona pionowo) ----
    // PlaneNode jest w plaszczyznie XZ. Obrocimy o 90 stopni wokol osi X aby stalo pionowo.
    backWall_->setPosition(Vec3(0.0f, 6.0f, -18.0f));
    backWall_->setRotation(Vec3(1.5707963f, 0.0f, 0.0f)); // 90 stopni wokol X
    backWall_->setMaterial(Material(
        Vec3(0.10f, 0.07f, 0.05f),
        Vec3(0.65f, 0.50f, 0.40f),
        Vec3(0.05f, 0.05f, 0.05f), 8.0f));
    backWall_->setTexture(wallTex_);

    // ---- Cel ----
    target_->setMaterial(Material(
        Vec3(0.20f, 0.10f, 0.02f),
        Vec3(0.95f, 0.55f, 0.10f),
        Vec3(0.95f, 0.95f, 0.95f), 32.0f));
    target_->setTexture(targetTex_);
    target_->setPosition(targetPos_);

    // ---- Swiatlo do oswietlenia areny ----
    engine_.getPointLight()->setPosition(Vec3(0.0f, 8.0f, 0.0f));
    engine_.getPointLight()->setAttenuation(1.0f, 0.020f, 0.0030f);

    // ---- Dodanie do grafu sceny ----
    auto root = engine_.getSceneRoot();
    root->addChild(ground_);
    root->addChild(backWall_);
    root->addChild(target_);

    // ---- Konfiguracja kamery: pierwszoosobowa, gra ja kontroluje ----
    engine_.setCameraControl(CameraControlMode::GAME_CONTROLLED);
    engine_.setCameraYawPitch(0.0f, 0.0f);
    engine_.getCamera()->setFirstPerson(playerEye_, 0.0f, 0.0f);

    spawnTarget();
}

void ShootingGallery::spawnTarget() {
    std::uniform_real_distribution<float> distX(-7.0f, 7.0f);
    std::uniform_real_distribution<float> distY( 1.0f, 4.0f);
    std::uniform_real_distribution<float> distZ(-14.0f, -4.0f);

    targetPos_ = Vec3(distX(rng_), distY(rng_), distZ(rng_));
    target_->setPosition(targetPos_);
    targetSpawnTime_ = totalTime_;
    targetBobPhase_  = 0.0f;
    targetAlive_     = true;
}

void ShootingGallery::resetGame() {
    score_         = 0;
    lives_         = START_LIVES;
    totalTime_     = 0.0f;
    gameOver_      = false;
    hitFlashTime_  = 0.0f;
    missFlashTime_ = 0.0f;
    spawnTarget();
}

void ShootingGallery::onUpdate(float dt) {
    if (gameOver_) {
        // Aktualizuj tylko kamere - reszta zamrozona az do R
        engine_.getCamera()->setFirstPerson(
            playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());
        return;
    }

    totalTime_     += dt;
    targetBobPhase_ += dt;
    targetSpinAngle_ += dt * 1.4f;  // obrot wokol osi Y
    hitFlashTime_   = std::max(0.0f, hitFlashTime_  - dt);
    missFlashTime_  = std::max(0.0f, missFlashTime_ - dt);

    if (targetAlive_) {
        // Animacja: lekkie kolysanie w pionie + obrot
        Vec3 displayPos = targetPos_;
        displayPos.y += 0.15f * std::sin(targetBobPhase_ * 2.5f);
        target_->setPosition(displayPos);
        target_->setRotation(Vec3(0.0f, targetSpinAngle_, 0.0f));

        // Kolor podczas blysku trafienia - chwilowo zielony przez zmiane materialu
        if (hitFlashTime_ > 0.0f) {
            float t = hitFlashTime_ / FLASH_DURATION;
            target_->setMaterial(Material(
                Vec3(0.05f, 0.20f, 0.05f),
                Vec3(0.10f + 0.85f * t, 1.0f, 0.10f + 0.85f * t),
                Vec3(1.0f, 1.0f, 1.0f), 64.0f));
        }

        // Wygasniecie celu
        if (totalTime_ - targetSpawnTime_ > TARGET_LIFETIME) {
            --lives_;
            missFlashTime_ = FLASH_DURATION;
            if (lives_ <= 0) {
                gameOver_ = true;
                targetAlive_ = false;
            } else {
                spawnTarget();
                // Przywroc material po blysku
                target_->setMaterial(Material(
                    Vec3(0.20f, 0.10f, 0.02f),
                    Vec3(0.95f, 0.55f, 0.10f),
                    Vec3(0.95f, 0.95f, 0.95f), 32.0f));
            }
        }
    }

    // Kamera pierwszoosobowa: stala pozycja, kierunek z yaw/pitch silnika
    engine_.getCamera()->setFirstPerson(
        playerEye_,
        engine_.getCameraYaw(),
        engine_.getCameraPitch());
}

void ShootingGallery::onShoot() {
    if (gameOver_ || !targetAlive_) return;

    Vec3 origin = engine_.getCamera()->eyePosition();
    Vec3 dir    = engine_.getCamera()->lookDirection();

    float t = 0.0f;
    // Pozycja celu z uwzglednieniem aktualnego kolysania
    Vec3 spherePos = targetPos_;
    spherePos.y   += 0.15f * std::sin(targetBobPhase_ * 2.5f);

    if (raySphereIntersect(origin, dir, spherePos, targetRadius_, t)
        && t < MAX_SHOOT_DIST) {
        // TRAFIENIE
        ++score_;
        hitFlashTime_ = FLASH_DURATION;
        targetAlive_ = false;

        // Krotki blysk - cel pozostaje widoczny, ale po momencie respawn
        // Robimy to przez ustawienie alive=false i respawn po czasie flash
        // Tu uproszczone: respawn natychmiast
        target_->setMaterial(Material(
            Vec3(0.05f, 0.20f, 0.05f),
            Vec3(0.20f, 1.0f, 0.20f),
            Vec3(1.0f, 1.0f, 1.0f), 64.0f));
        spawnTarget();
        // Po respawnie material przywracamy w onUpdate gdy hitFlashTime mija
    } else {
        // PUDLO
        --lives_;
        missFlashTime_ = FLASH_DURATION;
        if (lives_ <= 0) {
            gameOver_ = true;
        }
    }
}

void ShootingGallery::onReset() {
    if (gameOver_) {
        resetGame();
    }
}

void ShootingGallery::onHUD() {
    int W = engine_.windowW();
    int H = engine_.windowH();

    // ---- Czerwony blysk po pudle (przezroczyste tlo) ----
    if (missFlashTime_ > 0.0f) {
        float a = missFlashTime_ / FLASH_DURATION;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.1f, 0.1f, 0.25f * a);
        glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(W, 0);
        glVertex2i(W, H);
        glVertex2i(0, H);
        glEnd();
        glDisable(GL_BLEND);
    }

    // ---- Celownik (krzyzyk + okrag) ----
    int cx = W / 2;
    int cy = H / 2;
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2i(cx - 12, cy); glVertex2i(cx - 4, cy);
    glVertex2i(cx +  4, cy); glVertex2i(cx + 12, cy);
    glVertex2i(cx, cy - 12); glVertex2i(cx, cy - 4);
    glVertex2i(cx, cy +  4); glVertex2i(cx, cy + 12);
    glEnd();
    glLineWidth(1.0f);

    // Maly punkt w srodku
    glColor3f(1.0f, 0.2f, 0.2f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2i(cx, cy);
    glEnd();
    glPointSize(1.0f);

    // ---- Statystyki (prawa gora) ----
    int rx = W - 200;
    int ry = H - 22;

    glColor3f(0.4f, 1.0f, 0.4f);
    engine_.drawStringLarge(rx, ry, "WYNIK: " + std::to_string(score_));
    ry -= 28;

    glColor3f(1.0f, 0.4f, 0.4f);
    engine_.drawStringLarge(rx, ry, "ZYCIA: " + std::to_string(lives_));
    ry -= 26;

    glColor3f(0.8f, 0.8f, 0.4f);
    int   sec  = (int)totalTime_;
    int   csec = (int)((totalTime_ - sec) * 100);
    char  buf[64];
    std::snprintf(buf, sizeof(buf), "CZAS: %02d:%02d", sec, csec);
    engine_.drawString(rx, ry, buf);

    // Pasek czasu zycia celu (jesli zywy)
    if (targetAlive_ && !gameOver_) {
        float lifeLeft = 1.0f - (totalTime_ - targetSpawnTime_) / TARGET_LIFETIME;
        if (lifeLeft < 0.0f) lifeLeft = 0.0f;

        int barX = cx - 60;
        int barY = 32;
        int barW = 120;
        int barH = 8;

        // Tlo
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2i(barX,         barY);
        glVertex2i(barX + barW,  barY);
        glVertex2i(barX + barW,  barY + barH);
        glVertex2i(barX,         barY + barH);
        glEnd();

        // Wypelnienie (zielone -> czerwone zaleznie od czasu)
        float r = (lifeLeft < 0.5f) ? 1.0f : (1.0f - lifeLeft) * 2.0f;
        float g = (lifeLeft > 0.5f) ? 1.0f : lifeLeft * 2.0f;
        glColor3f(r, g, 0.1f);
        glBegin(GL_QUADS);
        glVertex2i(barX,                       barY);
        glVertex2i(barX + (int)(barW*lifeLeft), barY);
        glVertex2i(barX + (int)(barW*lifeLeft), barY + barH);
        glVertex2i(barX,                       barY + barH);
        glEnd();

        glColor3f(0.7f, 0.7f, 0.7f);
        engine_.drawString(cx - 30, barY + barH + 4, "czas celu");
    }

    // ---- Tytul ekranu ----
    glColor3f(0.7f, 0.8f, 1.0f);
    engine_.drawString(10, 40, "STRZELNICA 3D  -  Spacja=strzal, R=reset, LPM+mysz=celuj");

    // ---- Ekran "Game Over" ----
    if (gameOver_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
        glBegin(GL_QUADS);
        glVertex2i(0, 0); glVertex2i(W, 0);
        glVertex2i(W, H); glVertex2i(0, H);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.3f, 0.3f);
        engine_.drawStringLarge(cx - 90, cy + 30, "KONIEC GRY");

        glColor3f(1.0f, 1.0f, 1.0f);
        engine_.drawStringLarge(cx - 80, cy - 10, "Wynik: " + std::to_string(score_));

        glColor3f(0.8f, 0.8f, 0.4f);
        engine_.drawString(cx - 110, cy - 50, "Nacisnij R aby zagrac ponownie");
    }
}
