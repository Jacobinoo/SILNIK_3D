# -*- coding: utf-8 -*-
from _pdfkit import *

story = []
story += title_block(
    "Lukasz",
    "Osoba 1",
    "Matematyka 3D, graf sceny, budowa pokoju",
    "Engine/Math3D.h, Engine/Math3D.cpp, Engine/SceneNode.h, Engine/SceneNode.cpp, Demo/Room.cpp"
)

# ============ MAPA PLIKOW ============
story.append(H1("0. Gdzie sa moje pliki (mapa do prezentacji)"))
story.append(P("Ponizsze drzewo pokazuje caly projekt. Strzalka <b>&lt;&lt;&lt;</b> wskazuje pliki, "
               "ktore robilem ja (Lukasz). W trakcie pokazu otwieraj wlasnie te."))
story += TREE(
"""pgk/
|
+-- Engine/                  <- silnik (rdzen)
|   +-- Math3D.h        <<< MOJE  (wektory, macierze, raycasty)
|   +-- Math3D.cpp      <<< MOJE
|   +-- SceneNode.h     <<< MOJE  (graf sceny - drzewo obiektow)
|   +-- SceneNode.cpp   <<< MOJE
|   +-- Camera.* Light.* Texture.*   (Kacper)
|   +-- PrimitiveNode.*               (Rogert)
|   +-- Engine.*                      (Jakub)
|
+-- Demo/                    <- gra
|   +-- Room.cpp        <<< MOJE  (sciany + dekoracje pokoju)
|   +-- Obstacles.cpp                 (Rogert)
|   +-- Targets.cpp                   (Kacper)
|   +-- Gameplay.cpp Constants.h ShootingGallery.h  (Jakub)
|
+-- main.cpp CMakeLists.txt README.md  (Jakub)
""")
story.append(P("<b>Najwazniejsze do zapamietania:</b> ja odpowiadam za <b>matematyke</b> "
               "(wszystko co liczy wektory i macierze), za <b>strukture sceny</b> "
               "(jak obiekty sa ulozone w drzewo) i za <b>zbudowanie pomieszczenia</b> w grze."))

# ============ 1. WPROWADZENIE ============
story.append(PageBreak())
story.append(H1("1. O co chodzi w moich plikach (po ludzku)"))
story.append(P("Silnik 3D musi cos policzyc, zanim cokolwiek narysuje. Zeby pokazac szescian "
               "w odpowiednim miejscu i pod odpowiednim katem, trzeba mnozyc <b>macierze</b> i "
               "przeksztalcac <b>wektory</b>. To wlasnie robi <b>Math3D</b> - to nasz wlasny, "
               "reczny zestaw narzedzi matematycznych (nie uzywamy gotowej biblioteki jak glm)."))
story.append(P("<b>SceneNode</b> to z kolei sposob, w jaki trzymamy obiekty sceny. Zamiast luznej "
               "listy, obiekty tworza <b>drzewo</b> (graf sceny). Dzieki temu jak ruszymy "
               "rodzicem, dzieci ruszaja sie razem z nim."))
story.append(P("<b>Room.cpp</b> to juz czesc gry - uzywam swoich klas (i prymitywow kolegow), "
               "zeby zbudowac pokoj: podloge, sufit, 4 sciany i dekoracje."))

# ============ 2. MATH3D ============
story.append(H1("2. Math3D.h / Math3D.cpp - matematyka silnika"))

story.append(H2("2.1. Vec3 - wektor 3D"))
story.append(P("Wektor to po prostu trzy liczby: x, y, z. Moze oznaczac <b>punkt</b> w przestrzeni "
               "albo <b>kierunek</b>. Zdefiniowalem podstawowe operacje (dodawanie, odejmowanie, "
               "mnozenie przez liczbe) plus funkcje matematyczne."))
story += CODE(
"""float dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;   // iloczyn skalarny
}

Vec3 cross(const Vec3& a, const Vec3& b) {  // iloczyn wektorowy
    return Vec3(a.y*b.z - a.z*b.y,
                a.z*b.x - a.x*b.z,
                a.x*b.y - a.y*b.x);
}

float length(const Vec3& v) { return std::sqrt(dot(v, v)); }

Vec3 normalize(const Vec3& v) {
    float len = length(v);
    if (len <= 0.000001f) return Vec3(0,0,0);  // ochrona przed dzieleniem przez 0
    return v / len;
}""", "Math3D.cpp - kluczowe operacje na wektorach")
story.append(P("<b>dot</b> (iloczyn skalarny) mowi nam m.in. o kacie miedzy wektorami. "
               "<b>cross</b> (iloczyn wektorowy) daje wektor <b>prostopadly</b> do obu - "
               "uzywam go np. do policzenia osi kamery. <b>normalize</b> skraca wektor do dlugosci 1, "
               "zachowujac kierunek (wektory kierunku prawie zawsze chcemy znormalizowane)."))

story.append(H2("2.2. Mat4 - macierz 4x4 (najwazniejsza rzecz!)"))
story.append(P("Macierz 4x4 to 16 liczb, ktore opisuja przeksztalcenie: przesuniecie, obrot, "
               "skalowanie albo rzutowanie. Trzymam je w tablicy <b>column-major</b> "
               "(kolumnami), bo tego wymaga OpenGL."))
story += CODE(
"""float& Mat4::at(int row, int col) { return m[col * 4 + row]; }
//                                          ^^^^^^^^^^^^^
//   element (row,col) jest pod indeksem col*4+row  =>  column-major""",
"Math3D.cpp - dostep do elementu macierzy")
story.append(P("<b>Dlaczego column-major?</b> Bo funkcja OpenGL <i>glLoadMatrixf</i> czyta dane "
               "wlasnie w tej kolejnosci. Gdybym trzymal je wierszami (row-major), obraz bylby "
               "kompletnie przekrecony. To wazne - wykladowca moze o to spytac."))

story.append(H3("Macierz lookAt (i slynny bug, ktory naprawilismy)"))
story.append(P("<b>lookAt</b> tworzy macierz widoku - ustawia 'oko' kamery w punkcie <i>eye</i>, "
               "kierujac je na <i>center</i>. Liczymy trzy osie kamery: <i>forward</i> (do przodu), "
               "<i>side</i> (w prawo) i <i>trueUp</i> (w gore)."))
story += CODE(
"""Vec3 forward = normalize(center - eye);
Vec3 side    = normalize(cross(forward, up));
Vec3 trueUp  = cross(side, forward);

Mat4 result = Mat4::identity();
result.at(0,0)=side.x;  result.at(0,1)=side.y;  result.at(0,2)=side.z;   // wiersz 0
result.at(1,0)=trueUp.x;result.at(1,1)=trueUp.y;result.at(1,2)=trueUp.z; // wiersz 1
result.at(2,0)=-forward.x; result.at(2,1)=-forward.y; result.at(2,2)=-forward.z;
result.at(0,3)=-dot(side,eye);
result.at(1,3)=-dot(trueUp,eye);
result.at(2,3)= dot(forward,eye);""", "Math3D.cpp - lookAt (wersja poprawna)")
story.append(P("<b>Bug, ktory mielismy:</b> wektory osi byly wpisane jako <b>kolumny</b> zamiast "
               "<b>wierszy</b>. Dla obrotu transpozycja = odwrotnosc, wiec efekt byl taki, ze "
               "kamera 'krazyla wokol jakiegos dziwnego punktu', a strzaly nie trafialy w cele "
               "(bo kierunek patrzenia nie zgadzal sie z tym, co widac). Przy yaw=pitch=0 bug byl "
               "niewidoczny, bo zamieniane pola byly zerami - dlatego dlugo go nie zauwazylismy."))

story.append(H2("2.3. Funkcje przeciec promienia (raycasty)"))
story.append(P("To one sprawiaja, ze w grze mozna w cos 'trafic'. Promien to polprosta: "
               "<b>P(t) = origin + t * kierunek</b>. Szukamy najmniejszego dodatniego t, "
               "przy ktorym promien dotyka obiektu."))
story += CODE(
"""bool raySphereIntersect(const Vec3& origin, const Vec3& dir,
                        const Vec3& center, float radius, float& outT) {
    Vec3  oc = origin - center;
    float a  = dot(dir, dir);
    float b  = dot(oc, dir);
    float c  = dot(oc, oc) - radius*radius;
    float disc = b*b - a*c;            // wyroznik rownania kwadratowego
    if (disc < 0.0f) return false;     // brak przeciecia
    float sqrtD = std::sqrt(disc);
    float t1 = (-b - sqrtD) / a;       // blizsze
    float t2 = (-b + sqrtD) / a;       // dalsze
    if (t1 > 0.0001f) { outT = t1; return true; }
    if (t2 > 0.0001f) { outT = t2; return true; }
    return false;
}""", "Math3D.cpp - przeciecie promienia ze sfera")
story.append(P("To zwykle <b>rownanie kwadratowe</b>: podstawiamy P(t) do rownania sfery "
               "|P - center|^2 = r^2 i wychodzi a*t^2 + 2b*t + c = 0. Jezeli wyroznik (disc) "
               "jest ujemny - promien mija sfere. Mam tez analogiczne funkcje dla walca "
               "(<i>rayCylinderIntersect</i>), stozka (<i>rayConeIntersect</i>) i prostopadloscianu "
               "(<i>rayAABBIntersect</i> - metoda 'slab'). Kacper i Rogert uzywaja ich w grze."))

# ============ 3. SCENENODE ============
story.append(PageBreak())
story.append(H1("3. SceneNode.h / SceneNode.cpp - graf sceny"))
story.append(P("Graf sceny to <b>drzewo</b> obiektow. Kazdy obiekt (SceneNode) ma rodzica i liste "
               "dzieci. Dzieki temu transformacje sie <b>dziedzicza</b>: jak obroce rodzica, to "
               "dzieci obroca sie razem z nim. To standard w silnikach 3D."))

story.append(H2("3.1. Macierz lokalna"))
story += CODE(
"""Mat4 SceneNode::localMatrix() const {
    return Mat4::translation(localPosition)
         * Mat4::rotationY(localRotation.y)
         * Mat4::rotationX(localRotation.x)
         * Mat4::rotationZ(localRotation.z)
         * Mat4::scale(localScale);
}""", "SceneNode.cpp - sklejenie pozycji, obrotu i skali w jedna macierz")
story.append(P("Kolejnosc mnozenia ma znaczenie! Czytajac od prawej: najpierw <b>skalujemy</b> "
               "obiekt, potem <b>obracamy</b> (Z, X, Y), na koncu <b>przesuwamy</b>. Gdyby "
               "przesuniecie bylo pierwsze, obiekt obracalby sie wokol srodka swiata, a nie "
               "wokol siebie."))

story.append(H2("3.2. Rekurencyjne rysowanie"))
story += CODE(
"""void SceneNode::renderRecursive(const Mat4& parentMatrix) const {
    Mat4 world = parentMatrix * localMatrix();  // moja macierz swiata
    if (nodeVisible) renderSelf(world);         // narysuj siebie (jezeli widoczny)
    for (const auto& child : childNodes)        // potem wszystkie dzieci
        child->renderRecursive(world);
}""", "SceneNode.cpp - przejscie po calym drzewie sceny")
story.append(P("Silnik wola te funkcje na korzeniu (root) z macierza jednostkowa. Funkcja schodzi "
               "w dol drzewa, mnozac macierze. <b>renderSelf</b> jest puste w klasie bazowej - "
               "dopiero klasy potomne (szescian, sfera, swiatlo) nadpisuja je i rysuja geometrie. "
               "To przyklad <b>polimorfizmu</b> i wzorca, gdzie klasa bazowa zarzadza struktura, "
               "a potomne definiuja szczegoly."))
story.append(P("<b>nodeVisible</b> - jak ustawimy na false, obiekt sie nie rysuje (ale jego dzieci "
               "tak). Kacper uzywa tego do 'puli celow' - chowamy sfery zamiast je usuwac."))

# ============ 4. ROOM.CPP ============
story.append(H1("4. Demo/Room.cpp - budowa pomieszczenia"))
story.append(P("Tu uzywam wszystkiego powyzej plus prymitywow Rogerta (PlaneNode, SphereNode...). "
               "Buduje pokoj o wymiarach 20 x 6 x 30 metrow: podloge, sufit i 4 sciany, kazda jako "
               "<b>PlaneNode</b> obrocony w odpowiednia strone."))
story += CODE(
"""// sufit: plaszczyzna obrocona o 180 stopni (PI) wokol X, zeby normalna
// patrzyla w dol (do wnetrza pokoju)
ceiling_->setPosition(Vec3(0, ROOM_HEIGHT, ROOM_Z_CENTER));
ceiling_->setRotation(Vec3(PI, 0, 0));
ceiling_->setUVScale(FLOOR_UV_SCALE);
ceiling_->setMaterial(ceilingMat);
ceiling_->setTexture(ceilingTex_);""", "Room.cpp - przyklad jednej sciany (sufit)")
story.append(P("<b>buildDecorations()</b> dorzuca dekoracje: kule i kostki na scianach, torus "
               "(zyrandol) pod sufitem. Uzywam lambd (makeSphere, makeCube, makeCylinder), zeby "
               "nie powtarzac kodu. Dekoracje sa wysoko (y &gt;= 3), wiec gracz pod nimi przechodzi "
               "i nie ma z nimi kolizji."))
story.append(P("Tekstury sa albo wczytane z pliku BMP (jezeli istnieje), albo wygenerowane "
               "proceduralnie - np. <i>generatePerlinNoise</i> dla sufitu. Te funkcje napisal Kacper, "
               "ja ich tylko uzywam."))

# ============ 5. PYTANIA I ODPOWIEDZI ============
story.append(PageBreak())
story.append(H1("5. Pytania od wykladowcy - przygotowanie"))

story.append(H2("Math3D"))
story.append(QA("Dlaczego trzymasz macierz jako tablice 16 floatow, a nie jako tablica 4x4?",
    "Bo OpenGL (glLoadMatrixf / glMultMatrixf) przyjmuje plaska tablice 16 floatow. "
    "Trzymanie ich od razu w takiej formie pozwala podac wskaznik <i>data()</i> wprost do OpenGL "
    "bez konwersji. Dostep (row,col) realizuje funkcja <i>at</i> jako m[col*4+row]."))
story.append(QA("Co to znaczy column-major i dlaczego to wazne?",
    "Column-major znaczy, ze kolejne 4 liczby w pamieci to jedna kolumna macierzy. OpenGL tego "
    "wymaga. Gdyby trzymac dane wierszami (row-major), macierz bylaby transponowana i wszystkie "
    "transformacje bylyby zle - obraz przekrecony."))
story.append(QA("Wyjasnij dzialanie raySphereIntersect linijka po linijce.",
    "Podstawiam promien P(t)=origin+t*dir do rownania sfery |P-center|^2=r^2. Po rozwinieciu "
    "wychodzi rownanie kwadratowe a*t^2+2b*t+c=0, gdzie a=dot(dir,dir), b=dot(oc,dir), "
    "c=dot(oc,oc)-r^2 (oc=origin-center). Licze wyroznik disc=b^2-a*c. Jezeli &lt;0 - brak "
    "przeciecia. Inaczej licze dwa pierwiastki t1,t2 i wybieram najmniejszy dodatni (najblizszy "
    "punkt przed kamera)."))
story.append(QA("Dlaczego sprawdzasz t > 0.0001f, a nie t > 0?",
    "Maly epsilon chroni przed bledami zaokraglen float i przed 'trafieniem' w punkt startowy "
    "promienia (t=0). Bez tego moglyby pojawiac sie falszywe przeciecia tuz przy kamerze."))
story.append(QA("Co robi cross (iloczyn wektorowy) i gdzie go uzywasz?",
    "cross daje wektor prostopadly do dwoch podanych. Uzywam go w lookAt: majac kierunek patrzenia "
    "(forward) i przyblizona gore (up), licze prawo kamery side=cross(forward,up), a potem "
    "dokladna gore trueUp=cross(side,forward). Tak buduje 3 prostopadle osie kamery."))
story.append(QA("Dlaczego w normalize sprawdzasz dlugosc przed dzieleniem?",
    "Zeby nie dzielic przez zero. Wektor zerowy ma dlugosc 0 - dzielenie daloby nieskonczonosc/NaN. "
    "Zwracam wtedy wektor zerowy jako bezpieczna wartosc."))

story.append(H2("SceneNode"))
story.append(QA("Po co graf sceny? Nie wystarczy lista obiektow?",
    "Graf pozwala na hierarchie - obiekt-dziecko dziedziczy transformacje rodzica. Np. swiatlo "
    "doczepione do obiektu rusza sie razem z nim. Mnozenie macierzy rodzic*dziecko robi to "
    "automatycznie. Z plaska lista trzeba by recznie pamietac zaleznosci."))
story.append(QA("Dlaczego renderRecursive mnozy parentMatrix * localMatrix, a nie odwrotnie?",
    "Bo macierz swiata dziecka = macierz swiata rodzica zlozona z lokalna transformacja dziecka. "
    "Kolejnosc rodzic*dziecko sprawia, ze lokalna transformacja dziala 'wewnatrz' ukladu rodzica."))
story.append(QA("Czemu localMatrix mnozy w kolejnosci T * R * S?",
    "Czytajac od prawej (bo tak dzialaja macierze): najpierw skala, potem obrot, na koncu "
    "translacja. Dzieki temu obiekt skaluje i obraca sie wokol wlasnego srodka, a dopiero potem "
    "jest przesuwany na miejsce. Odwrotna kolejnosc daloby obrot wokol srodka swiata."))
story.append(QA("Czym jest renderSelf i dlaczego jest wirtualne?",
    "To metoda, ktora rysuje konkretna geometrie. W SceneNode jest pusta (wezel grupujacy). "
    "Klasy potomne (CubeNode, PointLight...) nadpisuja ja. Wirtualnosc pozwala wywolac wlasciwa "
    "wersje przez wskaznik do klasy bazowej - to polimorfizm."))
story.append(QA("Co daje flaga nodeVisible?",
    "Gdy false, renderSelf jest pomijane (obiekt znika), ale dzieci nadal sie rysuja. Uzywamy "
    "tego do puli celow - zamiast tworzyc i niszczyc sfery, po prostu je chowamy/pokazujemy."))

story.append(H2("Room.cpp"))
story.append(QA("Dlaczego sufit jest obrocony o PI (180 stopni)?",
    "PlaneNode ma normalna skierowana w gore (+Y). Sufit musi swiecic do wnetrza pokoju, czyli w "
    "dol. Obrot o 180 stopni wokol osi X odwraca normalna, zeby oswietlenie i sciany dzialaly "
    "poprawnie od spodu."))
story.append(QA("Co to jest setUVScale i po co?",
    "Skaluje wspolrzedne tekstury. Domyslnie 1 powtorzenie tekstury na 1 metr - na scianie 30m "
    "daloby 30 malych powtorzen (brzydko). Ustawiam mniejsza wartosc (np. 0.2), zeby kafelki byly "
    "wieksze. To metoda z PlaneNode (Rogert), ja ja tylko stosuje."))
story.append(QA("Czemu dekoracje nie maja kolizji?",
    "Sa umieszczone wysoko (y >= 3m), a gracz ma oczy na 1.75m i hitbox tylko w poziomie. Wiec "
    "fizycznie nie da sie w nie wejsc - gracz po prostu pod nimi przechodzi. Nie trzeba dla nich "
    "liczyc kolizji, co oszczedza kod."))

build("Lukasz_Osoba1.pdf", "Lukasz (Osoba 1) - Math3D, SceneNode, Room", story)
