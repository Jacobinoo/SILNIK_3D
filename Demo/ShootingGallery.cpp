#include "ShootingGallery.h"
#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdio>
#include <string>

namespace {
    const float PI = 3.14159265358979323846f;

    // Wymiary pokoju
    const float ROOM_X_HALF = 10.0f;
    const float ROOM_HEIGHT =  6.0f;
    const float ROOM_Z_MIN  = -20.0f;
    const float ROOM_Z_MAX  =  10.0f;
    const float ROOM_LENGTH = ROOM_Z_MAX - ROOM_Z_MIN;
    const float ROOM_WIDTH  = 2.0f * ROOM_X_HALF;
    const float ROOM_Z_CENTER = (ROOM_Z_MIN + ROOM_Z_MAX) * 0.5f;
    const float WALL_BUFFER = 0.5f;

    // Strefa pojawiania celow - praktycznie caly pokoj
    const float SPAWN_X_HALF = 8.5f;
    const float SPAWN_Y_MIN  = 1.0f;
    const float SPAWN_Y_MAX  = 4.5f;
    const float SPAWN_Z_MIN  = -18.0f;
    const float SPAWN_Z_MAX  = 7.0f;
    const float MIN_SPAWN_DIST_FROM_PLAYER = 3.5f;
    const float MIN_SPAWN_DIST_FROM_OTHER  = 3.0f;

    const int   MAX_TARGETS      = 3;
    const float TARGET_RADIUS    = 0.50f;
    const float HIT_TOLERANCE    = 1.40f;
    const float TARGET_LIFETIME  = 3.0f;     // bylo 9.0 - skrocone o ok. 67%
    const float WAVE_RESPAWN     = 0.50f;
    const int   START_LIVES      = 2;        // bylo 5 - trudniej
    const float FLASH_DURATION   = 0.35f;
    const float MAX_SHOOT_DIST   = 100.0f;
    const float MAX_MOVE_RANGE   = 1.6f;     // bylo 1.4 - wieksza amplituda ruchu

    const float PLAYER_HEIGHT     = 1.75f;
    const float PLAYER_RADIUS     = 0.35f;
    const float PLAYER_MOVE_SPEED = 4.5f;
}

Material ShootingGallery::defaultTargetMaterial() const {
    return Material(
        Vec3(0.30f, 0.15f, 0.05f),
        Vec3(0.98f, 0.50f, 0.08f),
        Vec3(1.00f, 1.00f, 1.00f), 48.0f);
}

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
      floorTex_   (std::make_shared<Texture>()),
      ceilingTex_ (std::make_shared<Texture>()),
      wallTex_    (std::make_shared<Texture>()),
      pillarTex_  (std::make_shared<Texture>()),
      coneTex_      (std::make_shared<Texture>()),
      boxTex_       (std::make_shared<Texture>()),
      targetTex_    (std::make_shared<Texture>()),
      decorationTex_(std::make_shared<Texture>()),
      torusTex_     (std::make_shared<Texture>()),
      score_(0), streak_(0), bestStreak_(0),
      lives_(START_LIVES), totalTime_(0.0f),
      gameOver_(false),
      hitFlashTime_(0.0f), missFlashTime_(0.0f), crosshairFlash_(0.0f),
      rng_((unsigned)std::chrono::steady_clock::now().time_since_epoch().count())
{
    // ---- Globalny ambient (silny, zeby ciemne katy nie byly czarne) ----
    engine_.setGlobalAmbient(0.35f, 0.35f, 0.38f);

    // ---- Tekstura celu: probujemy z pliku BMP, fallback do szachownicy ----
    // Zob. assets/textures/README.md jak dodac plik.
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

    // ---- Druga lampa (nowa) - tylna strefa, lekko chlodna ----
    secondLight_->setLightIndex(1);
    secondLight_->setPosition(Vec3(0.0f, ROOM_HEIGHT - 0.5f, -15.0f));
    secondLight_->setAmbient(Vec3(0.15f, 0.15f, 0.18f));
    secondLight_->setDiffuse(Vec3(0.85f, 0.92f, 1.05f));
    secondLight_->setSpecular(Vec3(0.9f, 0.95f, 1.0f));
    secondLight_->setAttenuation(1.0f, 0.005f, 0.0005f);
    // Dodajemy druga lampe PRZED scianami/przeszkodami, zeby renderer
    // ustawil obie pozycje swiatla przed rysowaniem geometrii.
    engine_.getSceneRoot()->addChild(secondLight_);

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

void ShootingGallery::buildRoom() {
    // -----------------------------------------------------------------
    // DOSTEPNE GENERATORY TEKSTUR (klasa Texture w Engine/Texture.h):
    //   generateCheckerboard(size, r1,g1,b1, r2,g2,b2, tileCount)
    //   generateGradient(size, r1,g1,b1, r2,g2,b2, horizontal)
    //   generateStripes(size, r1,g1,b1, r2,g2,b2, stripeCount)
    //   generatePerlinNoise(size, r1,g1,b1, r2,g2,b2, octaves, scale)
    //   generateWood(size, ciemny RGB, jasny RGB, rings)
    //   generateBricks(size, cegla RGB, fuga RGB, rowsPerTexture)
    //   generateMarble(size, zyla RGB, kamien RGB, turbulence)
    //   loadBMP("sciezka.bmp")   -> z pliku 24-bit BMP
    // -----------------------------------------------------------------

    // Podloga: probujemy zaladowac plik BMP, fallback do proceduralnych ceglek.
    if (!floorTex_->loadBMP("assets/textures/floor.bmp")) {
        floorTex_->generateBricks(256, 0.55f,0.30f,0.20f, 0.25f,0.22f,0.20f, 6);
    }
    // Wspolna tekstura wszystkich 4 scian: probujemy z pliku BMP, fallback do drewna.
    if (!wallTex_->loadBMP("assets/textures/wall.bmp")) {
        wallTex_->generateWood(256, 0.25f,0.15f,0.08f, 0.65f,0.45f,0.25f, 5);
    }
    ceilingTex_->generatePerlinNoise(256, 0.12f,0.12f,0.18f, 0.30f,0.30f,0.38f, 4, 5.0f);

    // Material z mocniejszym ambientem - lepiej widoczne w przyciemnionych obszarach
    Material wallMat   (Vec3(0.30f,0.25f,0.20f), Vec3(0.75f,0.65f,0.55f), Vec3(0.10f,0.10f,0.10f),  8.0f);
    Material floorMat  (Vec3(0.30f,0.30f,0.32f), Vec3(0.75f,0.75f,0.80f), Vec3(0.10f,0.10f,0.10f),  4.0f);
    Material ceilingMat(Vec3(0.18f,0.18f,0.22f), Vec3(0.45f,0.45f,0.50f), Vec3(0.05f,0.05f,0.05f),  4.0f);
    Material backMat   (Vec3(0.30f,0.20f,0.15f), Vec3(0.80f,0.55f,0.40f), Vec3(0.15f,0.10f,0.10f), 12.0f);

    // Skala UV - kontroluje jak duze sa pojedyncze powtorzenia tekstury.
    // Mniejsza wartosc = wieksze tile. Dla scian 30x6m wartosc 0.20 daje ok. 6 tile.
    // Mozesz zmienic ponizsze stale aby uzyskac inny efekt.
    const float FLOOR_UV_SCALE = 0.30f;   // podloga / sufit: tile co ~3.3m
    const float WALL_UV_SCALE  = 0.20f;   // sciany: tile co ~5m

    floor_->setPosition(Vec3(0, 0, ROOM_Z_CENTER));
    floor_->setUVScale(FLOOR_UV_SCALE);
    floor_->setMaterial(floorMat); floor_->setTexture(floorTex_);

    ceiling_->setPosition(Vec3(0, ROOM_HEIGHT, ROOM_Z_CENTER));
    ceiling_->setRotation(Vec3(PI, 0, 0));
    ceiling_->setUVScale(FLOOR_UV_SCALE);
    ceiling_->setMaterial(ceilingMat); ceiling_->setTexture(ceilingTex_);

    backWall_->setPosition(Vec3(0, ROOM_HEIGHT*0.5f, ROOM_Z_MIN));
    backWall_->setRotation(Vec3(0.5f*PI, 0, 0));
    backWall_->setUVScale(WALL_UV_SCALE);
    backWall_->setMaterial(backMat); backWall_->setTexture(wallTex_);

    frontWall_->setPosition(Vec3(0, ROOM_HEIGHT*0.5f, ROOM_Z_MAX));
    frontWall_->setRotation(Vec3(-0.5f*PI, 0, 0));
    frontWall_->setUVScale(WALL_UV_SCALE);
    frontWall_->setMaterial(wallMat); frontWall_->setTexture(wallTex_);

    leftWall_->setPosition(Vec3(-ROOM_X_HALF, ROOM_HEIGHT*0.5f, ROOM_Z_CENTER));
    leftWall_->setRotation(Vec3(0, 0, -0.5f*PI));
    leftWall_->setUVScale(WALL_UV_SCALE);
    leftWall_->setMaterial(wallMat); leftWall_->setTexture(wallTex_);

    rightWall_->setPosition(Vec3(ROOM_X_HALF, ROOM_HEIGHT*0.5f, ROOM_Z_CENTER));
    rightWall_->setRotation(Vec3(0, 0, 0.5f*PI));
    rightWall_->setUVScale(WALL_UV_SCALE);
    rightWall_->setMaterial(wallMat); rightWall_->setTexture(wallTex_);

    auto root = engine_.getSceneRoot();
    root->addChild(floor_);
    root->addChild(ceiling_);
    root->addChild(backWall_);
    root->addChild(frontWall_);
    root->addChild(leftWall_);
    root->addChild(rightWall_);
}

void ShootingGallery::buildObstacles() {
    pillarTex_->generateMarble    (256, 0.30f,0.30f,0.35f, 0.85f,0.85f,0.90f, 3.0f);

    // Stozki: probujemy z pliku BMP, fallback do paskow.
    if (!coneTex_->loadBMP("assets/textures/cone.bmp")) {
        coneTex_->generateStripes(128, 1.00f,0.55f,0.10f, 1.00f,0.95f,0.95f, 6);
    }
    // Szescian-skrzynie: probujemy z pliku BMP, fallback do proceduralnego drewna.
    if (!boxTex_->loadBMP("assets/textures/box.bmp")) {
        boxTex_->generateWood(256, 0.30f,0.18f,0.08f, 0.70f,0.50f,0.25f, 6);
    }

    Material pillarMat(Vec3(0.30f,0.30f,0.32f), Vec3(0.65f,0.65f,0.70f), Vec3(0.30f,0.30f,0.30f), 24.0f);
    Material coneMat  (Vec3(0.40f,0.20f,0.05f), Vec3(0.95f,0.55f,0.15f), Vec3(0.50f,0.50f,0.50f), 16.0f);
    Material boxMat   (Vec3(0.30f,0.20f,0.10f), Vec3(0.70f,0.50f,0.30f), Vec3(0.10f,0.10f,0.10f),  6.0f);

    // ---- Cylindryczne kolumny rozsiane po pokoju ----
    struct CylDef { Vec3 base; float r, h; };
    std::vector<CylDef> cyls = {
        // Tylna polowa
        { Vec3(-4.0f, 0.0f,  -7.5f), 0.55f, 4.0f },
        { Vec3( 4.5f, 0.0f, -10.5f), 0.55f, 4.0f },
        { Vec3( 0.0f, 0.0f, -14.0f), 0.65f, 4.5f },
        { Vec3(-6.5f, 0.0f, -16.0f), 0.50f, 3.5f },
        // Frontowa polowa (NOWE)
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
        { Vec3( 6.0f, 0.0f,  -7.0f), 0.85f, 3.2f },  // mid-right
        { Vec3(-5.5f, 0.0f,   4.0f), 0.70f, 3.0f },  // front-left (NOWY)
        { Vec3( 2.5f, 0.0f,  -2.0f), 0.60f, 2.5f },  // front-middle (NOWY)
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
        { Vec3( 5.5f, 1.0f, -15.5f), 1.6f, 2.0f, 1.6f },  // back-right
        { Vec3(-7.0f, 1.0f,   1.0f), 1.8f, 2.0f, 1.5f },  // front-left (NOWY)
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

void ShootingGallery::buildDecorations() {
    // Tekstury dekoracji
    decorationTex_->generateGradient(128,
        0.95f, 0.80f, 0.35f,
        0.65f, 0.50f, 0.20f, false);
    torusTex_->generateCheckerboard(128,
        0.85f, 0.65f, 0.30f,
        0.55f, 0.35f, 0.15f, 8);

    Material decMat(
        Vec3(0.40f, 0.30f, 0.15f),
        Vec3(0.90f, 0.70f, 0.30f),
        Vec3(0.85f, 0.75f, 0.50f), 48.0f);

    auto root = engine_.getSceneRoot();

    auto makeSphere = [&](Vec3 pos, float r) {
        auto s = std::make_shared<SphereNode>(r, 16, 24);
        s->setPosition(pos);
        s->setMaterial(decMat);
        s->setTexture(decorationTex_);
        root->addChild(s);
    };

    auto makeCube = [&](Vec3 pos, Vec3 size) {
        auto c = std::make_shared<CubeNode>(1.0f);
        c->setPosition(pos);
        c->setScale(size);
        c->setMaterial(decMat);
        c->setTexture(decorationTex_);
        root->addChild(c);
    };

    auto makeCylinder = [&](Vec3 pos, float r, float h) {
        auto c = std::make_shared<CylinderNode>(r, h, 16);
        c->setPosition(pos);
        c->setMaterial(decMat);
        c->setTexture(decorationTex_);
        root->addChild(c);
    };

    // ---- Tylna sciana: 3 kule wysoko + 1 wezsza kolumna w srodku ----
    makeSphere(Vec3(-5.0f, 4.5f, -19.4f), 0.45f);
    makeSphere(Vec3( 0.0f, 5.0f, -19.4f), 0.55f);
    makeSphere(Vec3( 5.0f, 4.5f, -19.4f), 0.45f);

    // ---- Frontowa sciana: 2 kostki dekoracyjne wysoko ----
    makeCube(Vec3(-5.0f, 4.0f,  9.3f), Vec3(0.8f, 0.8f, 0.4f));
    makeCube(Vec3( 5.0f, 4.0f,  9.3f), Vec3(0.8f, 0.8f, 0.4f));

    // ---- Lewa sciana: 3 kule na sredniej wysokosci ----
    makeSphere(Vec3(-9.4f, 3.0f,  3.0f), 0.40f);
    makeSphere(Vec3(-9.4f, 3.0f, -6.0f), 0.40f);
    makeSphere(Vec3(-9.4f, 3.0f, -14.0f), 0.40f);

    // ---- Prawa sciana: 3 kule analogicznie ----
    makeSphere(Vec3( 9.4f, 3.0f,  3.0f), 0.40f);
    makeSphere(Vec3( 9.4f, 3.0f, -6.0f), 0.40f);
    makeSphere(Vec3( 9.4f, 3.0f, -14.0f), 0.40f);

    // ---- Sufit: torus jako centralny zyrandol + 2 male walce wiszace (lampy) ----
    {
        auto t = std::make_shared<TorusNode>(0.7f, 0.18f, 32, 14);
        t->setPosition(Vec3(0.0f, 5.3f, -5.0f));
        t->setMaterial(decMat);
        t->setTexture(torusTex_);
        root->addChild(t);
    }
    // 4 wiszace lampy w rogach
    makeCylinder(Vec3(-5.0f, 5.3f,  3.0f), 0.18f, 0.6f);
    makeCylinder(Vec3( 5.0f, 5.3f,  3.0f), 0.18f, 0.6f);
    makeCylinder(Vec3(-5.0f, 5.3f, -13.0f), 0.18f, 0.6f);
    makeCylinder(Vec3( 5.0f, 5.3f, -13.0f), 0.18f, 0.6f);

    // ---- Maly stozek-pinkle nad drzwiami frontowymi ----
    {
        auto k = std::make_shared<ConeNode>(0.30f, 0.50f, 16);
        k->setPosition(Vec3(0.0f, 5.5f, 9.0f));
        // Apex w dol - rotacja 180 wokol X
        k->setRotation(Vec3(3.14159265f, 0.0f, 0.0f));
        k->setMaterial(decMat);
        k->setTexture(decorationTex_);
        root->addChild(k);
    }
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

Vec3 ShootingGallery::currentTargetPos(const Target& t) const {
    Vec3 pos = t.basePos;
    if (t.moveRange > 0.01f) {
        pos += t.moveAxis * (t.moveRange * std::sin(totalTime_ * t.moveSpeed + t.movePhase));
    }
    pos.y += 0.18f * std::sin(t.bobPhase * 2.2f);
    return pos;
}

bool ShootingGallery::isInsideObstacle(const Vec3& pos, float margin) const {
    // Cylindry
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
    // Stozki - promien zalezy od wysokosci nad podstawa
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
    // Skrzynie (AABB rozszerzony o margin)
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
    // Cylindry - kolizja okragla
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
    // Stozki - traktujemy jak cylinder o promieniu podstawy (gracz nigdy nie wejdzie pod nawis)
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
    // Skrzynie - znajdujemy punkt najblizszy na AABB, wypychamy o promien gracza
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

void ShootingGallery::spawnWave() {
    std::uniform_real_distribution<float> distR(0.0f, 1.0f);
    float r = distR(rng_);
    waveSize_ = (r < 0.50f) ? 1 : (r < 0.85f) ? 2 : 3;

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

        float mr = distR(rng_);
        if (mr < 0.30f) {
            // Statyczny - rzadziej (bylo 50%)
            t.moveAxis = Vec3(0,0,0); t.moveRange = 0; t.moveSpeed = 0;
        } else if (mr < 0.70f) {
            // Ruch po osi X - prawie 2x szybszy niz wczesniej
            t.moveAxis = Vec3(1,0,0);
            t.moveRange = MAX_MOVE_RANGE;
            t.moveSpeed = 1.7f + distR(rng_) * 1.1f;  // 1.7..2.8 (bylo 0.9..1.5)
        } else {
            // Ruch po osi Z - szybszy
            t.moveAxis = Vec3(0,0,1);
            t.moveRange = MAX_MOVE_RANGE * 0.8f;
            t.moveSpeed = 1.7f + distR(rng_) * 1.2f;  // 1.7..2.9 (bylo 0.9..1.6)
        }

        bool found = false;
        for (int attempt = 0; attempt < 60; ++attempt) {
            Vec3 candidate(distX(rng_), distY(rng_), distZ(rng_));
            if (length(candidate - playerEye_) < MIN_SPAWN_DIST_FROM_PLAYER) continue;

            // Margines obejmuje promien celu, pelen zakres ruchu i rezerwe
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

void ShootingGallery::resetGame() {
    score_ = streak_ = 0;
    lives_ = START_LIVES;
    totalTime_ = 0.0f;
    gameOver_ = false;
    hitFlashTime_ = missFlashTime_ = crosshairFlash_ = 0.0f;
    playerEye_ = playerEyeHome_;
    engine_.setCameraYawPitch(0.0f, 0.0f);
    spawnWave();
}

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

    if (targets_.empty()) {
        if (waveRespawnTimer_ > 0.0f) {
            waveRespawnTimer_ -= dt;
            if (waveRespawnTimer_ <= 0.0f) spawnWave();
        } else {
            waveRespawnTimer_ = WAVE_RESPAWN;
        }
        return;
    }

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

void ShootingGallery::onShoot() {
    if (gameOver_ || targets_.empty()) return;

    engine_.getCamera()->setFirstPerson(
        playerEye_, engine_.getCameraYaw(), engine_.getCameraPitch());

    Vec3 origin = engine_.getCamera()->eyePosition();
    Vec3 dir    = engine_.getCamera()->lookDirection();

    crosshairFlash_ = 0.20f;

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
        glVertex2i(0,0); glVertex2i(W,0); glVertex2i(W,H); glVertex2i(0,H);
        glEnd();
        glDisable(GL_BLEND);
    }
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
    }

    glColor3f(0.75f, 0.85f, 1.00f);
    engine_.drawString(10, 60, "STRZELNICA 3D");
    glColor3f(0.55f, 0.65f, 0.85f);
    engine_.drawString(10, 42, "WASD = ruch  |  mysz = celowanie  |  SPACJA = strzal  |  R = reset");

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
