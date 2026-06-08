# Podział pracy — 4 osoby

Projekt **Silnik 3D + Strzelnica** podzielony został tak, aby każda z 4 osób odpowiadała za podobną ilość kodu, jednocześnie zachowując spójną odpowiedzialność tematyczną — każda osoba ma swój kawałek **silnika** i swój kawałek **gry**.

---

## Osoba 1 — Matematyka, graf sceny, budowa pokoju

### Silnik
- **`Engine/Math3D.h`** + **`Engine/Math3D.cpp`** — biblioteka matematyczna
  - Struktury `Vec3` i `Mat4` (operacje wektorowe, dot, cross, length, normalize)
  - Transformacje: translation, rotation X/Y/Z, scale
  - Projekcje: `Mat4::perspective`, `Mat4::orthographic`, `Mat4::lookAt`
  - Mnożenie macierzy, transformacje punktów i wektorów
  - Funkcje przecięcia promień-prymityw:
    - `raySphereIntersect` (równanie kwadratowe)
    - `rayCylinderIntersect` (walec pionowy)
    - `rayConeIntersect` (analityczne na zwężającym się walcu)
    - `rayAABBIntersect` (metoda slab)

- **`Engine/SceneNode.h`** + **`Engine/SceneNode.cpp`** — graf sceny
  - Hierarchia węzłów z transformacjami lokalnymi/światowymi
  - `localMatrix`, `worldMatrix` (rekurencja po przodkach)
  - `renderRecursive` (przechodzi drzewo)
  - Flaga `isVisible` do ukrywania węzłów (np. pula celów)

### Demo
- **`Demo/Room.cpp`** — konstrukcja pomieszczenia 20×6×30 m
  - `buildRoom()`: 6 ścian (podłoga, sufit, 4 ściany boczne) z odpowiednimi rotacjami, materiałami Phonga i teksturami (BMP z fallbackiem do proceduralnych)
  - `buildDecorations()`: kule, kostki, walce na ścianach + torus na suficie + mini-stożek nad drzwiami (~16 elementów dekoracyjnych)
  - Dobór skali UV dla każdej powierzchni

---

## Osoba 2 — Prymitywy 3D, przeszkody i kolizje

### Silnik
- **`Engine/PrimitiveNode.h`** + **`Engine/PrimitiveNode.cpp`** — wszystkie prymitywy
  - Klasa bazowa `PrimitiveNode` (materiał + tekstura, `renderSelf` z `glMaterialfv` i `glMultMatrixf`)
  - `CubeNode` — sześcian (6 ścian, UV per face, normalne)
  - `CylinderNode` — walec (powierzchnia boczna + 2 dyski, UV cylindryczne)
  - `SphereNode` — sfera (latitude/longitude, UV sferyczne)
  - `ConeNode` — stożek (boczne trójkąty z uśrednionymi normalnymi na apexie + dolny dysk)
  - `TorusNode` — torus (parametryzacja u,v)
  - `PlaneNode` — płaszczyzna (UV scaling przez `setUVScale`)

### Demo
- **`Demo/Obstacles.cpp`** — wszystkie przeszkody w pokoju
  - `buildObstacles()`: 6 cylindrów (kolumny) + 3 stożki + 2 skrzynie (CubeNode z skalą), rozmieszczone w przedniej i tylnej połowie pokoju
  - `isInsideObstacle()`: sprawdza czy punkt z marginesem leży w którejś przeszkodzie (3 typy: cylinder, stożek z malejącym promieniem, AABB)
  - `obstacleBlocksRay()`: sprawdza czy ray uderza w przeszkodę przed celem (używa funkcji z Math3D)
  - `applyObstacleCollision()`: 3 typy kolizji gracza (cylinder = okrągła, stożek = okrągła z promieniem podstawy, AABB = wypchnięcie do najbliższej krawędzi)

---

## Osoba 3 — Kamera, oświetlenie, tekstury, cele

### Silnik
- **`Engine/Camera.h`** + **`Engine/Camera.cpp`** — kamera
  - Dwa tryby: `ORBIT` (kamera krąży wokół celu) i `FIRST_PERSON` (stała pozycja, look-direction z yaw/pitch)
  - `setOrbit`, `setFirstPerson`, `lookDirection`, `eyePosition`
  - `viewMatrix` (używa `Mat4::lookAt`)
  - Projekcje perspektywiczna i ortogonalna

- **`Engine/Light.h`** + **`Engine/Light.cpp`** — oświetlenie
  - Struktura `Material` (ambient, diffuse, specular, shininess)
  - Klasa `PointLight` (pozycja, kolory, attenuation, `lightIndex` dla GL_LIGHTx)
  - `renderSelf` ustawia `glLightfv` w eye-space

- **`Engine/Texture.h`** + **`Engine/Texture.cpp`** — tekstury
  - `loadBMP()` — ręczny parser 24-bit BMP (header, BGR→RGB, padding wierszy, opcjonalny flip pionowy)
  - 7 generatorów proceduralnych:
    - `generateCheckerboard`, `generateGradient`, `generateStripes` (proste)
    - `generatePerlinNoise` (value noise + FBM)
    - `generateWood` (zaburzone słoje)
    - `generateBricks` (z fugą i wariacją koloru per cegła)
    - `generateMarble` (turbulencja na funkcji sin)
  - Upload do GPU z ręcznie generowanymi mipmapami (uśrednianie bloków 2×2)

### Demo
- **`Demo/Targets.cpp`** — cele i fale
  - `initTargetPool()`: pula 3 sfer `SphereNode` (visibility-toggled)
  - `defaultTargetMaterial()`: pomarańczowy materiał Phonga
  - `currentTargetPos()`: pozycja celu z bobem pionowym i ruchem liniowym (ping-pong sin)
  - `spawnWave()`: losowanie rozmiaru fali (1–3 cele), tryb ruchu (30% statyczny, 40% oś X, 30% oś Z) z losową fazą, walidacja pozycji (min. dystans od gracza, nie wewnątrz przeszkody, nie za blisko innych celów; do 60 prób)

### Assets graficzne
- **`assets/textures/*.bmp`** — pliki BMP do podłogi, ścian, celu, stożków, skrzyń (właściciel klasy `Texture` dobiera/konwertuje tekstury)
- **`assets/textures/README.md`** — instrukcja dla zespołu jak dodawać własne tekstury (źródła CC0, konwersja PNG→BMP, wymagania formatu)

---

## Osoba 4 — Rdzeń silnika, integracja, logika gry, HUD

### Silnik
- **`Engine/Engine.h`** + **`Engine/Engine.cpp`** — główna pętla i infrastruktura
  - Inicjalizacja FreeGLUT (okno, kontekst GL, depth buffer, double buffering)
  - Callbacki GLUT (`displayCallback`, `reshapeCallback`, `keyboardDownCallback`, `keyboardUpCallback`, `mouseCallback`, `motionCallback`, `timerCallback`)
  - Pętla `onTimer` z liczeniem dt i wywoływaniem `updateCallback`
  - Obsługa wejścia: tablica klawiszy `keys[256]`, free mouse look z `glutWarpPointer` na środek ekranu
  - System pauzy (ESC toggluje, Q wychodzi)
  - Tryby kontroli kamery: `ENGINE_ORBIT` (WASD silnika) vs `GAME_CONTROLLED` (gra zarządza)
  - HUD framework: `drawString`, `drawStringLarge`, ortho 2D, status silnika (FPS, oświetlenie, cieniowanie, siatka)
  - Wywoływanie callbacków gry: `updateCallback`, `shootCallback`, `hudCallback`, `resetCallback`

- **`main.cpp`** — punkt wejścia
  - Tworzy `Engine`, `ShootingGallery`, podłącza callbacki, woła `engine.run()`

### Infrastruktura / build
- **`CMakeLists.txt`** — konfiguracja CMake (źródła, biblioteki dla Windows/macOS/Linux, kopiowanie assets, freeglut.dll)
- **`.gitignore`** — wykluczenia git (build/, docs/, artefakty CMake, Silnik3D, .gitkeep)
- **`Doxyfile`** — konfiguracja Doxygen do generowania dokumentacji projektu (komentarze pisze każda osoba w swoich plikach, ale sam konfig jest u Osoby 4)
- **`README.md`** — dokumentacja projektu (instrukcje budowania, sterowanie, struktura, jak generować Doxygen, jak dodawać tekstury)

### Demo
- **`Demo/ShootingGallery.h`** — deklaracja głównej klasy gry (struktury `Target`, `ObstacleCyl`, `ObstacleCone`, `ObstacleBox`; wszystkie pola, metody publiczne i prywatne)
- **`Demo/Constants.h`** — wspólne stałe gry (`namespace sg`) — wymiary pokoju, strefa spawnowania, parametry celów, gracza
- **`Demo/Gameplay.cpp`** — logika gry i HUD
  - Konstruktor: setup ambient/oświetlenia, generuje teksturę celu, wywołuje wszystkie `buildX` z innych plików, konfiguruje silnik (FP camera, free mouse look)
  - `onUpdate`: ruch gracza WASD + kolizje + kamera + dekrementacja timerów flashy + respawn fali + animacja celów + wygasanie życia celu
  - `onShoot`: raycast od kamery, znajduje najbliższy cel, sprawdza blokowanie przez przeszkodę, przyznaje punkty/odbiera życie, bonus +2s dla pozostałych celów w fali
  - `onReset`, `resetGame`: restart po Game Over
  - `onHUD`: nakładki flashy (czerwone/zielone), celownik z animacją wystrzału, statystyki (cele/wynik/życia/czas/seria), pasek czasu celu, wskaźnik bonusu, ekran pauzy, ekran Game Over

---

## Statystyki podziału (przybliżone liczby linii)

| Osoba | Engine (linie) | Demo (linie) | Razem |
|---|---|---|---|
| **Osoba 1** | Math3D + SceneNode ≈ **380** | Room.cpp ≈ **155** | **535** |
| **Osoba 2** | PrimitiveNode ≈ **440** | Obstacles.cpp ≈ **180** | **620** |
| **Osoba 3** | Camera + Light + Texture ≈ **580** | Targets.cpp ≈ **115** | **695** |
| **Osoba 4** | Engine.cpp/h + main + Constants + ShootingGallery.h ≈ **550** | Gameplay.cpp ≈ **350** | **900** |

Osoba 4 ma więcej z uwagi na rozległy HUD (~150 linii) i integrację (konstruktor wiążący wszystko + 4 typy callbacków). Reszta osób ma kompaktowy moduł z czystą odpowiedzialnością tematyczną.

---

## Zalety podziału

- **Spójność tematyczna**: każda osoba widzi pełen pionowy slice (od matematyki/silnika do użycia w grze) — np. Osoba 2 zarówno implementuje cylinder/cone/AABB jako prymitywy, jak i używa ich do kolizji.
- **Minimalne zależności krzyżowe**: Osoba 1 (math) nie zależy od nikogo, Osoba 2 i 3 zależą od Osoby 1, Osoba 4 spina wszystko callbackami.
- **Możliwość pracy równoległej**: każda osoba ma swój plik `.cpp` — brak konfliktów merge w trakcie pisania.
- **`ShootingGallery.h` jako kontrakt**: Osoba 4 owns header → reszta wie jakie pola/metody są dostępne i implementuje swoje funkcje członkowskie w swoich plikach `.cpp`.

---

## Struktura plików — pełne przypisanie

```
pgk/
├── main.cpp                                  ← Osoba 4
├── CMakeLists.txt                            ← Osoba 4
├── .gitignore                                ← Osoba 4
├── Doxyfile                                  ← Osoba 4
├── README.md                                 ← Osoba 4
├── OSOBY.md                                  ← (meta — dla całego zespołu)
├── Engine/
│   ├── Math3D.h/cpp                          ← Osoba 1
│   ├── SceneNode.h/cpp                       ← Osoba 1
│   ├── PrimitiveNode.h/cpp                   ← Osoba 2
│   ├── Camera.h/cpp                          ← Osoba 3
│   ├── Light.h/cpp                           ← Osoba 3
│   ├── Texture.h/cpp                         ← Osoba 3
│   └── Engine.h/cpp                          ← Osoba 4
├── Demo/
│   ├── Constants.h                           ← Osoba 4
│   ├── ShootingGallery.h                     ← Osoba 4
│   ├── Room.cpp                              ← Osoba 1
│   ├── Obstacles.cpp                         ← Osoba 2
│   ├── Targets.cpp                           ← Osoba 3
│   └── Gameplay.cpp                          ← Osoba 4
└── assets/textures/
    ├── README.md                             ← Osoba 3
    └── *.bmp (5 plików)                      ← Osoba 3
```

**Notka**: każda osoba pisze komentarze Doxygen w swoich plikach `.h` — `Doxyfile` to wspólny konfig (utrzymywany przez Osobę 4 jako część build/infrastructure).
