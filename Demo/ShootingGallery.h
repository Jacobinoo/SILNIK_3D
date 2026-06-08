/**
 * @file ShootingGallery.h
 * @brief Glowna klasa gry demo - Strzelnica 3D FPP.
 *
 * Gra rozgrywa sie w zamknietym pokoju z roznymi przeszkodami (cylindry,
 * stozki, skrzynie). Pojawiaja sie fale 1-3 ruchomych celow ktore trzeba
 * trafiac w okreslonym czasie. Implementacja klasy podzielona na 4 pliki
 * .cpp (Room.cpp, Obstacles.cpp, Targets.cpp, Gameplay.cpp - patrz OSOBY.md).
 */
#ifndef SHOOTING_GALLERY_H
#define SHOOTING_GALLERY_H

#include "Engine/Engine.h"
#include <memory>
#include <random>
#include <vector>

/**
 * @brief Glowna klasa gry "Strzelnica 3D".
 *
 * @details Klasa zarzadza calym stanem gry: scena (pokoj, przeszkody,
 * dekoracje), aktywne cele i ich animacje, pozycja gracza, punkty/zycia/czas,
 * fale celow z bonusami za seryjne trafienia, HUD.
 *
 * Wszystkie public metody to callbacki rejestrowane w silniku przez
 * Engine::setXxxCallback() - silnik wola je w odpowiednich momentach
 * petli renderowania / wejscia.
 */
class ShootingGallery {
public:
    /**
     * @brief Konstruuje gre - buduje scene, dodaje swiatla, konfiguruje silnik.
     * @param engine referencja do silnika (gra trzyma ja jako engine_)
     */
    explicit ShootingGallery(Engine& engine);

    /** @brief Co klatke - ruch gracza, kolizje, animacje, wygasanie celow. */
    void onUpdate(float dt);
    /** @brief Strzal - raycast od kamery, znajduje cel, przyznaje pkty / odbiera zycia. */
    void onShoot();
    /** @brief Rysuje HUD gry (celownik, statystyki, paski, ekran Game Over/Pauza). */
    void onHUD();
    /** @brief Reset po Game Over (klawisz R). */
    void onReset();

private:
    // ---------- Struktury wewnetrzne ----------

    /**
     * @brief Aktywny cel w obecnej fali.
     *
     * basePos to "kotwica" wokol ktorej cel oscyluje, moveAxis to kierunek
     * ruchu liniowego (zerowy = cel stoi), moveRange to amplituda. movePhase
     * to losowe przesuniecie zeby cele w fali nie ruszaly sie synchronicznie.
     * bobPhase to faza pionowego kolysania. spinAngle to aktualny kat obrotu
     * wokol Y (animacja). spawnTime to znacznik czasu utworzenia - sluzy do
     * obliczania pozostalego czasu zycia. nodeIndex to indeks do
     * targetNodePool_ - sfera ktora reprezentuje ten cel wizualnie.
     */
    struct Target {
        Vec3  basePos;
        Vec3  moveAxis;
        float moveRange;
        float moveSpeed;
        float movePhase;
        float bobPhase;
        float spinAngle;
        float spawnTime;
        int   nodeIndex;
    };

    /** @brief Przeszkoda - kolumna walcowa, kolizja XZ + zakres Y. */
    struct ObstacleCyl  { Vec3 base;   float radius, height; };
    /** @brief Przeszkoda - stozek (promien zalezny od wysokosci). */
    struct ObstacleCone { Vec3 base;   float radius, height; };
    /** @brief Przeszkoda - prostopadloscian (AABB). */
    struct ObstacleBox  { Vec3 boxMin; Vec3  boxMax; };

    // ---------- Metody pomocnicze (implementowane w 4 plikach .cpp) ----------

    void buildRoom();          ///< Room.cpp - sciany, podloga, sufit
    void buildObstacles();     ///< Obstacles.cpp - kolumny, stozki, skrzynie
    void buildDecorations();   ///< Room.cpp - dekoracje na scianach/suficie
    void initTargetPool();     ///< Targets.cpp - rezerwuje 3 SphereNode jako pula
    void spawnWave();          ///< Targets.cpp - losowanie nowej fali 1-3 celow
    void resetGame();          ///< Gameplay.cpp - reset stanu po Game Over

    /** @brief Aktualna pozycja celu z uwzglednieniem ruchu liniowego i kolysania. */
    Vec3  currentTargetPos(const Target& t) const;
    /** @brief Czy punkt z marginesem jest wewnatrz ktorejs z przeszkod. */
    bool  isInsideObstacle(const Vec3& pos, float margin) const;
    /** @brief Czy promien uderza w jakas przeszkode wczesniej niz w cel. */
    bool  obstacleBlocksRay(const Vec3& origin, const Vec3& dir, float maxT) const;
    /** @brief Wypycha gracza z przeszkod (3 typy kolizji: cyl, stozek, AABB). */
    void  applyObstacleCollision();
    /** @brief Standardowy material celu (pomaranczowy Phong). */
    Material defaultTargetMaterial() const;

    // ---------- Pola ----------

    Engine& engine_;

    Vec3 playerEye_;       ///< aktualna pozycja oka gracza (FP camera)
    Vec3 playerEyeHome_;   ///< pozycja startowa - do resetu po Game Over

    // ---- Pokoj ----
    std::shared_ptr<PlaneNode>  floor_;
    std::shared_ptr<PlaneNode>  ceiling_;
    std::shared_ptr<PlaneNode>  backWall_;
    std::shared_ptr<PlaneNode>  frontWall_;
    std::shared_ptr<PlaneNode>  leftWall_;
    std::shared_ptr<PlaneNode>  rightWall_;

    /** @brief Druga lampa (pierwsza domyslnie wystepuje w silniku). */
    std::shared_ptr<PointLight> secondLight_;

    // ---- Przeszkody i ich wezly sceny ----
    std::vector<std::shared_ptr<CylinderNode>> cylinderNodes_;
    std::vector<std::shared_ptr<ConeNode>>     coneNodes_;
    std::vector<std::shared_ptr<CubeNode>>     boxNodes_;
    std::vector<ObstacleCyl>  cylinderObs_;
    std::vector<ObstacleCone> coneObs_;
    std::vector<ObstacleBox>  boxObs_;

    /** @brief Pula 3 sfer dla celow - cele "respawnuja" przez setVisible zamiast tworzenia/niszczenia wezlow. */
    std::vector<std::shared_ptr<SphereNode>> targetNodePool_;

    // ---- Aktywne cele i fala ----
    std::vector<Target> targets_;    ///< cele w obecnej fali
    float waveRespawnTimer_;         ///< odliczanie do nastepnej fali
    int   waveSize_;                 ///< ile celow bylo w tej fali (dla HUD)

    // ---- Tekstury ----
    std::shared_ptr<Texture> floorTex_;
    std::shared_ptr<Texture> ceilingTex_;
    std::shared_ptr<Texture> wallTex_;       ///< wspolna tekstura wszystkich 4 scian
    std::shared_ptr<Texture> pillarTex_;
    std::shared_ptr<Texture> coneTex_;
    std::shared_ptr<Texture> boxTex_;
    std::shared_ptr<Texture> targetTex_;
    std::shared_ptr<Texture> decorationTex_;
    std::shared_ptr<Texture> torusTex_;

    // ---- Stan gry ----
    int   score_;            ///< liczba trafien
    int   streak_;           ///< aktualna seria z rzedu (resetuje sie po pudle)
    int   bestStreak_;       ///< najlepsza seria w tej rundzie (pokazywana na Game Over)
    int   lives_;            ///< pozostale zycia
    float totalTime_;        ///< sekundy od startu gry
    bool  gameOver_;
    float hitFlashTime_;     ///< zielony flash ekranu po trafieniu (zmniejsza sie do 0)
    float missFlashTime_;    ///< czerwony flash po pudle
    float crosshairFlash_;   ///< rozmycie celownika tuz po strzale
    float bonusFlashTime_;   ///< wskaznik "+2s do pozostalych celow"

    /** @brief Generator liczb losowych do spawnowania pozycji celow i wyboru trybu ruchu. */
    std::mt19937 rng_;
};

#endif
