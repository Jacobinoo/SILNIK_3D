/**
 * @file Constants.h
 * @brief Wspolne stale gry uzywane w 4 plikach .cpp.
 *
 * Constexpr zmienne w namespace sa od C++17 niejawnie inline, dlatego naglowek
 * mozna bezpiecznie wlaczac w wielu jednostkach kompilacji bez konfliktow
 * linkera. Wszystko zmienne sa w namespace `sg` zeby nie zasmiecac
 * globalnej przestrzeni.
 *
 * Latwo modyfikowac balans gry - poziom trudnosci, rozmiar pokoju, zakres
 * spawnowania - wszystko tu w jednym miejscu.
 */
#ifndef SHOOTING_GALLERY_CONSTANTS_H
#define SHOOTING_GALLERY_CONSTANTS_H

/** @brief Namespace dla stalych gry Strzelnica. */
namespace sg {

    constexpr float PI = 3.14159265358979323846f;  ///< no PI tutaj kazdy zna

    // ---- Wymiary pokoju ----
    constexpr float ROOM_X_HALF   = 10.0f;     ///< polowa szerokosci pokoju w X
    constexpr float ROOM_HEIGHT   =  6.0f;     ///< wysokosc pokoju (Y)
    constexpr float ROOM_Z_MIN    = -20.0f;    ///< najdalej w Z (tylna sciana)
    constexpr float ROOM_Z_MAX    =  10.0f;    ///< najblizej (frontowa sciana)
    constexpr float ROOM_LENGTH   = ROOM_Z_MAX - ROOM_Z_MIN;   ///< 30m
    constexpr float ROOM_WIDTH    = 2.0f * ROOM_X_HALF;        ///< 20m
    constexpr float ROOM_Z_CENTER = (ROOM_Z_MIN + ROOM_Z_MAX) * 0.5f;
    constexpr float WALL_BUFFER   = 0.5f;      ///< odstep od scian przy clamping pozycji gracza

    // ---- Strefa pojawiania celow (praktycznie caly pokoj) ----
    constexpr float SPAWN_X_HALF              = 8.5f;   ///< lekko mniejsza niz pokoj zeby cele nie ocieraly o sciany
    constexpr float SPAWN_Y_MIN               = 1.0f;
    constexpr float SPAWN_Y_MAX               = 4.5f;
    constexpr float SPAWN_Z_MIN               = -18.0f;
    constexpr float SPAWN_Z_MAX               = 7.0f;
    constexpr float MIN_SPAWN_DIST_FROM_PLAYER = 3.5f;  ///< zeby cel nie zaspaowal pod nosem
    constexpr float MIN_SPAWN_DIST_FROM_OTHER  = 3.0f;  ///< cele w fali sie nie pokrywaja

    // ---- Cele i fale ----
    constexpr int   MAX_TARGETS     = 3;       ///< max ile celow naraz w fali (rozmiar puli)
    constexpr float TARGET_RADIUS   = 0.50f;
    constexpr float HIT_TOLERANCE   = 1.40f;   ///< mnoznik promienia dla raycast - latwiej trafic
    constexpr float TARGET_LIFETIME = 3.45f;   ///< czas zycia celu w sekundach
    constexpr float WAVE_HIT_BONUS  = 2.0f;    ///< +2s dla pozostalych celow w fali po trafieniu jednego
    constexpr float WAVE_RESPAWN    = 0.50f;   ///< pauza miedzy fala a kolejna fala
    constexpr float MAX_MOVE_RANGE  = 1.6f;    ///< amplituda ruchu liniowego celu

    // ---- Gracz / rozgrywka ----
    constexpr int   START_LIVES       = 2;     ///< trudna gra - tylko 2 zycia na start
    constexpr float FLASH_DURATION    = 0.35f; ///< dlugosc flashy ekranowych (hit/miss)
    constexpr float MAX_SHOOT_DIST    = 100.0f;
    constexpr float PLAYER_HEIGHT     = 1.75f; ///< wzrost gracza (pozycja oka)
    constexpr float PLAYER_RADIUS     = 0.35f; ///< promien hitboxu gracza (do kolizji ze scianami / obstacles)
    constexpr float PLAYER_MOVE_SPEED = 4.5f;  ///< m/s

}  // namespace sg

#endif
