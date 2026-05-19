// =============================================================================
// Room.cpp - Osoba 1
// Konstrukcja pomieszczenia: 6 scian (podloga, sufit, 4 sciany boczne) +
// dekoracje (kule, kostki, walce, torus, mini-stozek) z teksturami i materialami.
// =============================================================================

#include "ShootingGallery.h"
#include "Demo/Constants.h"

using namespace sg;

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

    Material wallMat   (Vec3(0.30f,0.25f,0.20f), Vec3(0.75f,0.65f,0.55f), Vec3(0.10f,0.10f,0.10f),  8.0f);
    Material floorMat  (Vec3(0.30f,0.30f,0.32f), Vec3(0.75f,0.75f,0.80f), Vec3(0.10f,0.10f,0.10f),  4.0f);
    Material ceilingMat(Vec3(0.18f,0.18f,0.22f), Vec3(0.45f,0.45f,0.50f), Vec3(0.05f,0.05f,0.05f),  4.0f);
    Material backMat   (Vec3(0.30f,0.20f,0.15f), Vec3(0.80f,0.55f,0.40f), Vec3(0.15f,0.10f,0.10f), 12.0f);

    const float FLOOR_UV_SCALE = 0.30f;
    const float WALL_UV_SCALE  = 0.20f;

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

void ShootingGallery::buildDecorations() {
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

    // Tylna sciana: 3 kule wysoko
    makeSphere(Vec3(-5.0f, 4.5f, -19.4f), 0.45f);
    makeSphere(Vec3( 0.0f, 5.0f, -19.4f), 0.55f);
    makeSphere(Vec3( 5.0f, 4.5f, -19.4f), 0.45f);

    // Frontowa sciana: 2 kostki dekoracyjne wysoko
    makeCube(Vec3(-5.0f, 4.0f,  9.3f), Vec3(0.8f, 0.8f, 0.4f));
    makeCube(Vec3( 5.0f, 4.0f,  9.3f), Vec3(0.8f, 0.8f, 0.4f));

    // Lewa sciana: 3 kule
    makeSphere(Vec3(-9.4f, 3.0f,  3.0f), 0.40f);
    makeSphere(Vec3(-9.4f, 3.0f, -6.0f), 0.40f);
    makeSphere(Vec3(-9.4f, 3.0f, -14.0f), 0.40f);

    // Prawa sciana: 3 kule analogicznie
    makeSphere(Vec3( 9.4f, 3.0f,  3.0f), 0.40f);
    makeSphere(Vec3( 9.4f, 3.0f, -6.0f), 0.40f);
    makeSphere(Vec3( 9.4f, 3.0f, -14.0f), 0.40f);

    // Sufit: torus centralny + 4 wiszace lampy
    {
        auto t = std::make_shared<TorusNode>(0.7f, 0.18f, 32, 14);
        t->setPosition(Vec3(0.0f, 5.3f, -5.0f));
        t->setMaterial(decMat);
        t->setTexture(torusTex_);
        root->addChild(t);
    }
    makeCylinder(Vec3(-5.0f, 5.3f,  3.0f),  0.18f, 0.6f);
    makeCylinder(Vec3( 5.0f, 5.3f,  3.0f),  0.18f, 0.6f);
    makeCylinder(Vec3(-5.0f, 5.3f, -13.0f), 0.18f, 0.6f);
    makeCylinder(Vec3( 5.0f, 5.3f, -13.0f), 0.18f, 0.6f);

    // Maly stozek-pinkle nad drzwiami (apex w dol - rotacja 180 wokol X)
    {
        auto k = std::make_shared<ConeNode>(0.30f, 0.50f, 16);
        k->setPosition(Vec3(0.0f, 5.5f, 9.0f));
        k->setRotation(Vec3(PI, 0.0f, 0.0f));
        k->setMaterial(decMat);
        k->setTexture(decorationTex_);
        root->addChild(k);
    }
}
