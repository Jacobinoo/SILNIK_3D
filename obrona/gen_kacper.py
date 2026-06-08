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

story.append(H2("Kamera"))
story.append(QA("Czym rozni sie tryb orbity od pierwszej osoby?",
    "W orbicie oko krazy wokol stalego punktu (target) - dobre do ogladania obiektu. W FPP oko "
    "stoi w miejscu, a yaw/pitch obracaja kierunek patrzenia - tak jak w grach FPS. W grze "
    "uzywamy FPP."))
story.append(QA("Jak z katow yaw i pitch liczysz kierunek patrzenia?",
    "To wspolrzedne sferyczne. X=sin(yaw)*cos(pitch), Y=sin(pitch), Z=-cos(yaw)*cos(pitch). "
    "Przy yaw=0,pitch=0 patrzymy w -Z (do przodu w OpenGL). yaw obraca w poziomie, pitch w pionie. "
    "cos(pitch) skaluje skladowe poziome, zeby wektor zostal jednostkowy."))
story.append(QA("Dlaczego ograniczasz pitch do +/- 85 stopni?",
    "Zeby nie dalo sie patrzec idealnie w gore/dol i 'przekrecic' kamery do gory nogami. Przy 90 "
    "stopniach wektor patrzenia stalby sie rownolegly do osi gory, co psuje lookAt (degeneracja)."))
story.append(QA("Co to jest macierz widoku?",
    "Macierz, ktora przenosi caly swiat do ukladu kamery - tak, jakby kamera byla w poczatku "
    "ukladu i patrzyla w -Z. Buduje ja przez lookAt z pozycji oka i punktu, na ktory patrzymy."))

story.append(H2("Oswietlenie"))
story.append(QA("Wytlumacz model Phonga.",
    "Kolor punktu to suma trzech skladnikow: ambient (stale swiatlo otoczenia, widoczne nawet w "
    "cieniu), diffuse (zalezny od kata miedzy normalna a kierunkiem swiatla - rozproszenie) i "
    "specular (jasny blysk odbicia, sterowany przez shininess). OpenGL liczy to za nas, my "
    "podajemy parametry materialu i swiatla."))
story.append(QA("Co oznacza czwarta wspolrzedna pozycji swiatla (w=1)?",
    "w=1 to swiatlo punktowe - ma konkretna pozycje i swieci we wszystkie strony. w=0 to swiatlo "
    "kierunkowe (jak slonce) - liczy sie tylko kierunek, nie pozycja. My uzywamy punktowych (w=1)."))
story.append(QA("Co to jest attenuation (tlumienie)?",
    "Spadek jasnosci swiatla z odlegloscia, wg wzoru 1/(kc + kl*d + kq*d^2). kc to stala, kl "
    "liniowa, kq kwadratowa. Mniejsze wspolczynniki = swiatlo siega dalej. Ustawiamy male, zeby "
    "caly pokoj byl oswietlony."))
story.append(QA("Dlaczego swiatla musza byc dodane do sceny przed geometria?",
    "Bo renderRecursive przechodzi wezly po kolei i ustawia stan OpenGL na biezaco. Swiatlo "
    "ustawia glLight dopiero gdy do niego dojdziemy. Geometria narysowana wczesniej nie 'widzi' "
    "tego swiatla. Dlatego dodaje swiatla jako pierwsze dzieci roota."))

story.append(H2("Tekstury"))
story.append(QA("Dlaczego w BMP zamieniasz R z B?",
    "Bo BMP zapisuje piksele w kolejnosci BGR (niebieski, zielony, czerwony), a OpenGL chce RGB. "
    "Wiec przy kopiowaniu biore bajt B na pozycje R i odwrotnie."))
story.append(QA("Czemu odwracasz wiersze obrazu?",
    "BMP z dodatnia wysokoscia trzyma wiersze od dolu do gory (pierwszy wiersz pliku to dol "
    "obrazu). OpenGL oczekuje od gory. Wiec wiersz srcRow = h-1-row, czyli czytam od konca."))
story.append(QA("Co to jest rowStride / padding w BMP?",
    "Kazdy wiersz BMP jest dopelniany zerami do wielokrotnosci 4 bajtow. rowStride = (w*3+3) & ~3 "
    "to faktyczna dlugosc wiersza w pliku. Bez uwzglednienia tego obraz by sie 'przesuwal' "
    "skosnie."))
story.append(QA("Co to sa mipmapy i po co je liczysz?",
    "To pomniejszone kopie tekstury (1/2, 1/4...). Gdy obiekt jest daleko, OpenGL uzywa mniejszej "
    "wersji - inaczej tekstura migocze i szumi. Licze je recznie: kazdy poziom to srednia z "
    "blokow 2x2 poprzedniego poziomu."))
story.append(QA("Czym jest FBM / value noise w teksturach proceduralnych?",
    "Value noise to losowe wartosci na siatce, plynnie interpolowane (smoothstep). FBM sumuje "
    "kilka takich szumow o coraz drobniejszej skali i mniejszej sile - daje naturalny, "
    "'chmurkowy' wzor. Uzywam go w drewnie, marmurze, cegłach i sufiecie."))

story.append(H2("Targets"))
story.append(QA("Po co pula celow zamiast tworzenia sfer na biezaco?",
    "Tworzenie i niszczenie obiektow OpenGL co chwile jest kosztowne i ryzykowne. Lepiej raz "
    "stworzyc 3 sfery i tylko je chowac (setVisible false) i pokazywac. To wzorzec 'object pool'."))
story.append(QA("Dlaczego ruch celu liczysz sinusem?",
    "Sinus daje gladki ruch tam i z powrotem (ping-pong) bez skokow na koncach. pos = base + os * "
    "zakres * sin(czas*predkosc + faza). Faza jest losowa, zeby cele nie ruszaly sie rownoczesnie."))
story.append(QA("Czemu ta sama funkcja liczy pozycje przy rysowaniu i przy strzale?",
    "Zeby strzal trafial dokladnie tam, gdzie cel jest w danej klatce. Gdybym liczyl pozycje "
    "inaczej do rysowania, a inaczej do kolizji, celowanie byloby niesprawiedliwe - widzialbym cel "
    "gdzie indziej niz jest 'naprawde'."))
story.append(QA("Jak unikasz pojawienia sie celu w scianie albo w przeszkodzie?",
    "Losuje pozycje i sprawdzam warunki: dystans od gracza, czy nie w przeszkodzie (z marginesem "
    "na caly zakres ruchu), czy nie za blisko innego celu. Jezeli warunek niespelniony, losuje "
    "ponownie - do 60 prob. Margines uwzglednia ruch, wiec cel nie wjedzie w kolumne podczas "
    "oscylacji."))

build("Kacper_Osoba3.pdf", "Kacper (Osoba 3) - Kamera, swiatlo, tekstury, cele", story)
