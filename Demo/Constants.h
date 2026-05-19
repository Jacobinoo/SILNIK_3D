#ifndef SHOOTING_GALLERY_CONSTANTS_H
#define SHOOTING_GALLERY_CONSTANTS_H

// Wspolne stale dla wszystkich plikow gry (Room, Obstacles, Targets, Gameplay).
// constexpr zmienne w namespace sa od C++17 niejawnie inline, wiec naglowek
// moze byc wlaczany w wielu jednostkach kompilacji bez konfliktow linkera.
namespace sg {

    constexpr float PI = 3.14159265358979323846f;

    // ---- Wymiary pokoju ----
    constexpr float ROOM_X_HALF   = 10.0f;
    constexpr float ROOM_HEIGHT   =  6.0f;
    constexpr float ROOM_Z_MIN    = -20.0f;
    constexpr float ROOM_Z_MAX    =  10.0f;
    constexpr float ROOM_LENGTH   = ROOM_Z_MAX - ROOM_Z_MIN;
    constexpr float ROOM_WIDTH    = 2.0f * ROOM_X_HALF;
    constexpr float ROOM_Z_CENTER = (ROOM_Z_MIN + ROOM_Z_MAX) * 0.5f;
    constexpr float WALL_BUFFER   = 0.5f;

    // ---- Strefa pojawiania celow (caly pokoj) ----
    constexpr float SPAWN_X_HALF              = 8.5f;
    constexpr float SPAWN_Y_MIN               = 1.0f;
    constexpr float SPAWN_Y_MAX               = 4.5f;
    constexpr float SPAWN_Z_MIN               = -18.0f;
    constexpr float SPAWN_Z_MAX               = 7.0f;
    constexpr float MIN_SPAWN_DIST_FROM_PLAYER = 3.5f;
    constexpr float MIN_SPAWN_DIST_FROM_OTHER  = 3.0f;

    // ---- Cele i fale ----
    constexpr int   MAX_TARGETS     = 3;
    constexpr float TARGET_RADIUS   = 0.50f;
    constexpr float HIT_TOLERANCE   = 1.40f;
    constexpr float TARGET_LIFETIME = 3.45f;
    constexpr float WAVE_HIT_BONUS  = 2.0f;
    constexpr float WAVE_RESPAWN    = 0.50f;
    constexpr float MAX_MOVE_RANGE  = 1.6f;

    // ---- Gracz / rozgrywka ----
    constexpr int   START_LIVES       = 2;
    constexpr float FLASH_DURATION    = 0.35f;
    constexpr float MAX_SHOOT_DIST    = 100.0f;
    constexpr float PLAYER_HEIGHT     = 1.75f;
    constexpr float PLAYER_RADIUS     = 0.35f;
    constexpr float PLAYER_MOVE_SPEED = 4.5f;

}  // namespace sg

#endif
