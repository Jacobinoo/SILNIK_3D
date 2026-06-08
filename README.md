# Silnik 3D — FreeGLUT / OpenGL

Silnik grafiki 3D napisany w C++ z użyciem biblioteki FreeGLUT. Zaimplementowany od podstaw bez zewnętrznych silników czy frameworków graficznych — FreeGLUT służy wyłącznie jako pomoc do tworzenia okna i obsługi wejścia.

---

## Budowanie projektu

### Wymagania

- CMake 3.10+
- MinGW-w64 (GCC 14+) lub inny kompilator C++17
- FreeGLUT (zainstalowany przez vcpkg: `vcpkg install freeglut`)

### Kompilacja

```powershell
cmake -B build -G "MinGW Makefiles" -S .
cmake --build build --parallel 4
```

Plik wykonywalny: `build\Silnik3D.exe`  
`freeglut.dll` jest automatycznie kopiowany obok exe.

### Ponowna kompilacja po zmianach

```powershell
cmake --build build --parallel 4
```

---

## Dokumentacja Doxygen

W projekcie sa komentarze Doxygen na wszystkich publicznych klasach i metodach
w headerach silnika (`Engine/*.h`) oraz w glownej klasie gry (`Demo/ShootingGallery.h`).
Zeby wygenerowac dokumentacje HTML:

### Wymagania
- [Doxygen](https://www.doxygen.nl/download.html) (na Windows MSI installer)
- Opcjonalnie [Graphviz](https://graphviz.org/download/) dla diagramow klas

### Generowanie
```powershell
doxygen Doxyfile
```

Wygenerowana dokumentacja: `docs/html/index.html`

Konfiguracja w `Doxyfile`:
- `OUTPUT_LANGUAGE = Polish` - polskie nazwy sekcji
- `EXTRACT_ALL = YES` - wyciaga nawet niedokumentowane klasy (pokazuje strukture)
- `EXTRACT_PRIVATE = YES` - widoczne pola prywatne (przydaje sie do nauki)
- `USE_MDFILE_AS_MAINPAGE = README.md` - ten plik jako strona glowna

---

## Demo: Strzelnica 3D

Pomarańczowy obracający się cel pojawia się losowo w przestrzeni przed graczem. Trafienie spacją = +1 punkt. Pudło lub niezestrzelenie w czasie 5 sekund = -1 życie. Start: 5 żyć.

### Sterowanie gry

| Klawisz / Akcja         | Efekt                                       |
|-------------------------|---------------------------------------------|
| `LPM` + ruch myszy      | Celowanie (rozglądanie się w trybie FP)     |
| `Spacja`                | Strzał — raycast od kamery w kierunku patrzenia |
| `R`                     | Restart gry (po Game Over)                  |

### Sterowanie silnika (dostępne zawsze)

| Klawisz | Efekt                                          |
|---------|------------------------------------------------|
| `L`     | Włącz / wyłącz oświetlenie                    |
| `G`     | Przełącz cieniowanie: gładkie ↔ płaskie        |
| `M`     | Przełącz tryb siatki (wireframe)               |
| `P`     | Rzutowanie perspektywiczne                     |
| `O`     | Rzutowanie ortogonalne                         |
| `+`/`-` | Zwiększ / zmniejsz docelowy FPS                |
| `ESC`   | Zamknij aplikację                              |

### Funkcje techniczne demonstrowane przez grę

- **Ray-sphere intersection** — własna implementacja w `Math3D.cpp` (`raySphereIntersect`)
- **Animacje** — obrót celu wokół osi Y + sinusoidalne kołysanie pionowe
- **Licznik czasu** — `dt` (delta time) liczony przez silnik, przekazywany do gry
- **Tryb pierwszoosobowy kamery** — nowy tryb `FIRST_PERSON` w `Camera`
- **Tekstury proceduralne** — szachownica, paski, generowane bez plików
- **HUD 2D z grą** — krzyż celownika, wynik, życia, pasek czasu celu, ekran Game Over
- **Callbacks update/shoot/reset/hud** — silnik odpina się od logiki gry

---

## Architektura silnika

### Hierarchia klas

```
SceneNode                  ← bazowy węzeł grafu sceny
├── Camera                 ← kamera orbitalna
├── PointLight             ← punktowe źródło światła (Phong)
└── PrimitiveNode          ← baza dla obiektów rysowanych
    ├── CubeNode           ← sześcian z UV
    ├── CylinderNode       ← walec z UV
    ├── SphereNode         ← sfera z UV sferycznym
    └── PlaneNode          ← płaszczyzna XZ z UV
```

Każdy `SceneNode` może mieć dzieci — transformacje (pozycja, obrót, skala) są dziedziczone w dół hierarchii.

### Pliki źródłowe

| Plik                  | Opis                                                    |
|-----------------------|---------------------------------------------------------|
| `Math3D.h/cpp`        | Wektory (Vec3) i macierze (Mat4 4×4), operacje 3D       |
| `SceneNode.h/cpp`     | Węzeł grafu sceny, transformacje lokalne i globalne     |
| `Camera.h/cpp`        | Kamera orbitalna, macierz widoku i projekcji            |
| `Light.h/cpp`         | Struktura Material, klasa PointLight (OpenGL GL_LIGHTx) |
| `PrimitiveNode.h/cpp` | Prymitywy 3D z UV i obsługą tekstur                    |
| `Texture.h/cpp`       | Ładowanie BMP + generatory proceduralne                 |
| `Engine.h/cpp`        | Główna pętla, callbacki GLUT, HUD, zarządzanie sceną   |
| `main.cpp`            | Punkt wejścia                                           |

---

## Funkcjonalności

### Obsługa klawiatury i myszy
- Pełna obsługa zdarzeń klawiatury (keyDown / keyUp) z tablicą stanu klawiszy.
- Mysz: obrót kamery przez drag lewego przycisku, zoom przez scroll.

### Zmienna szybkość odświeżania
- Cel FPS regulowany klawiszami `+` / `-` w czasie działania.
- Licznik FPS wyświetlany na ekranie w czasie rzeczywistym.
- Pętla oparta na `glutTimerFunc` (nie blokuje wątku głównego).

### Prymitywy 3D
Wszystkie prymitywy rysowane ręcznie przez `glBegin/glEnd` bez użycia funkcji `glutSolid*`:

- **CubeNode** — 6 ścian jako `GL_QUADS`, każda z własną normalną i UV (0,0)→(1,1)
- **CylinderNode** — boczna powierzchnia jako `GL_TRIANGLE_STRIP`, dwa dyski jako `GL_TRIANGLE_FAN`
- **SphereNode** — siatka stacks×slices (`GL_TRIANGLE_STRIP`), UV sferyczne (φ/2π, θ/π)
- **PlaneNode** — czworokąt w płaszczyźnie XZ, normalny w górę, UV skalowane do rozmiaru

### Kamera
Kamera orbitalna śledząca punkt docelowy (target):
- Parametry: `yaw` (poziomo), `pitch` (pionowo), `distance` (odległość)
- Macierz widoku generowana przez własną implementację `lookAt`
- Tryb perspektywiczny i ortogonalny z własną macierzą projekcji

### Transformacje geometryczne
Każdy `SceneNode` obsługuje:
- `setPosition(Vec3)` — przesunięcie
- `setRotation(Vec3)` — obrót (euler: X, Y, Z)
- `setScale(Vec3)` — skalowanie
- Metody przyrostowe: `translate()`, `rotate()`, `rescale()`
- Macierz lokalna = `T * Ry * Rx * Rz * S`
- Macierz globalna = rekurencyjny iloczyn macierzy od korzenia grafu

### Oświetlenie
Model oświetlenia Phonga przez OpenGL Fixed-Function Pipeline:
- Światło punktowe (`PointLight`) z parametrami ambient / diffuse / specular
- Tłumienie (constant / linear / quadratic attenuation)
- Material per-obiekt: ambient, diffuse, specular, shininess
- Klawisz `L` włącza / wyłącza oświetlenie w czasie rzeczywistym

### Cieniowanie
- **Gładkie** (`GL_SMOOTH`) — interpolacja kolorów między wierzchołkami (Gouraud)
- **Płaskie** (`GL_FLAT`) — jednolity kolor na każdym trójkącie
- Przełączanie klawiszem `G` bez restartu aplikacji

### Teksturowanie
Klasa `Texture` zaimplementowana od podstaw:

**Ładowanie BMP** (`loadBMP`):
- Ręczny parser nagłówka pliku BMP (14 B) i nagłówka DIB (40 B)
- Obsługuje 24-bitowe nieskompresowane BMP (BI_RGB)
- Konwersja BGR → RGB, obsługa obrazów top-down (ujemna wysokość)
- Wyrównanie wierszy do 4 bajtów (stride padding)

**Tekstury proceduralne**:
- `generateCheckerboard(size, kolor1, kolor2, tileCount)` — szachownica
- `generateGradient(size, kolor1, kolor2, horizontal)` — gradient liniowy
- `generateStripes(size, kolor1, kolor2, stripeCount)` — pionowe paski

**OpenGL**:
- Upload przez `gluBuild2DMipmaps` (automatyczne mipmapy)
- Filtrowanie: `GL_LINEAR_MIPMAP_LINEAR` (min), `GL_LINEAR` (mag)
- Tryb: `GL_MODULATE` — tekstura mnoży kolor materiału

**Domyślne tekstury w scenie**:
| Obiekt    | Tekstura               |
|-----------|------------------------|
| Sześcian  | Szachownica b/w        |
| Walec     | Paski niebiesko-pomarańczowe |
| Sfera     | Gradient zielono-biały |
| Podłoga   | Szachownica szara      |

Aby załadować własną teksturę BMP:
```cpp
auto tex = std::make_shared<Texture>();
tex->loadBMP("moja_tekstura.bmp");  // 24-bit, nieskompresowany BMP
engine.getCube()->setTexture(tex);
```

---

## HUD (wyświetlacz ekranowy)

W lewym górnym rogu wyświetlane są aktualne stany:
```
FPS: 60
Oswietlenie [L]: WL
Cieniowanie [G]: Gladkie
Siatka      [M]: WYL
Rzutowanie [P/O]: Perspektywiczne
```

Na dole ekranu: skrócona pomoc ze skrótami klawiszowymi.

---

## Rozszerzanie silnika

### Dodanie własnego prymitywu

```cpp
// MojPrymityw.h
class MojPrymityw : public PrimitiveNode {
public:
    MojPrymityw() : PrimitiveNode("MojPrymityw") {}
protected:
    void drawGeometry() const override {
        // glBegin / glEnd z glNormal3f i glTexCoord2f
    }
};
```

### Dodanie obiektu do sceny

```cpp
auto obj = std::make_shared<MojPrymityw>();
obj->setPosition(Vec3(0.0f, 1.0f, 0.0f));
obj->setMaterial(Material(ambient, diffuse, specular, shininess));
engine.getSceneRoot()->addChild(obj);
```

### Animacja w pętli

Animację najlepiej umieścić w podklasie `SceneNode` nadpisując `renderSelf`, lub wywoływać transformacje przed każdą klatką wewnątrz `Engine::onTimer()`.

---

## Struktura katalogów

```
pgk/
├── build/              ← katalog kompilacji (generowany)
│   ├── Silnik3D.exe
│   └── freeglut.dll
├── CMakeLists.txt
├── README.md
├── main.cpp
├── Engine.h / Engine.cpp
├── Math3D.h / Math3D.cpp
├── SceneNode.h / SceneNode.cpp
├── Camera.h / Camera.cpp
├── Light.h / Light.cpp
├── PrimitiveNode.h / PrimitiveNode.cpp
└── Texture.h / Texture.cpp
```
