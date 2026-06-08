# -*- coding: utf-8 -*-
from _pdfkit import *

story = []
story += title_block(
    "Jakub",
    "Osoba 4",
    "Rdzen silnika, integracja, logika gry, HUD",
    "Engine/Engine.*, main.cpp, CMakeLists.txt, Demo/Gameplay.cpp, Demo/ShootingGallery.h, Demo/Constants.h"
)

# ============ MAPA ============
story.append(H1("0. Gdzie sa moje pliki (mapa do prezentacji)"))
story.append(P("Strzalka <b>&lt;&lt;&lt;</b> pokazuje pliki, ktore robilem ja (Jakub)."))
story += TREE(
"""pgk/
|
+-- main.cpp              <<< MOJE  (start programu, spina wszystko)
+-- CMakeLists.txt        <<< MOJE  (konfiguracja kompilacji)
+-- README.md  Doxyfile  .gitignore  <<< MOJE  (infrastruktura)
|
+-- Engine/
|   +-- Math3D.* SceneNode.*           (Lukasz)
|   +-- PrimitiveNode.*                 (Rogert)
|   +-- Camera.* Light.* Texture.*      (Kacper)
|   +-- Engine.h / Engine.cpp   <<< MOJE  (okno, petla, wejscie, callbacki)
|
+-- Demo/
|   +-- Room.cpp Obstacles.cpp Targets.cpp   (Lukasz/Rogert/Kacper)
|   +-- ShootingGallery.h  <<< MOJE  (deklaracja calej klasy gry)
|   +-- Constants.h        <<< MOJE  (stale gry - balans)
|   +-- Gameplay.cpp       <<< MOJE  (logika gry, strzelanie, HUD)
""")
story.append(P("<b>W skrocie:</b> ja jestem 'spinaczem'. Robie <b>rdzen silnika</b> (okno przez "
               "FreeGLUT, petla rysowania, obsluga klawiatury i myszy) oraz <b>glowna logike gry</b> "
               "(co sie dzieje co klatke, strzelanie, punkty, zycia, HUD). Naglowek "
               "ShootingGallery.h jest 'kontraktem' - reszta zespolu implementuje swoje funkcje, "
               "ktore ja tu zadeklarowalem."))

# ============ 1. WPROWADZENIE ============
story.append(PageBreak())
story.append(H1("1. O co chodzi w moich plikach (po ludzku)"))
story.append(P("<b>Engine</b> to silnik bez gry - sam w sobie tylko otwiera okno, ustawia OpenGL i "
               "kreci petla: 'policz -> narysuj -> powtorz'. Gra podpina sie do niego przez "
               "<b>callbacki</b> (funkcje zwrotne): silnik mowi 'mamy nowa klatke' i wola moja "
               "funkcje gry."))
story.append(P("<b>Gameplay.cpp</b> to mozg gry: porusza graczem, sprawdza strzaly, liczy punkty i "
               "zycia, rysuje HUD (celownik, wyniki, ekran konca gry). <b>Constants.h</b> trzyma "
               "wszystkie liczby balansu w jednym miejscu (np. ile zyc, jak dlugo zyje cel)."))

# ============ 2. ENGINE - petla i wejscie ============
story.append(H1("2. Engine - serce silnika"))
story.append(H2("2.1. Petla czasu (zmienne FPS)"))
story.append(P("FreeGLUT wola moja funkcje onTimer co pewien czas. Licze <b>dt</b> (czas od "
               "ostatniej klatki) i przekazuje go do gry - dzieki temu ruch jest plynny niezaleznie "
               "od FPS."))
story += CODE(
"""void Engine::onTimer() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (paused) { lastTickMs = now; glutPostRedisplay();
                  glutTimerFunc(1000/targetFPS, timerCallback, 0); return; }
    float dt = (now - lastTickMs) / 1000.0f;   // sekundy od ostatniej klatki
    lastTickMs = now;
    ... // ruch kamery silnika (gdy tryb ENGINE_ORBIT)
    if (updateCallback) updateCallback(dt);    // <- logika gry
    glutPostRedisplay();                       // popros o przerysowanie
    glutTimerFunc(1000/targetFPS, timerCallback, 0);  // zaplanuj kolejna klatke
}""", "Engine.cpp - petla czasu z dt i pauza")
story.append(P("Klawisze +/- zmieniaja targetFPS, czyli jak czesto planujemy klatke - to spelnia "
               "wymaganie 'zmienna szybkosc odswiezania'. Pauza (ESC) pomija logike gry, ale dalej "
               "rysuje (zeby pokazac ekran PAUZA)."))

story.append(H2("2.2. Obsluga wejscia + free mouse look"))
story.append(P("Klawiatura: trzymam tablice keys[256] (true gdy wcisniety). Gra pyta isKeyDown('w'). "
               "Mysz w trybie FPP chowam kursor i po kazdym ruchu przesuwam go z powrotem na srodek "
               "ekranu - dzieki temu mozna sie obracac w nieskonczonosc."))
story += CODE(
"""void Engine::handleMouseMotion(int x, int y) {
    if (paused) return;
    if (freeMouseLook) {
        int cx = windowWidth/2, cy = windowHeight/2;
        if (x==cx && y==cy) return;            // ignoruj wlasny warp
        cameraYaw   += (x-cx) * mouseSensitivity;
        cameraPitch += -(y-cy) * mouseSensitivity;
        cameraPitch  = clamp(cameraPitch, -85deg, +85deg);
        glutWarpPointer(cx, cy);               // wroc kursor na srodek
    }
}""", "Engine.cpp - rozgladanie sie myszka (uproszczone)")

story.append(H2("2.3. Callbacki - jak silnik laczy sie z gra"))
story.append(P("Silnik nie wie nic o 'strzelnicy'. Trzyma tylko wskazniki na funkcje "
               "(std::function), ktore gra mu daje. To rozdziela silnik od konkretnej gry."))
story += CODE(
"""// w main.cpp - gra rejestruje sie w silniku:
engine.setUpdateCallback([&](float dt){ game.onUpdate(dt); });
engine.setShootCallback ([&](){ game.onShoot(); });
engine.setHUDCallback   ([&](){ game.onHUD(); });
engine.setResetCallback ([&](){ game.onReset(); });""",
    "main.cpp - podpiecie logiki gry pod silnik")

# ============ 3. STATYCZNE CALLBACKI ============
story.append(H2("2.4. Dlaczego callbacki GLUT sa statyczne"))
story.append(P("FreeGLUT to biblioteka w C - przyjmuje zwykle funkcje (wskazniki), nie metody "
               "klasy. Dlatego mam statyczne funkcje, ktore przez wskaznik 'instance' wolaja "
               "prawdziwe metody obiektu Engine."))
story += CODE(
"""static Engine* instance;   // jedyny obiekt silnika

void Engine::displayCallback() { if (instance) instance->render(); }
void Engine::timerCallback(int){ if (instance) instance->onTimer(); }
// GLUT dostaje displayCallback, a ta woła instance->render()""",
    "Engine.cpp - mostek miedzy C-owym GLUT a obiektem C++")

# ============ 4. GAMEPLAY ============
story.append(PageBreak())
story.append(H1("3. Demo/Gameplay.cpp - logika gry"))
story.append(P("Tu jest konstruktor gry (ustawia swiatla, wola buildRoom/buildObstacles itd. od "
               "kolegow, konfiguruje kamere FPP) oraz 4 glowne funkcje: onUpdate, onShoot, onHUD, "
               "onReset."))

story.append(H2("3.1. onShoot - strzelanie (raycast)"))
story += CODE(
"""void ShootingGallery::onShoot() {
    if (gameOver_ || targets_.empty()) return;
    // odswiez kamere NAJNOWSZYM yaw/pitch (mysz mogla sie ruszyc!)
    engine_.getCamera()->setFirstPerson(playerEye_,
            engine_.getCameraYaw(), engine_.getCameraPitch());
    Vec3 origin = engine_.getCamera()->eyePosition();
    Vec3 dir    = engine_.getCamera()->lookDirection();

    float closestT = MAX_SHOOT_DIST; int hitIdx = -1;
    for (size_t i=0; i<targets_.size(); ++i) {
        float t;
        if (raySphereIntersect(origin, dir, currentTargetPos(targets_[i]),
                               TARGET_RADIUS*HIT_TOLERANCE, t)
            && t > 0 && t < closestT) { closestT = t; hitIdx = i; }
    }
    if (hitIdx >= 0 && obstacleBlocksRay(origin, dir, closestT)) hitIdx = -1;
    // hitIdx >= 0 -> trafienie; inaczej pudlo (-1 zycie)
}""", "Gameplay.cpp - rdzen strzelania")
story.append(P("<b>Najwazniejszy szczegol:</b> przed raycastem odswiezam kamere najnowszym "
               "yaw/pitch. To naprawilo bug, gdzie strzaly mijaly cel - mysz mogla sie ruszyc "
               "miedzy klatka a wcisnieciem spacji, wiec kierunek byl nieaktualny. "
               "<b>HIT_TOLERANCE</b> (1.4) lekko powieksza cel, zeby latwiej bylo trafic."))

story.append(H2("3.2. onUpdate - co klatke"))
story.append(P("Porusza graczem (WASD), wola kolizje (Rogert), aktualizuje kamere, odlicza timery "
               "efektow, sprawdza czy cele nie wygasly, respawnuje fale. Wszystko skalowane przez "
               "<b>dt</b>, zeby tempo bylo niezalezne od FPS."))
story += CODE(
"""float speed = PLAYER_MOVE_SPEED * dt;          // metry = predkosc * czas
if (engine_.isKeyDown('w')) playerEye_ += fwd * speed;
...
applyObstacleCollision();                       // wypchnij z przeszkod
playerEye_.x = clamp(playerEye_.x, sciany...);  // nie wychodz z pokoju""",
    "Gameplay.cpp - ruch gracza z dt")

story.append(H2("3.3. onHUD - interfejs 2D"))
story.append(P("Rysuje na plasko (2D): celownik na srodku, wyniki/zycia/czas w rogu, pasek czasu "
               "celu, czerwony/zielony blysk po pudle/trafieniu, ekran Game Over i Pauza. Uzywam "
               "pomocnikow drawString z silnika."))

# ============ 5. CONSTANTS + HEADER ============
story.append(H1("4. Constants.h i ShootingGallery.h"))
story.append(P("<b>Constants.h</b> to wszystkie liczby balansu w jednym miejscu (namespace sg): "
               "rozmiar pokoju, ile zyc, jak dlugo zyje cel, predkosc gracza. Zmiana trudnosci = "
               "zmiana jednej liczby. Uzywam constexpr, ktore w C++17 sa inline - mozna wlaczac "
               "naglowek w wielu plikach bez bledu linkera."))
story.append(P("<b>ShootingGallery.h</b> deklaruje cala klase gry: struktury (Target, Obstacle...), "
               "wszystkie pola i metody. To 'kontrakt' - ja go pisze, a koledzy implementuja swoje "
               "metody (buildRoom, buildObstacles, spawnWave) w swoich plikach .cpp. Dzieki temu "
               "mozemy pracowac rownolegle bez kolizji w gicie."))

# ============ 6. CMAKE ============
story.append(H1("5. CMakeLists.txt - jak to sie kompiluje"))
story.append(P("CMake zbiera wszystkie pliki .cpp, znajduje OpenGL i FreeGLUT (rozne sciezki dla "
               "Windows/macOS/Linux) i tworzy plik wykonywalny. Dodatkowo kopiuje folder assets "
               "(tekstury BMP) i freeglut.dll obok exe, zeby gra je znalazla."))
story += CODE(
"""set(ENGINE_SOURCES Engine/Engine.cpp Engine/Math3D.cpp ...)
set(DEMO_SOURCES   Demo/Room.cpp Demo/Obstacles.cpp
                   Demo/Targets.cpp Demo/Gameplay.cpp)
add_executable(${PROJECT_NAME} main.cpp ${ENGINE_SOURCES} ${DEMO_SOURCES})
# osobny target kopiuje assets/ przy KAZDYM buildzie (ALL)
add_custom_target(copy_assets ALL COMMAND ... copy_directory assets ...)""",
    "CMakeLists.txt - szkielet")

# ============ 7. Q&A ============
story.append(PageBreak())
story.append(H1("6. Pytania od wykladowcy - przygotowanie"))

story.append(H2("Engine - petla i czas"))
story.append(QA("Jak dziala petla glowna w waszym silniku?",
    "FreeGLUT po glutMainLoop wola zarejestrowane callbacki. timerCallback (onTimer) odpala sie co "
    "1000/targetFPS ms - tam licze dt, aktualizuje logike i wolam glutPostRedisplay, ktore prosi "
    "o przerysowanie (displayCallback -> render). Potem planuje kolejny timer. Tak kreci sie "
    "petla 'licz -> rysuj'."))
story.append(QA("Co to jest dt i dlaczego mnozysz przez nie ruch?",
    "dt to czas od poprzedniej klatki w sekundach. Mnozac predkosc przez dt, dostaje faktyczny "
    "dystans (droga = predkosc * czas). Dzieki temu przy 30 i 120 FPS gracz porusza sie tak samo "
    "szybko - inaczej przy wyzszym FPS ruszalby sie szybciej."))
story.append(QA("Jak realizujecie zmienna szybkosc odswiezania?",
    "targetFPS steruje interwalem timera (1000/targetFPS ms). Klawisze +/- zmieniaja go o 10. "
    "Pokazuje go na HUD obok zmierzonego FPS. To pokazuje, ze petla jest sterowalna."))
story.append(QA("Jak dziala pauza?",
    "Flaga paused. W onTimer, gdy paused, pomijam logike gry (nie wolam updateCallback), ale dalej "
    "rysuje (zeby pokazac napis PAUZA). Aktualizuje lastTickMs, zeby po wznowieniu dt nie bylo "
    "ogromne. ESC przelacza pauze, Q w pauzie wychodzi."))

story.append(H2("Engine - wejscie i callbacki"))
story.append(QA("Dlaczego callbacki GLUT sa statyczne?",
    "FreeGLUT jest biblioteka C - przyjmuje wskazniki na zwykle funkcje, nie metody klasy (te maja "
    "ukryty wskaznik this). Dlatego robie statyczne funkcje, ktore przez globalny wskaznik "
    "'instance' wolaja prawdziwe metody obiektu Engine. To typowy mostek C/C++."))
story.append(QA("Jak dziala free mouse look?",
    "Chowam kursor i po kazdym ruchu liczę przesuniecie wzgledem srodka ekranu, dodaje do "
    "yaw/pitch, a potem przesuwam kursor z powrotem na srodek (glutWarpPointer). Dzieki temu kursor "
    "nigdy nie dochodzi do krawedzi i mozna obracac sie bez konca."))
story.append(QA("Po co callbacki (std::function) zamiast wpisac logike gry wprost w silnik?",
    "Zeby silnik byl niezalezny od konkretnej gry. Engine wie tylko 'jest nowa klatka' i wola "
    "podana funkcje. Mozna podpiac dowolna gre bez zmiany silnika. To luzne sprzezenie - dobra "
    "praktyka projektowa."))
story.append(QA("Co to jest CameraControlMode?",
    "Przelacznik kto rzadzi kamera. ENGINE_ORBIT - silnik sam obraca kamera (tryb testowy). "
    "GAME_CONTROLLED - silnik nie rusza kamery, robi to gra w swoim onUpdate (FPP gracza). Gra "
    "ustawia GAME_CONTROLLED."))

story.append(H2("Gameplay - logika i strzelanie"))
story.append(QA("Wytlumacz krok po kroku co dzieje sie przy strzale.",
    "1) Odswiezam kamere najnowszym yaw/pitch. 2) Biore origin (oko) i dir (kierunek patrzenia). "
    "3) Dla kazdego celu robie raySphereIntersect i szukam najblizszego trafionego (najmniejsze t). "
    "4) Sprawdzam, czy zadna przeszkoda nie jest blizej (obstacleBlocksRay) - jak tak, to pudlo. "
    "5) Trafienie: +1 punkt, znika cel, bonus +2s dla reszty fali. Pudlo: -1 zycie."))
story.append(QA("Dlaczego odswiezasz kamere przed raycastem - to byl wasz bug?",
    "Tak. Kamera (yaw/pitch) byla aktualizowana raz na klatke w onUpdate. Ale gracz mogl ruszyc "
    "mysz i wcisnac spacje MIEDZY klatkami - wtedy raycast szedl w stary kierunek, mimo ze "
    "celownik wskazywal nowy. Strzaly mijaly cel. Naprawa: tuz przed raycastem ustawiam kamere "
    "aktualnymi katami."))
story.append(QA("Co to jest HIT_TOLERANCE?",
    "Mnoznik (1.4), ktorym powiekszam promien celu tylko na potrzeby trafienia. Dzieki temu mozna "
    "celowac troche obok srodka i nadal trafic - gra jest mniej frustrujaca. Wizualnie cel zostaje "
    "tej samej wielkosci."))
story.append(QA("Jak dziala bonus +2s po trafieniu?",
    "Gdy w fali zostaja jeszcze cele, do kazdego dodaje 2 sekundy zycia - przez przesuniecie ich "
    "spawnTime do przodu. Timeout liczy sie jako totalTime - spawnTime, wiec przesuniecie spawnTime "
    "odracza wygasniecie. Prosty trik bez dodatkowego pola."))

story.append(H2("Constants / Header / CMake"))
story.append(QA("Dlaczego stale sa constexpr w naglowku, a nie #define?",
    "constexpr ma typ i zasieg (namespace sg), jest bezpieczniejsze niz #define (zwykla podmiana "
    "tekstu). W C++17 constexpr w namespace jest niejawnie inline, wiec naglowek mozna wlaczac w "
    "wielu plikach .cpp bez bledu wielokrotnej definicji."))
story.append(QA("Po co rozdzielacie .h (deklaracja) od .cpp (implementacja)?",
    "Naglowek to 'kontrakt' - mowi jakie sa klasy i metody. Implementacja jest osobno. Dzieki temu "
    "4 osoby moga pisac rozne pliki .cpp tej samej klasy rownolegle, nie wchodzac sobie w droge, a "
    "kompilator wie z naglowka, co istnieje."))
story.append(QA("Dlaczego assets kopiujecie osobnym targetem z ALL, a nie POST_BUILD?",
    "POST_BUILD odpala sie tylko gdy exe jest przebudowywane. Jak dodam nowy plik BMP, ale kod sie "
    "nie zmienil, exe nie jest linkowane i kopiowanie sie nie uruchamia. Osobny target z ALL "
    "wykonuje sie przy kazdym buildzie, wiec nowe tekstury zawsze trafiaja obok exe."))
story.append(QA("Jak CMake radzi sobie z roznymi systemami?",
    "Sprawdzam zmienne APPLE / WIN32. Na Windows uzywam vcpkg lub MSYS2 (ucrt64), na macOS "
    "frameworka OpenGL i Homebrew, na Linux find_package(GLUT). Dzieki temu ten sam projekt "
    "kompiluje sie na 3 systemach."))

build("Jakub_Osoba4.pdf", "Jakub (Osoba 4) - Silnik, integracja, logika gry", story)
