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
story.append(P("Przy kazdym pytaniu jest pasek <b>Gdzie</b> - mowi w ktorym pliku i w ktorej "
               "funkcji szukac odpowiedzi, zebys w trakcie obrony szybko otworzyl wlasciwe miejsce."))

story.append(H2("Math3D - wektory (Vec3)"))
story.append(QA("Co to jest iloczyn skalarny (dot) i co nam mowi?",
    "dot(a,b)=ax*bx+ay*by+az*bz. Jest rowny |a|*|b|*cos(kat). Gdy wektory sa znormalizowane, dot "
    "to wprost cosinus kata miedzy nimi - mowi czy wskazuja w te sama strone (dodatni), prostopadle "
    "(0) czy przeciwne (ujemny). Uzywam go tez do liczenia dlugosci: length=sqrt(dot(v,v)).",
    "Math3D.cpp -> dot(), length()"))
story.append(QA("Co robi cross (iloczyn wektorowy) i gdzie go uzywasz?",
    "cross daje wektor prostopadly do dwoch podanych. Uzywam go w lookAt: majac forward i "
    "przyblizona gore up, licze prawo kamery side=cross(forward,up), a potem dokladna gore "
    "trueUp=cross(side,forward). Tak buduje 3 prostopadle osie kamery.",
    "Math3D.cpp -> cross() oraz Mat4::lookAt()"))
story.append(QA("Dlaczego w normalize sprawdzasz dlugosc przed dzieleniem?",
    "Zeby nie dzielic przez zero. Wektor zerowy ma dlugosc 0 - dzielenie daloby NaN/nieskonczonosc. "
    "Sprawdzam if (len <= 0.000001f) i zwracam wtedy wektor zerowy jako bezpieczna wartosc.",
    "Math3D.cpp -> normalize()"))
story.append(QA("Czemu length uzywa sqrt(dot(v,v)) zamiast osobnego wzoru?",
    "dot(v,v) = x*x+y*y+z*z, czyli dokladnie to, co jest pod pierwiastkiem we wzorze na dlugosc. "
    "Wiec sqrt(dot(v,v)) = sqrt(x^2+y^2+z^2). Uzycie dot to po prostu mniej powtorzonego kodu.",
    "Math3D.cpp -> length()"))

story.append(H2("Math3D - macierze (Mat4)"))
story.append(QA("Dlaczego trzymasz macierz jako tablice 16 floatow, a nie 4x4?",
    "Bo OpenGL (glLoadMatrixf / glMultMatrixf) przyjmuje plaska tablice 16 floatow. Trzymanie ich "
    "od razu w takiej formie pozwala podac wskaznik data() wprost do OpenGL bez zadnej konwersji.",
    "Math3D.h -> struct Mat4 (pole float m[16]); Math3D.cpp -> data()"))
story.append(QA("Co dokladnie robi funkcja at(row, col)? Wytlumacz wzor col*4+row.",
    "at zwraca element macierzy w danym wierszu i kolumnie. Wzor m[col*4+row] oznacza, ze "
    "przechodzac przez pamiec po kolei dostajemy: cala kolumna 0 (4 liczby), potem kolumna 1 itd. "
    "To wlasnie definicja column-major. Gdyby bylo row*4+col, byloby row-major.",
    "Math3D.cpp -> Mat4::at() (linia m[col*4+row])"))
story.append(QA("Co to znaczy column-major i dlaczego to wazne?",
    "Column-major znaczy, ze kolejne 4 liczby w pamieci to jedna kolumna macierzy. OpenGL tego "
    "wymaga. Gdyby trzymac dane wierszami (row-major), macierz bylaby transponowana i wszystkie "
    "transformacje bylyby zle - obraz kompletnie przekrecony.",
    "Math3D.cpp -> Mat4::at()"))
story.append(QA("Jak zbudowana jest macierz translacji? Czemu offset jest w 4. kolumnie?",
    "Translacja to jednostkowa macierz z wpisanym przesunieciem w kolumnie 3 (at(0,3)=x, at(1,3)=y, "
    "at(2,3)=z). Przy mnozeniu macierz*punkt te wartosci dodaja sie do wspolrzednych, bo punkt ma "
    "czwarta wspolrzedna w=1. Dlatego translacja siedzi wlasnie w ostatniej kolumnie.",
    "Math3D.cpp -> Mat4::translation()"))
story.append(QA("Jak dziala macierz rotacji wokol Y? Skad cos i sin w konkretnych miejscach?",
    "rotationY wstawia cos i sin tak, by obrocic wspolrzedne X i Z (Y zostaje). at(0,0)=c, "
    "at(0,2)=s, at(2,0)=-s, at(2,2)=c. To standardowa macierz obrotu - punkt (x,z) przechodzi na "
    "(x*cos+z*sin, -x*sin+z*cos). Znaki decyduja o kierunku obrotu.",
    "Math3D.cpp -> Mat4::rotationY() (analogicznie rotationX/Z)"))
story.append(QA("Jak dziala mnozenie macierzy w waszym kodzie?",
    "Potrojna petla: dla kazdego wiersza i kolumny wyniku sumuje iloczyny left.at(row,i)*right.at"
    "(i,col) po i od 0 do 3. To klasyczna definicja mnozenia macierzy - wiersz lewej przez kolumne "
    "prawej.",
    "Math3D.cpp -> operator*(Mat4, Mat4)"))
story.append(QA("Czym rozni sie transformPoint od transformVector?",
    "transformPoint traktuje argument jak punkt (w=1) - uwzglednia translacje (4. kolumne) i dzieli "
    "przez w na koncu (dla perspektywy). transformVector traktuje go jak kierunek (w=0) - pomija "
    "translacje, bo kierunku sie nie przesuwa, tylko obraca/skaluje.",
    "Math3D.cpp -> transformPoint(), transformVector()"))
story.append(QA("Po co w transformPoint dzielenie przez w na koncu?",
    "Przy rzutowaniu perspektywicznym czwarta wspolrzedna w przestaje byc 1. Dzielenie x/w, y/w, "
    "z/w to 'dzielenie perspektywiczne' - to ono sprawia, ze obiekty dalej wygladaja mniejsze. "
    "Sprawdzam fabs(w)>epsilon, zeby nie dzielic przez zero.",
    "Math3D.cpp -> transformPoint()"))

story.append(H2("Math3D - projekcje i lookAt"))
story.append(QA("Co robi macierz perspective i co oznacza f = 1/tan(fov/2)?",
    "perspective buduje macierz rzutu perspektywicznego. f=1/tan(fov/2) to 'ogniskowa' - im wezsze "
    "pole widzenia (fov), tym wiekszy f, tym bardziej przyblizone. Dziele f przez aspect w at(0,0), "
    "zeby obraz nie byl rozciagniety na szerokim oknie. at(3,2)=-1 przenosi -z do w (dzielenie "
    "perspektywiczne).",
    "Math3D.cpp -> Mat4::perspective()"))
story.append(QA("Po co osobna macierz orthographic skoro jest perspective?",
    "Ortogonalna nie ma perspektywy - obiekty maja ten sam rozmiar niezaleznie od odleglosci. "
    "Uzywamy jej do rysowania HUD (2D na ekranie) i do trybu rzutu rownoleglego. To rozne "
    "zastosowania niz scena 3D.",
    "Math3D.cpp -> Mat4::orthographic(); uzycie w Engine drawHUD()"))
story.append(QA("Wyjasnij dokladnie, jak lookAt buduje macierz widoku.",
    "Najpierw licze 3 osie kamery: forward=normalize(center-eye), side=normalize(cross(forward,up)), "
    "trueUp=cross(side,forward). Te osie wpisuje jako WIERSZE macierzy (side w wiersz 0, trueUp w "
    "1, -forward w 2). 4. kolumna to -dot(os,eye) - przesuniecie, ktore przenosi eye do poczatku "
    "ukladu. Efekt: macierz obraca swiat tak, jakby kamera patrzyla wzdluz -Z.",
    "Math3D.cpp -> Mat4::lookAt()"))
story.append(QA("Dlaczego osie kamery sa w wierszach, a nie w kolumnach? (slynny bug)",
    "Bo macierz widoku to ODWROTNOSC ulozenia kamery w swiecie. Dla obrotu odwrotnosc = transpozycja, "
    "czyli osie ida w wierszach. Mielismy bug, gdzie wpisalismy je w kolumny - kamera 'krazyla "
    "wokol dziwnego punktu', a strzaly mijaly cele. Przy yaw=pitch=0 bug byl niewidoczny, bo "
    "zamieniane pola byly zerami.",
    "Math3D.cpp -> Mat4::lookAt() (komentarz o transpozycji w kodzie)"))
story.append(QA("Czemu w lookAt jest -forward, a nie forward?",
    "Bo w OpenGL kamera domyslnie patrzy wzdluz osi -Z. Trzecia os kamery (w glab ekranu) to wiec "
    "-forward. Dlatego w wierszu 2 macierzy wpisuje -forward.x, -forward.y, -forward.z.",
    "Math3D.cpp -> Mat4::lookAt()"))

story.append(H2("Math3D - raycasty (przeciecia promienia)"))
story.append(QA("Wyjasnij raySphereIntersect linijka po linijce.",
    "Podstawiam promien P(t)=origin+t*dir do rownania sfery |P-center|^2=r^2. Po rozwinieciu wychodzi "
    "a*t^2+2b*t+c=0, gdzie oc=origin-center, a=dot(dir,dir), b=dot(oc,dir), c=dot(oc,oc)-r^2. Licze "
    "wyroznik disc=b^2-a*c. Jezeli &lt;0 - promien mija sfere. Inaczej licze t1,t2 i wybieram "
    "najmniejszy dodatni (najblizszy punkt przed kamera).",
    "Math3D.cpp -> raySphereIntersect()"))
story.append(QA("Dlaczego sprawdzasz t > 0.0001f, a nie t > 0?",
    "Maly epsilon chroni przed bledami zaokraglen float i przed 'trafieniem' w punkt startowy "
    "promienia (t=0). Bez tego moglyby pojawiac sie falszywe przeciecia tuz przy kamerze.",
    "Math3D.cpp -> wszystkie ray...Intersect (warunek t > 0.0001f)"))
story.append(QA("Czemu w raySphereIntersect najpierw probujesz t1, potem t2?",
    "t1=(-b-sqrtD)/a jest zawsze mniejsze (blizsze), bo odejmujemy pierwiastek. Chcemy najblizsze "
    "trafienie przed kamera, wiec najpierw sprawdzam t1. Jezeli t1 jest ujemne (sfera za nami albo "
    "jestesmy w srodku), probuje t2 - dalszy punkt przeciecia.",
    "Math3D.cpp -> raySphereIntersect()"))
story.append(QA("Jak rayCylinderIntersect rozni sie od sfery?",
    "Walec jest pionowy, wiec rzutuje problem na plaszczyzne XZ (ignoruje Y w rownaniu kola): "
    "(px-bx)^2+(pz-bz)^2=r^2. To tez rownanie kwadratowe, ale po znalezieniu t musze jeszcze "
    "sprawdzic, czy punkt trafienia ma Y w zakresie wysokosci walca (base.y do base.y+height) - "
    "inaczej promien przechodzi obok nad/pod walcem.",
    "Math3D.cpp -> rayCylinderIntersect()"))
story.append(QA("Jak dziala rayAABBIntersect (metoda slab)?",
    "Dla kazdej z 3 osi licze przedzial parametru t, w ktorym promien jest miedzy dwiema "
    "rownoleglymi scianami (slab). Czesc wspolna wszystkich 3 przedzialow to przeciecie z pudelkiem. "
    "Jezeli tmin>tmax po ktoryms kroku - promien mija pudelko. Obsluguje tez przypadek promienia "
    "rownoleglego do osi (dir blisko 0).",
    "Math3D.cpp -> rayAABBIntersect()"))

story.append(H2("SceneNode - graf sceny"))
story.append(QA("Po co graf sceny? Nie wystarczy lista obiektow?",
    "Graf pozwala na hierarchie - dziecko dziedziczy transformacje rodzica. Np. swiatlo doczepione "
    "do obiektu rusza sie razem z nim. Mnozenie rodzic*dziecko robi to automatycznie. Z plaska "
    "lista trzeba by recznie pamietac zaleznosci.",
    "SceneNode.h/.cpp -> cala klasa; childNodes, parentNode"))
story.append(QA("Czemu localMatrix mnozy w kolejnosci T * R * S?",
    "Czytajac od prawej (tak dzialaja macierze): najpierw skala, potem obrot (Y*X*Z), na koncu "
    "translacja. Dzieki temu obiekt skaluje i obraca sie wokol wlasnego srodka, a dopiero potem "
    "jest przesuwany. Odwrotna kolejnosc daloby obrot wokol srodka swiata.",
    "SceneNode.cpp -> localMatrix()"))
story.append(QA("Dlaczego obroty sa w kolejnosci Y * X * Z, a nie np. X * Y * Z?",
    "To kwestia konwencji katow Eulera. My najpierw obracamy w poziomie (Y - yaw), potem w pionie "
    "(X - pitch), potem przechylenie (Z - roll). Kolejnosc wplywa na efekt przy laczeniu obrotow, "
    "ale dla naszych obiektow (zwykle obrot tylko wokol jednej osi) jest to wystarczajace i "
    "intuicyjne.",
    "SceneNode.cpp -> localMatrix()"))
story.append(QA("Dlaczego renderRecursive mnozy parentMatrix * localMatrix, a nie odwrotnie?",
    "Bo macierz swiata dziecka = macierz swiata rodzica zlozona z lokalna transformacja dziecka. "
    "Kolejnosc rodzic*dziecko sprawia, ze lokalna transformacja dziala 'wewnatrz' ukladu rodzica.",
    "SceneNode.cpp -> renderRecursive()"))
story.append(QA("Czym jest renderSelf i dlaczego jest wirtualne?",
    "To metoda rysujaca konkretna geometrie. W SceneNode jest pusta (wezel grupujacy). Klasy "
    "potomne (CubeNode, PointLight...) nadpisuja ja. Wirtualnosc pozwala wywolac wlasciwa wersje "
    "przez wskaznik do klasy bazowej - to polimorfizm.",
    "SceneNode.cpp -> renderSelf() (pusta); nadpisana w PrimitiveNode/Light"))
story.append(QA("Co daje flaga nodeVisible i gdzie jest sprawdzana?",
    "Gdy false, renderSelf jest pomijane (obiekt znika), ale dzieci nadal sie rysuja. Sprawdzam "
    "to w renderRecursive: if (nodeVisible) renderSelf(world). Kacper uzywa tego do puli celow - "
    "zamiast tworzyc i niszczyc sfery, chowamy je.",
    "SceneNode.cpp -> renderRecursive() (if nodeVisible); SceneNode.h -> setVisible()"))
story.append(QA("Co sie dzieje w addChild? Czemu dziecko zapamietuje rodzica?",
    "addChild ustawia child->parentNode = this i dodaje dziecko do listy childNodes. Wskaznik na "
    "rodzica pozwala dziecku policzyc worldMatrix idac w gore drzewa. Sprawdzam tez czy child nie "
    "jest nullem.",
    "SceneNode.cpp -> addChild()"))
story.append(QA("Jaka jest roznica miedzy localMatrix a worldMatrix?",
    "localMatrix to transformacja obiektu wzgledem rodzica. worldMatrix to transformacja wzgledem "
    "calego swiata - liczona rekurencyjnie jako parent->worldMatrix() * localMatrix(). Dla obiektu "
    "bez rodzica obie sa rowne.",
    "SceneNode.cpp -> localMatrix(), worldMatrix()"))

story.append(H2("Room.cpp - budowa pokoju"))
story.append(QA("Dlaczego sufit jest obrocony o PI (180 stopni) wokol X?",
    "PlaneNode ma normalna w gore (+Y). Sufit musi swiecic do wnetrza pokoju, czyli w dol. Obrot o "
    "180 stopni wokol osi X odwraca normalna na -Y, zeby oswietlenie liczylo sie poprawnie od "
    "spodu.",
    "Room.cpp -> buildRoom() (ceiling_->setRotation(Vec3(PI,0,0)))"))
story.append(QA("Jak ustawiona jest sciana boczna? Czemu obrot wokol Z?",
    "Plaszczyzna jest domyslnie pozioma (XZ). Zeby postawic ja pionowo jako sciane lewa/prawa, "
    "obracam o 90 stopni wokol osi Z (setRotation(0,0,+-PI/2)). Tylna/przednia sciana to obrot "
    "wokol X. Pozycje ustawiam na krawedzi pokoju (+-ROOM_X_HALF).",
    "Room.cpp -> buildRoom() (leftWall_/rightWall_ setRotation wokol Z)"))
story.append(QA("Co to jest setUVScale i po co go ustawiasz na scianach?",
    "Skaluje wspolrzedne tekstury. Domyslnie 1 powtorzenie na metr - na scianie 30m daloby 30 "
    "drobnych kafelkow. Ustawiam mniejsza wartosc (0.2-0.3), zeby kafelki byly wieksze i ladniejsze. "
    "To metoda PlaneNode (Rogert), ja ja tylko stosuje.",
    "Room.cpp -> buildRoom() (setUVScale); PlaneNode.cpp (implementacja)"))
story.append(QA("Po co lambdy makeSphere / makeCube / makeCylinder w dekoracjach?",
    "Zeby nie powtarzac tego samego kodu (stworz wezel, ustaw pozycje/material/teksture, dodaj do "
    "sceny) kilkanascie razy. Lambda to mala lokalna funkcja - wywoluje ja z roznymi pozycjami. "
    "Czystszy i krotszy kod.",
    "Room.cpp -> buildDecorations() (lambdy makeSphere itd.)"))
story.append(QA("Czemu dekoracje nie maja kolizji?",
    "Sa wysoko (y >= 3m), a gracz ma oczy na 1.75m i hitbox tylko w poziomie. Fizycznie nie da sie "
    "w nie wejsc - gracz przechodzi pod nimi. Nie trzeba dla nich liczyc kolizji, co oszczedza kod "
    "i czas.",
    "Room.cpp -> buildDecorations(); por. Obstacles.cpp (tam sa kolizje)"))
story.append(QA("Skad biora sie tekstury w pokoju - z pliku czy generowane?",
    "Probuje najpierw wczytac plik BMP (loadBMP). Jezeli pliku nie ma albo ma zly format, funkcja "
    "zwraca false i wtedy generuje teksture proceduralnie (np. generateBricks dla podlogi, "
    "generatePerlinNoise dla sufitu). Te funkcje napisal Kacper, ja ich uzywam.",
    "Room.cpp -> buildRoom() (if(!loadBMP) generate...); Texture.cpp (Kacper)"))

build("Lukasz_Osoba1.pdf", "Lukasz (Osoba 1) - Math3D, SceneNode, Room", story)
