# -*- coding: utf-8 -*-
from _pdfkit import *

story = []
story += title_block(
    "Kacper",
    "Osoba 3",
    "Kamera, oswietlenie, tekstury, cele",
    "Engine/Camera.*, Engine/Light.*, Engine/Texture.*, Demo/Targets.cpp, assets/textures/"
)

# ============ MAPA ============
story.append(H1("0. Gdzie sa moje pliki (mapa do prezentacji)"))
story.append(P("Strzalka <b>&lt;&lt;&lt;</b> pokazuje pliki, ktore robilem ja (Kacper)."))
story += TREE(
"""pgk/
|
+-- Engine/
|   +-- Math3D.* SceneNode.*           (Lukasz)
|   +-- PrimitiveNode.*                 (Rogert)
|   +-- Camera.h / Camera.cpp   <<< MOJE  (kamera FPP i orbita)
|   +-- Light.h  / Light.cpp    <<< MOJE  (swiatlo + material Phonga)
|   +-- Texture.h/ Texture.cpp  <<< MOJE  (BMP + tekstury proceduralne)
|   +-- Engine.*                        (Jakub)
|
+-- Demo/
|   +-- Room.cpp                        (Lukasz)
|   +-- Obstacles.cpp                   (Rogert)
|   +-- Targets.cpp            <<< MOJE  (cele, fale, ruch celow)
|   +-- Gameplay.cpp Constants.h ...    (Jakub)
|
+-- assets/textures/*.bmp      <<< MOJE  (pliki tekstur + instrukcja)
""")
story.append(P("<b>W skrocie:</b> ja zajmuje sie tym, jak scena <b>wyglada</b> i jak <b>patrzymy</b> "
               "na nia. Kamera (skad i w ktora strone), oswietlenie (model Phonga), tekstury "
               "(wczytane z BMP albo wygenerowane wzorami). W grze robie <b>cele</b> - ich "
               "pojawianie sie, ruch i animacje."))

# ============ 1. WPROWADZENIE ============
story.append(PageBreak())
story.append(H1("1. O co chodzi w moich plikach (po ludzku)"))
story.append(P("<b>Kamera</b> decyduje, co widzimy. Mam dwa tryby: 'orbita' (kamera krazy wokol "
               "punktu) i 'pierwsza osoba' (FPP - oko stoi w miejscu, a my rozgladamy sie myszka). "
               "W grze uzywamy FPP."))
story.append(P("<b>Swiatlo</b> i <b>material</b> razem daja realistyczne cieniowanie (model Phonga: "
               "ambient + diffuse + specular). <b>Tekstury</b> to obrazki nalepiane na obiekty - "
               "albo wczytane z pliku BMP (sam napisalem parser), albo wygenerowane algorytmem "
               "(szachownica, drewno, cegly, marmur, szum)."))
story.append(P("W <b>Targets.cpp</b> robie cele do strzelania: losowe pojawianie sie, ruch tam i "
               "z powrotem, obrot i kolysanie."))

# ============ 2. KAMERA ============
story.append(H1("2. Camera - dwa tryby patrzenia"))
story.append(P("Kamera musi wyprodukowac <b>macierz widoku</b> - mowi OpenGL, gdzie jest oko i "
               "w ktora strone patrzy. W trybie FPP licze kierunek patrzenia z dwoch katow: "
               "<b>yaw</b> (obrot lewo-prawo) i <b>pitch</b> (gora-dol)."))
story += CODE(
"""Vec3 Camera::lookDirection() const {
    if (cameraMode == Mode::FIRST_PERSON) {
        return Vec3( sin(yaw)*cos(pitch),   // X
                     sin(pitch),            // Y (gora/dol)
                    -cos(yaw)*cos(pitch) ); // Z (do przodu = -Z)
    }
    return normalize(orbitTarget - eyePosition());  // tryb orbity
}

Mat4 Camera::viewMatrix() const {
    Vec3 eye    = eyePosition();
    Vec3 center = (cameraMode == Mode::FIRST_PERSON)
                ? (eye + lookDirection())   // FPP: patrz przed siebie
                :  orbitTarget;             // orbita: patrz na cel
    return Mat4::lookAt(eye, center, Vec3(0,1,0));
}""", "Camera.cpp - kierunek patrzenia i macierz widoku")
story.append(P("Funkcje lookAt napisal Lukasz - ja jej uzywam, podajac pozycje oka i punkt, na "
               "ktory patrzymy. Ograniczam pitch do +/- 85 stopni, zeby nie dalo sie 'przewrocic' "
               "kamery do gory nogami."))

# ============ 3. SWIATLO ============
story.append(H1("3. Light - oswietlenie Phonga"))
story.append(P("Korzystam z oswietlenia OpenGL (GL_LIGHT0..7). Material ma 3 skladniki:"))
story += BULLET([
    "<b>ambient</b> - kolor w cieniu (swiatlo otoczenia, zawsze obecne),",
    "<b>diffuse</b> - glowny kolor obiektu pod swiatlem rozproszonym,",
    "<b>specular</b> - blysk/odbicie, plus <b>shininess</b> mowiacy jak ostry jest blysk.",
])
story += CODE(
"""void PointLight::renderSelf(const Mat4& worldMatrix) const {
    GLenum lightId = GL_LIGHT0 + activeLightIndex;
    if (!isEnabled) { glDisable(lightId); return; }

    glPushMatrix();
    glMultMatrixf(worldMatrix.data());
    GLfloat position[4] = { 0,0,0, 1.0f };   // 1.0 = swiatlo punktowe
    glEnable(lightId);
    glLightfv(lightId, GL_POSITION, position);
    glLightfv(lightId, GL_DIFFUSE,  diffuse);
    // ... ambient, specular, attenuation ...
    glPopMatrix();
}""", "Light.cpp - ustawienie swiatla w danej pozycji sceny")
story.append(P("Pozycja swiatla to (0,0,0) <b>po</b> nalozeniu macierzy swiata wezla - czyli "
               "swiatlo swieci z miejsca, gdzie stoi jego wezel. Czwarta wspolrzedna = 1 oznacza "
               "swiatlo punktowe (gdyby 0, byloby kierunkowe jak slonce). <b>Attenuation</b> to "
               "tlumienie - im dalej, tym ciemniej, wedlug wzoru 1/(kc + kl*d + kq*d^2)."))
story.append(P("<b>Wazne:</b> swiatla musza byc w grafie sceny <b>przed</b> geometria, bo "
               "renderRecursive idzie po kolei. Gdyby sciana byla przed swiatlem, nie zostalaby "
               "nim oswietlona."))

# ============ 4. TEKSTURY ============
story.append(PageBreak())
story.append(H1("4. Texture - wczytywanie i generowanie obrazkow"))

story.append(H2("4.1. Wlasny parser BMP"))
story.append(P("Napisalem reczny loader 24-bitowych BMP. Czyta naglowki bajt po bajcie, sprawdza "
               "format i przerabia piksele. Dwie pulapki BMP:"))
story += BULLET([
    "piksele sa w kolejnosci <b>BGR</b> (nie RGB) - trzeba zamienic miejscami R i B,",
    "wiersze sa zapisane <b>od dolu do gory</b> - trzeba je odwrocic,",
    "kazdy wiersz jest <b>dosztukowany do wielokrotnosci 4 bajtow</b> (padding).",
])
story += CODE(
"""// konwersja BGR -> RGB z odwroceniem wierszy
for (int row = 0; row < h; ++row) {
    int srcRow = topDown ? row : (h - 1 - row);   // odwrocenie
    for (int col = 0; col < w; ++col) {
        int src = srcRow * rowStride + col * 3;    // rowStride = padding 4B
        int dst = (row * w + col) * 3;
        pixels[dst+0] = raw[src+2];  // R (z pozycji B)
        pixels[dst+1] = raw[src+1];  // G
        pixels[dst+2] = raw[src+0];  // B (z pozycji R)
    }
}""", "Texture.cpp - sedno parsera BMP")

story.append(H2("4.2. Tekstury proceduralne (bez plikow)"))
story.append(P("Mam 7 generatorow: szachownica, gradient, paski, szum (Perlin/FBM), drewno, "
               "cegly, marmur. To czysta matematyka na pikselach. Przyklad - drewno to "
               "koncentryczne kregi zaburzone szumem:"))
story += CODE(
"""float turb = fbm2D(fx*2.5f, fy*2.5f, 4, seed) - 0.5f;  // zaburzenie
float dist = sqrt(fx*fx + fy*fy) + turb*0.20f;          // odleglosc od srodka
float ring = dist * rings;
float fract = ring - floor(ring);    // pozycja w obrebie sloja (0..1)
// jasne tlo, ciemne sloje - interpolacja koloru wg fract""",
    "Texture.cpp - generateWood (sloje drewna)")
story.append(P("<b>fbm2D</b> (Fractional Brownian Motion) sumuje kilka 'oktaw' szumu o coraz "
               "drobniejszej skali - daje naturalny, samopodobny wzor. To moja wlasna implementacja "
               "value noise (losowe wartosci na siatce + plynna interpolacja smoothstep)."))

story.append(H2("4.3. Wlasne mipmapy"))
story.append(P("Mipmapy to pomniejszone wersje tekstury (1/2, 1/4, ...). OpenGL uzywa ich dla "
               "obiektow daleko, zeby nie migotaly. Generuje je recznie - kazdy poziom to "
               "usrednienie blokow 2x2 z poprzedniego."))
story += CODE(
"""while (mipW > 1 || mipH > 1) {
    int newW = max(1, mipW/2), newH = max(1, mipH/2);
    // dla kazdego piksela: srednia z 4 pikseli (2x2) z wyzszego poziomu
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, newW, newH, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, dst.data());
    mipW = newW; mipH = newH; ++level;
}""", "Texture.cpp - reczne generowanie mipmap")

# ============ 5. TARGETS ============
story.append(PageBreak())
story.append(H1("5. Demo/Targets.cpp - cele do strzelania"))
story.append(P("Cele to sfery, ktore pojawiaja sie w falach po 1-3 sztuki. Trzymam <b>pule</b> "
               "3 gotowych sfer i tylko chowam/pokazuje je (setVisible), zamiast tworzyc i niszczyc "
               "- to szybsze i prostsze."))
story.append(H2("5.1. Ruch celu"))
story += CODE(
"""Vec3 ShootingGallery::currentTargetPos(const Target& t) const {
    Vec3 pos = t.basePos;
    if (t.moveRange > 0.01f)   // cel rusza sie tam i z powrotem
        pos += t.moveAxis * (t.moveRange * sin(totalTime_*t.moveSpeed + t.movePhase));
    pos.y += 0.18f * sin(t.bobPhase * 2.2f);   // delikatne kolysanie w pionie
    return pos;
}""", "Targets.cpp - pozycja celu liczona z sinusa (ruch ping-pong)")
story.append(P("Sinus daje plynny ruch tam i z powrotem. <b>movePhase</b> jest losowe, zeby cele "
               "nie ruszaly sie zsynchronizowane. Ta sama funkcja jest uzywana przy rysowaniu i "
               "przy strzale - dzieki temu strzal trafia tam, gdzie cel naprawde jest w danej "
               "chwili."))
story.append(H2("5.2. Bezpieczne pojawianie"))
story.append(P("Przy losowaniu pozycji celu sprawdzam: czy nie jest za blisko gracza, czy nie jest "
               "w srodku przeszkody (uzywam isInsideObstacle Rogerta, z marginesem na caly zakres "
               "ruchu) i czy nie nachodzi na inny cel. Probuje do 60 razy, zanim dam za wygrana."))

# ============ 6. Q&A ============
story.append(PageBreak())
story.append(H1("6. Pytania od wykladowcy - przygotowanie"))
story.append(P("Przy kazdym pytaniu jest pasek <b>Gdzie</b> - mowi w ktorym pliku i w ktorej "
               "funkcji szukac odpowiedzi, zebys w trakcie obrony szybko otworzyl wlasciwe miejsce."))

story.append(H2("Kamera - tryby"))
story.append(QA("Czym rozni sie tryb orbity od pierwszej osoby?",
    "W orbicie oko krazy wokol stalego punktu (orbitTarget) - dobre do ogladania obiektu. W FPP oko "
    "stoi w miejscu (fpEye), a yaw/pitch obracaja kierunek patrzenia - jak w grach FPS. W grze "
    "uzywamy FPP. Tryb trzyma pole cameraMode.",
    "Camera.h -> enum Mode; Camera.cpp -> eyePosition(), viewMatrix()"))
story.append(QA("Jak z katow yaw i pitch liczysz kierunek patrzenia?",
    "To wspolrzedne sferyczne: X=sin(yaw)*cos(pitch), Y=sin(pitch), Z=-cos(yaw)*cos(pitch). Przy "
    "yaw=0,pitch=0 patrzymy w -Z (do przodu w OpenGL). yaw obraca w poziomie, pitch w pionie. "
    "cos(pitch) skaluje skladowe poziome, zeby caly wektor mial dlugosc 1.",
    "Camera.cpp -> lookDirection() (galaz FIRST_PERSON)"))
story.append(QA("Jak liczysz pozycje oka w trybie orbity?",
    "eye = target + (cos(pitch)*sin(yaw), sin(pitch), cos(pitch)*cos(yaw)) * distance. Czyli punkt "
    "na sferze o promieniu distance wokol targetu, wyznaczony przez dwa katy. W FPP po prostu "
    "zwracam fpEye.",
    "Camera.cpp -> eyePosition()"))
story.append(QA("Jak budujesz macierz widoku i czym rozni sie center w obu trybach?",
    "Wolam Mat4::lookAt(eye, center, up). W FPP center = eye + lookDirection() (patrzymy przed "
    "siebie). W orbicie center = orbitTarget (patrzymy na staly punkt). Sama lookAt to kod Lukasza.",
    "Camera.cpp -> viewMatrix(); Math3D.cpp -> Mat4::lookAt()"))
story.append(QA("Dlaczego ograniczasz pitch do +/- 85 stopni?",
    "Zeby nie dalo sie patrzec idealnie w gore/dol i 'przekrecic' kamery do gory nogami. Przy 90 "
    "stopniach wektor patrzenia stalby sie rownolegly do osi 'up', co psuje lookAt - cross(forward,"
    "up) daloby wektor zerowy (degeneracja).",
    "Engine.cpp -> handleMouseMotion() (clamp pitch); ograniczenie dotyczy yaw/pitch silnika"))
story.append(QA("Czemu w setOrbit/setFirstPerson zmieniasz cameraMode?",
    "Bo te metody jednoznacznie okreslaja tryb. Jak ktos wola setFirstPerson, na pewno chce FPP - "
    "wiec ustawiam cameraMode=FIRST_PERSON. To zapobiega pomyłce, gdzie zostalby stary tryb i "
    "kamera liczylaby widok wedlug zlych pol.",
    "Camera.cpp -> setOrbit(), setFirstPerson()"))

story.append(H2("Oswietlenie - model Phonga"))
story.append(QA("Wytlumacz model Phonga (3 skladniki).",
    "Kolor punktu to suma: ambient (stale swiatlo otoczenia, widoczne nawet w cieniu), diffuse "
    "(zalezny od kata miedzy normalna a kierunkiem swiatla - rozproszenie, glowny kolor) i specular "
    "(jasny blysk odbicia, ostrosc sterowana przez shininess). OpenGL liczy to za nas, my podajemy "
    "parametry materialu (glMaterialfv) i swiatla (glLightfv).",
    "Light.h -> struct Material; Light.cpp -> renderSelf(); PrimitiveNode.cpp -> glMaterialfv"))
story.append(QA("Co to jest shininess i jak wplywa na blysk?",
    "To wykladnik w czesci specular modelu Phonga. Maly shininess = szeroki, rozmyty blysk (mat). "
    "Duzy = maly, ostry punkt swiatla (polysk, jak metal/plastik). Ustawiam go per material - np. "
    "cele maja wysoki, sciany niski.",
    "Light.cpp -> glMaterialf(..., GL_SHININESS, ...); wartosci w Material"))
story.append(QA("Co oznacza czwarta wspolrzedna pozycji swiatla (w=1)?",
    "w=1 to swiatlo punktowe - ma konkretna pozycje i swieci we wszystkie strony. w=0 to swiatlo "
    "kierunkowe (jak slonce, w nieskonczonosci) - liczy sie tylko kierunek, nie pozycja. My "
    "uzywamy punktowych, wiec position[3] = 1.0f.",
    "Light.cpp -> renderSelf() (GLfloat position[4] = {0,0,0, 1.0f})"))
story.append(QA("Czemu pozycja swiatla to (0,0,0)? Przeciez lampa jest gdzies w pokoju.",
    "Bo najpierw robie glMultMatrixf(worldMatrix), ktora przenosi uklad do miejsca wezla swiatla. "
    "Wtedy (0,0,0) w tym ukladzie = faktyczna pozycja lampy w swiecie. OpenGL przeksztalca pozycje "
    "swiatla aktualna macierza modelview, wiec to dziala.",
    "Light.cpp -> renderSelf() (glMultMatrixf przed glLightfv POSITION)"))
story.append(QA("Co to jest attenuation (tlumienie) i jaki ma wzor?",
    "Spadek jasnosci z odlegloscia: 1/(kc + kl*d + kq*d^2), gdzie d to odleglosc od swiatla, kc "
    "stala, kl liniowa, kq kwadratowa. Mniejsze wspolczynniki = swiatlo siega dalej. Ustawiamy "
    "male wartosci, zeby caly pokoj byl rownomiernie oswietlony.",
    "Light.cpp -> renderSelf() (GL_CONSTANT/LINEAR/QUADRATIC_ATTENUATION)"))
story.append(QA("Dlaczego swiatla musza byc dodane do sceny przed geometria?",
    "Bo renderRecursive (Lukasz) przechodzi wezly po kolei i ustawia stan OpenGL na biezaco. "
    "Swiatlo ustawia glLight dopiero gdy do niego dojdziemy. Geometria narysowana wczesniej nie "
    "'widzi' jeszcze tego swiatla. Dlatego w grze dodaje swiatla jako pierwsze dzieci roota.",
    "Light.cpp -> renderSelf(); kolejnosc w Gameplay.cpp (addChild swiatel najpierw)"))
story.append(QA("Jak obsluguje wiele swiatel naraz?",
    "Kazde swiatlo ma activeLightIndex (0..7), z ktorego licze GLenum lightId = GL_LIGHT0 + index. "
    "Dzieki temu dwie lampy (index 0 i 1) nie nadpisuja sie - kazda steruje innym GL_LIGHTx. OpenGL "
    "obsluguje do 8 swiatel jednoczesnie.",
    "Light.cpp -> renderSelf() (GL_LIGHT0 + activeLightIndex); setLightIndex()"))

story.append(H2("Tekstury - parser BMP"))
story.append(QA("Jakie formaty BMP obsluguje twoj loader?",
    "Tylko 24-bitowe nieskompresowane (BI_RGB, kompresja=0). Sprawdzam to czytajac z naglowka DIB "
    "bpp (bits per pixel) i compression. Jezeli to nie 24-bit bez kompresji, zwracam false i gra "
    "uzywa tekstury proceduralnej. Inne formaty (32-bit, RLE) sa odrzucane.",
    "Texture.cpp -> loadBMP() (sprawdzenie bpp != 24 || compression != 0)"))
story.append(QA("Dlaczego w BMP zamieniasz R z B?",
    "Bo BMP zapisuje piksele w kolejnosci BGR (niebieski, zielony, czerwony), a OpenGL chce RGB. "
    "Przy kopiowaniu biore raw[src+2] (B) na pozycje R i raw[src+0] (R) na pozycje B. G zostaje w "
    "srodku.",
    "Texture.cpp -> loadBMP() (petla pixels[dst+0]=raw[src+2] itd.)"))
story.append(QA("Czemu odwracasz wiersze obrazu?",
    "BMP z dodatnia wysokoscia trzyma wiersze od dolu do gory (pierwszy wiersz pliku to dol obrazu). "
    "OpenGL oczekuje od gory. Wiec srcRow = topDown ? row : (h-1-row) - dla zwyklego BMP czytam od "
    "konca. Ujemna wysokosc w BMP oznacza juz top-down i wtedy nie odwracam.",
    "Texture.cpp -> loadBMP() (srcRow = topDown ? row : h-1-row)"))
story.append(QA("Co to jest rowStride / padding w BMP?",
    "Kazdy wiersz BMP jest dopelniany do wielokrotnosci 4 bajtow. rowStride = (w*3+3) & ~3 to "
    "faktyczna dlugosc wiersza w pliku (z dopelnieniem). Bez uwzglednienia tego obraz by sie "
    "'przesuwal' skosnie, bo zle bym liczyl poczatek kazdego wiersza.",
    "Texture.cpp -> loadBMP() (rowStride = (w*3+3) & ~3)"))
story.append(QA("Skad wiesz, gdzie w pliku zaczynaja sie piksele?",
    "Z naglowka pliku (14 bajtow) czytam dataOffset (4 bajty od pozycji 10) - to przesuniecie do "
    "danych pikseli. Robie fseek(f, dataOffset) zanim zaczne czytac piksele. Dzieki temu dziala "
    "nawet jak naglowek ma niestandardowy rozmiar.",
    "Texture.cpp -> loadBMP() (dataOffset, fseek)"))

story.append(H2("Tekstury - generatory proceduralne"))
story.append(QA("Jak dziala generateCheckerboard (szachownica)?",
    "Dla kazdego piksela licze ktore pole szachownicy: (x/tileSize + y/tileSize) % 2. Parzyste = "
    "kolor 1, nieparzyste = kolor 2. tileSize = size/tileCount. To czysta arytmetyka na indeksach "
    "pikseli, bez zadnego szumu.",
    "Texture.cpp -> generateCheckerboard()"))
story.append(QA("Czym jest value noise i FBM w teksturach proceduralnych?",
    "Value noise: losowe wartosci w rogach calkowitej kraty, wnetrze interpolowane plynnie "
    "(smoothstep). FBM (Fractional Brownian Motion) sumuje kilka oktaw takiego szumu o coraz "
    "wyzszej czestotliwosci i nizszej amplitudzie - daje naturalny, samopodobny wzor. Uzywam w "
    "drewnie, marmurze, cegłach, sufiecie.",
    "Texture.cpp -> valueNoise2D(), fbm2D() (anonimowy namespace)"))
story.append(QA("Jak generujesz drewno (sloje)?",
    "Licze odleglosc piksela od srodka tekstury, dodaje do niej zaburzenie z fbm (turbulencja). Z "
    "tej odleglosci robie pile (dist*rings, czesc ulamkowa) - to daje koncentryczne kregi. "
    "Interpoluje kolor miedzy ciemnym slojem a jasnym tlem wg pozycji w kregu.",
    "Texture.cpp -> generateWood()"))
story.append(QA("Jak generujesz cegly?",
    "Dziele teksture na rzedy (rowHeight) i cegly (brickWidth = 2*rowHeight). Co drugi rzad "
    "przesuwam o pol cegly (naprzemienny uklad). Tam gdzie jest fuga (mortarPx) maluje kolor "
    "zaprawy, reszta to cegla z drobna wariacja koloru (hash per cegla + mikroszum). To daje "
    "realistyczny mur.",
    "Texture.cpp -> generateBricks()"))
story.append(QA("Jak generujesz marmur?",
    "Licze turbulencje (suma modulow szumu o malejacej amplitudzie), potem zyly = sin((x + "
    "turbulencja*sila)*2pi). Modul i pierwiastek (pow 0.5) wyostrzaja przejscia. To daje wzor "
    "falujacych zyl typowy dla marmuru.",
    "Texture.cpp -> generateMarble()"))
story.append(QA("Co to sa mipmapy i jak je liczysz?",
    "To pomniejszone kopie tekstury (1/2, 1/4...). Gdy obiekt jest daleko, OpenGL uzywa mniejszej "
    "wersji - inaczej tekstura migocze. Licze je recznie w petli: kazdy poziom to srednia z blokow "
    "2x2 poprzedniego poziomu, wgrywana przez glTexImage2D z numerem poziomu (level).",
    "Texture.cpp -> uploadToGPU() (petla while mipW>1 || mipH>1)"))
story.append(QA("Dlaczego klasa Texture ma usuniety konstruktor kopiujacy?",
    "Bo trzyma uchwyt GL (textureId). Kopia miala by ten sam id - i destruktor zwolnilby teksture "
    "dwa razy (glDeleteTextures na tym samym id), co jest bledem. Dlatego = delete na kopiowaniu; "
    "teksture trzymam przez shared_ptr.",
    "Texture.h -> Texture(const Texture&) = delete;"))

story.append(H2("Targets - cele"))
story.append(QA("Po co pula celow zamiast tworzenia sfer na biezaco?",
    "Tworzenie i niszczenie obiektow OpenGL co chwile jest kosztowne i ryzykowne. Lepiej raz "
    "stworzyc MAX_TARGETS (3) sfery i tylko je chowac (setVisible false) i pokazywac. To wzorzec "
    "'object pool'. Kazdy aktywny cel ma nodeIndex wskazujacy ktora sfera go reprezentuje.",
    "Targets.cpp -> initTargetPool(); ShootingGallery.h -> targetNodePool_"))
story.append(QA("Dlaczego ruch celu liczysz sinusem?",
    "Sinus daje gladki ruch tam i z powrotem (ping-pong) bez skokow na koncach. pos = basePos + "
    "moveAxis * moveRange * sin(totalTime*moveSpeed + movePhase). Dodatkowo lekkie kolysanie w "
    "pionie (drugi sinus na Y).",
    "Targets.cpp -> currentTargetPos()"))
story.append(QA("Po co kazdy cel ma losowa faze (movePhase)?",
    "Zeby cele w jednej fali nie ruszaly sie idealnie zsynchronizowane (co wygladaloby sztucznie). "
    "Losowa faza przesuwa sinus kazdego celu, wiec poruszaja sie niezaleznie.",
    "Targets.cpp -> spawnWave() (movePhase = distR(rng_)*6.28); currentTargetPos()"))
story.append(QA("Czemu ta sama funkcja liczy pozycje przy rysowaniu i przy strzale?",
    "Zeby strzal trafial dokladnie tam, gdzie cel jest w danej klatce. Gdybym liczyl pozycje inaczej "
    "do rysowania, a inaczej do kolizji, celowanie byloby niesprawiedliwe - widzialbym cel gdzie "
    "indziej niz jest 'naprawde'. currentTargetPos jest wolane i w onUpdate (render) i w onShoot.",
    "Targets.cpp -> currentTargetPos(); uzycie w Gameplay.cpp onUpdate/onShoot"))
story.append(QA("Jak losujesz rozmiar fali (1-3 cele)?",
    "Losuje liczbe r w [0,1). Jezeli r<0.5 -> 1 cel, r<0.85 -> 2 cele, inaczej 3. Czyli najczesciej "
    "1, czasem 2, rzadko 3. Potem dla kazdego celu losuje pozycje i tryb ruchu.",
    "Targets.cpp -> spawnWave() (waveSize_ = r<0.5 ? 1 : ...)"))
story.append(QA("Jak unikasz pojawienia sie celu w scianie albo w przeszkodzie?",
    "Losuje pozycje i sprawdzam 3 warunki: dystans od gracza (>MIN), czy nie w przeszkodzie "
    "(isInsideObstacle z marginesem na CALY zakres ruchu), czy nie za blisko innego celu z fali. "
    "Jezeli ktorys warunek niespelniony, losuje ponownie - do 60 prob. Po 60 stawiam cel statyczny "
    "w bezpiecznym miejscu (fallback).",
    "Targets.cpp -> spawnWave() (petla attempt < 60); isInsideObstacle z Obstacles.cpp"))
story.append(QA("Czemu margines przy spawnie uwzglednia zakres ruchu celu?",
    "Bo cel sie porusza. Gdybym sprawdzal tylko pozycje startowa, ruchomy cel moglby wjechac w "
    "kolumne podczas oscylacji. Margines = TARGET_RADIUS + moveRange + zapas gwarantuje, ze caly "
    "tor ruchu jest wolny od przeszkod.",
    "Targets.cpp -> spawnWave() (margin = TARGET_RADIUS + t.moveRange + 0.25)"))

build("Kacper_Osoba3.pdf", "Kacper (Osoba 3) - Kamera, swiatlo, tekstury, cele", story)
