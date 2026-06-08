# -*- coding: utf-8 -*-
from _pdfkit import *

story = []
story += title_block(
    "Rogert",
    "Osoba 2",
    "Prymitywy 3D, przeszkody i kolizje",
    "Engine/PrimitiveNode.h, Engine/PrimitiveNode.cpp, Demo/Obstacles.cpp"
)

# ============ MAPA ============
story.append(H1("0. Gdzie sa moje pliki (mapa do prezentacji)"))
story.append(P("Strzalka <b>&lt;&lt;&lt;</b> pokazuje pliki, ktore robilem ja (Rogert)."))
story += TREE(
"""pgk/
|
+-- Engine/
|   +-- Math3D.* SceneNode.*           (Lukasz)
|   +-- PrimitiveNode.h   <<< MOJE  (wszystkie ksztalty 3D)
|   +-- PrimitiveNode.cpp <<< MOJE
|   +-- Camera.* Light.* Texture.*     (Kacper)
|   +-- Engine.*                       (Jakub)
|
+-- Demo/
|   +-- Room.cpp                       (Lukasz)
|   +-- Obstacles.cpp     <<< MOJE  (kolumny, stozki, skrzynie + kolizje)
|   +-- Targets.cpp                    (Kacper)
|   +-- Gameplay.cpp Constants.h ShootingGallery.h  (Jakub)
|
+-- main.cpp CMakeLists.txt            (Jakub)
""")
story.append(P("<b>W skrocie:</b> ja robie <b>ksztalty</b> (szescian, walec, sfera, stozek, torus, "
               "plaszczyzna) - kazdy rysowany wlasnym kodem OpenGL - oraz <b>przeszkody</b> w grze "
               "wraz z <b>kolizjami</b> (gracz nie przechodzi przez sciany, strzal nie przelatuje "
               "przez kolumne)."))

# ============ 1. WPROWADZENIE ============
story.append(PageBreak())
story.append(H1("1. O co chodzi w moich plikach (po ludzku)"))
story.append(P("Komputer nie wie, czym jest 'szescian'. Trzeba mu podac kazdy <b>wierzcholek</b> "
               "(rog), <b>normalna</b> (kierunek, w ktory patrzy powierzchnia - potrzebny do "
               "swiatla) i <b>wspolrzedne UV</b> (gdzie na teksturze jest dany punkt). To wlasnie "
               "robie w <b>PrimitiveNode</b> - recznie, przez glBegin/glEnd. Nie uzywam gotowcow "
               "typu glutSolidCube, bo wymaganiem projektu bylo napisanie geometrii samemu."))
story.append(P("W <b>Obstacles.cpp</b> uzywam tych ksztaltow, zeby postawic w pokoju kolumny, "
               "stozki i skrzynie. Dopisuje tez <b>matematyke kolizji</b>: zeby gracz nie wchodzil "
               "w przeszkode i zeby przeszkoda zaslaniala strzal."))

# ============ 2. PRIMITIVENODE - architektura ============
story.append(H1("2. PrimitiveNode - wspolna baza wszystkich ksztaltow"))
story.append(P("<b>PrimitiveNode</b> dziedziczy po SceneNode (Lukasz) i dodaje <b>material</b> "
               "(kolory dla swiatla) oraz <b>teksture</b>. Ma jedna wspolna metode renderSelf, "
               "ktora ustawia material/teksture, a potem wola <b>drawGeometry()</b> - i to wlasnie "
               "ta funkcja jest rozna dla kazdego ksztaltu."))
story += CODE(
"""void PrimitiveNode::renderSelf(const Mat4& worldMatrix) const {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glColor3f(diffuse.x, diffuse.y, diffuse.z);   // fallback gdy swiatlo off

    bool hasTexture = (surfaceTexture && surfaceTexture->isLoaded());
    if (hasTexture) { glEnable(GL_TEXTURE_2D); surfaceTexture->bind(); }

    glPushMatrix();
    glMultMatrixf(worldMatrix.data());   // ustaw transformacje obiektu
    drawGeometry();                      // <- nadpisane w kazdej podklasie
    glPopMatrix();

    if (hasTexture) { surfaceTexture->unbind(); glDisable(GL_TEXTURE_2D); }
}""", "PrimitiveNode.cpp - wspolny render, geometria w drawGeometry()")
story.append(P("<b>drawGeometry()</b> jest <b>czysto wirtualna</b> (= 0) - to znaczy, ze "
               "PrimitiveNode sam jej nie ma, a kazda podklasa MUSI ja napisac. To wzorzec "
               "'szablon metody': baza robi powtarzalne rzeczy (material, transformacja), a detal "
               "(jaki ksztalt) zostawia potomkom."))

# ============ 3. Geometria - przyklady ============
story.append(H1("3. Jak rysuje konkretne ksztalty"))

story.append(H2("3.1. Sfera - siatka rownoleznikow i poludnikow"))
story.append(P("Sfere dziele na poziome pasy (stacks) i pionowe wycinki (slices), jak globus. "
               "Kazdy punkt liczy ze wspolrzednych sferycznych. Wazne: dla sfery jednostkowej "
               "<b>normalna = pozycja punktu</b> (kierunek od srodka)."))
story += CODE(
"""float nx = sin(theta)*cos(phi);
float ny = cos(theta);
float nz = sin(theta)*sin(phi);
glNormal3f(nx, ny, nz);                 // normalna = kierunek na zewnatrz
glTexCoord2f(u, v);                     // u = phi/2pi, v = theta/pi
glVertex3f(nx*radius, ny*radius, nz*radius);""",
    "SphereNode::drawGeometry() - jeden wierzcholek")

story.append(H2("3.2. Stozek - dlaczego osobne trojkaty, a nie wachlarz"))
story.append(P("Stozek mozna narysowac jako TRIANGLE_FAN z jednym wierzcholkiem na czubku. Ale "
               "wtedy czubek ma <b>jedna</b> normalna, przez co cieniowanie wyglada plasko/brzydko. "
               "Dlatego rysuje N osobnych trojkatow - kazdy ma na czubku <b>usredniona</b> normalna "
               "z dwoch sasiednich scianek. Wyglada duzo lepiej."))
story += CODE(
"""float L  = sqrt(r*r + h*h);
float nh = h / L;     // pozioma skladowa normalnej
float nv = r / L;     // pionowa skladowa (stozek zwęża sie ku gorze)
// normalna sciany bocznej w kierunku kata 'ang':
glNormal3f(cos(ang)*nh, nv, sin(ang)*nh);""",
    "ConeNode::drawGeometry() - nachylenie normalnej zalezy od proporcji r:h")

story.append(H2("3.3. Plaszczyzna i skala UV"))
story.append(P("PlaneNode to zwykly prostokat w plaszczyznie XZ z normalna w gore. Dodalem "
               "<b>setUVScale</b>, bo na duzej scianie tekstura powtarzala sie zbyt gesto."))
story += CODE(
"""// uMax/vMax decyduja ile razy tekstura sie powtorzy
const float uMax = planeWidth  * planeUVScale;
const float vMax = planeDepth  * planeUVScale;
glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, 0, hd);
glTexCoord2f(uMax, vMax); glVertex3f( hw, 0,-hd);   // itd.""",
    "PlaneNode::drawGeometry() - UV mnozone przez skale")

# ============ 4. OBSTACLES ============
story.append(PageBreak())
story.append(H1("4. Demo/Obstacles.cpp - przeszkody i kolizje"))
story.append(P("Tu stawiam w pokoju: 6 kolumn (walce), 3 stozki i 2 skrzynie (szesciany). Kazda "
               "przeszkode zapisuje w dwoch miejscach: jako <b>wezel sceny</b> (zeby ja narysowac) "
               "i jako <b>dane kolizji</b> (zeby liczyc fizyke)."))

story.append(H2("4.1. Trzy rodzaje kolizji gracza"))
story.append(P("Gracz ma 'hitbox' - kolko o promieniu PLAYER_RADIUS. Po ruchu sprawdzam, czy "
               "wszedl w przeszkode, i jak tak - <b>wypycham</b> go na zewnatrz."))
story += CODE(
"""// kolizja z walcem (kolizja kolowa w plaszczyznie XZ)
float dx = playerEye_.x - o.base.x;
float dz = playerEye_.z - o.base.z;
float minR = o.radius + PLAYER_RADIUS;
float distSq = dx*dx + dz*dz;
if (distSq < minR*minR && distSq > 0.0001f) {
    float dist = sqrt(distSq);
    float push = (minR - dist) / dist;   // ile brakuje do wyjscia
    playerEye_.x += dx * push;           // wypchnij wzdluz osi srodek->gracz
    playerEye_.z += dz * push;
}""", "Obstacles.cpp - wypychanie gracza z kolumny")
story.append(P("Dla <b>skrzyni</b> (prostokat) szukam najblizszego punktu na jej brzegu i "
               "wypycham gracza o promien. Dla <b>stozka</b> traktuje go jak walec o promieniu "
               "podstawy (gracz i tak nie wejdzie pod nawis)."))

story.append(H2("4.2. Przeszkoda zaslania strzal"))
story.append(P("Gdy gracz strzela, sprawdzam czy <b>najpierw</b> nie trafi w przeszkode. Uzywam "
               "funkcji raycast od Lukasza."))
story += CODE(
"""bool ShootingGallery::obstacleBlocksRay(const Vec3& origin,
                                       const Vec3& dir, float maxT) const {
    float t;
    for (const auto& o : cylinderObs_)
        if (rayCylinderIntersect(origin,dir,o.base,o.radius,o.height,t)
            && t > 0.0001f && t < maxT) return true;
    for (const auto& o : coneObs_)
        if (rayConeIntersect(origin,dir,o.base,o.radius,o.height,t)
            && t > 0.0001f && t < maxT) return true;
    for (const auto& o : boxObs_)
        if (rayAABBIntersect(origin,dir,o.boxMin,o.boxMax,t)
            && t > 0.0001f && t < maxT) return true;
    return false;
}""", "Obstacles.cpp - czy przeszkoda jest blizej niz cel")
story.append(P("<b>maxT</b> to odleglosc do trafionego celu. Jezeli ktoras przeszkoda jest blizej "
               "(t &lt; maxT), strzal jest zablokowany i liczy sie jako pudlo - cel byl schowany za "
               "kolumna."))

# ============ 5. Q&A ============
story.append(PageBreak())
story.append(H1("5. Pytania od wykladowcy - przygotowanie"))
story.append(P("Przy kazdym pytaniu jest pasek <b>Gdzie</b> - mowi w ktorym pliku i w ktorej "
               "funkcji szukac odpowiedzi, zebys w trakcie obrony szybko otworzyl wlasciwe miejsce."))

story.append(H2("PrimitiveNode - architektura"))
story.append(QA("Dlaczego nie uzywacie gotowych funkcji jak glutSolidSphere?",
    "Bo wymaganiem projektu bylo, zeby grupa sama napisala geometrie prymitywow. FreeGLUT ma "
    "tylko pomagac z oknem i wejsciem. Dlatego kazdy ksztalt rysuje recznie przez glBegin/glEnd, "
    "samodzielnie liczac wierzcholki, normalne i UV.",
    "PrimitiveNode.cpp -> drawGeometry() w kazdej klasie"))
story.append(QA("Dlaczego drawGeometry jest czysto wirtualna (= 0)?",
    "Bo PrimitiveNode to klasa abstrakcyjna - nie istnieje 'ogolny prymityw', istnieje konkretny "
    "szescian czy sfera. =0 wymusza, ze kazda podklasa musi napisac wlasne rysowanie. Wspolne "
    "rzeczy (material, transformacja, tekstura) sa w renderSelf w bazie - to wzorzec szablonu "
    "metody (template method).",
    "PrimitiveNode.h -> virtual void drawGeometry() const = 0;"))
story.append(QA("Co dokladnie robi renderSelf w PrimitiveNode?",
    "Ustawia material przez glMaterialfv (ambient/diffuse/specular/shininess), ustawia kolor "
    "fallback przez glColor3f (na wypadek wylaczonego swiatla), wlacza teksture jezeli jest, robi "
    "glPushMatrix + glMultMatrixf(world) + drawGeometry() + glPopMatrix, na koncu wylacza teksture. "
    "To powtarzalna otoczka wokol rysowania konkretnego ksztaltu.",
    "PrimitiveNode.cpp -> PrimitiveNode::renderSelf()"))
story.append(QA("Po co glPushMatrix / glPopMatrix wokol rysowania?",
    "glPushMatrix zapisuje aktualna macierz na stosie, glMultMatrixf naklada transformacje obiektu, "
    "rysujemy, a glPopMatrix przywraca poprzedni stan. Dzieki temu transformacja jednego obiektu "
    "nie wplywa na nastepny rysowany.",
    "PrimitiveNode.cpp -> PrimitiveNode::renderSelf()"))
story.append(QA("Czemu ustawiasz i glMaterial, i glColor3f naraz?",
    "glMaterial dziala gdy oswietlenie jest WLACZONE. glColor3f jest fallbackiem - gdy gracz "
    "wylaczy swiatlo (klawisz L), liczy sie kolor z glColor. Ustawiam oba, zeby obiekt byl "
    "widoczny w obu trybach.",
    "PrimitiveNode.cpp -> PrimitiveNode::renderSelf() (glColor3f)"))
story.append(QA("Co to jest GL_MODULATE przy teksturze?",
    "Tryb laczenia tekstury z kolorem/oswietleniem. MODULATE mnozy kolor tekstury przez kolor "
    "oswietlenia obiektu - dzieki temu oteksturowany obiekt nadal reaguje na swiatlo (ciemnieje w "
    "cieniu). Gdyby bylo GL_REPLACE, tekstura zastapilaby oswietlenie calkowicie.",
    "PrimitiveNode.cpp -> renderSelf() (glTexEnvf ... GL_MODULATE)"))

story.append(H2("Pojecia: normalne i UV"))
story.append(QA("Co to jest normalna i po co ja podajesz?",
    "Normalna to wektor prostopadly do powierzchni - mowi, w ktora strone 'patrzy' scianka. OpenGL "
    "uzywa jej do liczenia oswietlenia: im bardziej normalna skierowana do swiatla, tym jasniejszy "
    "punkt. Bez normalnych model bylby plaski, bez cieniowania.",
    "PrimitiveNode.cpp -> glNormal3f w kazdym drawGeometry()"))
story.append(QA("Co to sa wspolrzedne UV?",
    "To wspolrzedne na teksturze (od 0 do 1). Mowia, ktory fragment obrazka nalepic na dany "
    "wierzcholek. glTexCoord2f(u,v) przed glVertex3f przypisuje punktowi miejsce na teksturze. "
    "u to pozioma os tekstury, v pionowa.",
    "PrimitiveNode.cpp -> glTexCoord2f w kazdym drawGeometry()"))
story.append(QA("Czemu wartosci normalnej musza byc znormalizowane?",
    "Bo OpenGL liczy oswietlenie z cosinusa kata, ktory zaklada wektory dlugosci 1. Niezormalizowana "
    "normalna zafalszowalaby jasnosc. W silniku mamy tez wlaczone glEnable(GL_NORMALIZE), ktore "
    "normalizuje normalne automatycznie po skalowaniu - to wazne, bo skala psuje dlugosc "
    "normalnych.",
    "Engine.cpp -> glEnable(GL_NORMALIZE); normalne liczone w PrimitiveNode.cpp"))

story.append(H2("Geometria - szescian i walec"))
story.append(QA("Jak rysujesz szescian? Ile scian, jakie prymitywy?",
    "6 scian jako GL_QUADS (czworokaty). Kazda sciana ma jedna wspolna normalna (np. przod ma "
    "(0,0,1)) i 4 wierzcholki z UV od (0,0) do (1,1). Wszystkie wspolrzedne licze z polowy boku "
    "(halfSize), wiec szescian jest wycentrowany w srodku.",
    "PrimitiveNode.cpp -> CubeNode::drawGeometry()"))
story.append(QA("Jak rysujesz boczna powierzchnie walca?",
    "Jako GL_TRIANGLE_STRIP po obwodzie - dla kazdego kata stawiam dwa wierzcholki (gorny i dolny). "
    "Normalna jest pozioma, skierowana od osi na zewnatrz (cos(kat),0,sin(kat)). Gora i dol walca "
    "to osobne TRIANGLE_FAN (dyski). slices to liczba podzialow - wiecej = gladszy walec.",
    "PrimitiveNode.cpp -> CylinderNode::drawGeometry()"))
story.append(QA("Czemu normalna na boku walca nie zalezy od promienia?",
    "Bo normalna to tylko KIERUNEK (na zewnatrz od osi), a nie pozycja. Kierunek jest taki sam "
    "niezaleznie od tego, jak gruby jest walec - dlatego uzywam czystego (cos,0,sin) bez mnozenia "
    "przez promien. Mnoze przez promien tylko pozycje wierzcholka.",
    "PrimitiveNode.cpp -> CylinderNode::drawGeometry()"))

story.append(H2("Geometria - sfera, stozek, torus"))
story.append(QA("Jak parametryzujesz sfere?",
    "Dwoma katami: theta (od bieguna, 0..pi) dzieli na poziome pasy (stacks), phi (dlugosc, 0..2pi) "
    "na pionowe wycinki (slices). Pozycja punktu to (sin(theta)*cos(phi), cos(theta), "
    "sin(theta)*sin(phi)) razy promien. Rysuje pasami jako TRIANGLE_STRIP.",
    "PrimitiveNode.cpp -> SphereNode::drawGeometry()"))
story.append(QA("Dlaczego dla sfery normalna = pozycja wierzcholka (bez promienia)?",
    "Bo w sferze kierunek od srodka do punktu jest jednoczesnie kierunkiem 'na zewnatrz', czyli "
    "normalna. Ten sam wektor uzywam raz jako normalna (czysty, dlugosc 1) i raz jako pozycje "
    "(pomnozony przez promien).",
    "PrimitiveNode.cpp -> SphereNode::drawGeometry()"))
story.append(QA("Czemu stozek rysujesz osobnymi trojkatami, a nie TRIANGLE_FAN?",
    "W wachlarzu czubek to jeden wspolny wierzcholek z jedna normalna - cieniowanie wokol czubka "
    "wyglada plasko/brzydko. Rysujac N osobnych trojkatow, czubek kazdego dostaje normalna "
    "usredniona z dwoch sasiednich scianek, wiec swiatlo plynnie przechodzi dookola stozka.",
    "PrimitiveNode.cpp -> ConeNode::drawGeometry() (petla GL_TRIANGLES)"))
story.append(QA("Jak liczysz nachylenie normalnej na bocznej sciance stozka?",
    "Z proporcji promienia do wysokosci. L=sqrt(r^2+h^2) to dlugosc tworzacej. Skladowa pozioma "
    "normalnej to nh=h/L, pionowa nv=r/L. Pionowa jest dodatnia, bo stozek zwęża sie ku gorze, "
    "wiec powierzchnia lekko patrzy w gore. Normalna w kierunku kata: (cos*nh, nv, sin*nh).",
    "PrimitiveNode.cpp -> ConeNode::drawGeometry() (L, nh, nv)"))
story.append(QA("Jak parametryzowany jest torus?",
    "Dwoma katami u (wokol calego pierscienia, 0..2pi) i v (wokol rurki, 0..2pi). Pozycja: "
    "((R+r*cos v)*cos u, r*sin v, (R+r*cos v)*sin u), gdzie R to glowny promien (do srodka rurki), "
    "r maly (grubosc rurki). Normalna to kierunek od srodka rurki do punktu. Torus jest u nas tylko "
    "dekoracyjny (zyrandol).",
    "PrimitiveNode.cpp -> TorusNode::drawGeometry()"))

story.append(H2("Plaszczyzna i UV scaling"))
story.append(QA("Co robi setUVScale w PlaneNode i jak jest zaimplementowane?",
    "Mnozy wspolrzedne UV. uMax = planeWidth * planeUVScale, vMax = planeDepth * planeUVScale. "
    "Domyslnie skala 1 daje 1 powtorzenie na metr - na scianie 30m to 30 drobnych kafelkow. "
    "Mniejsza skala (0.2) daje wieksze kafelki. Sama geometria sie nie zmienia, tylko UV.",
    "PrimitiveNode.cpp -> PlaneNode::drawGeometry(); PlaneNode.h -> setUVScale()"))
story.append(QA("Czemu PlaneNode ma normalna w gore (+Y)?",
    "Bo to plaszczyzna pozioma (w plaszczyznie XZ), domyslnie podloga. Normalna (0,1,0) patrzy w "
    "gore. Zeby zrobic z niej sciane czy sufit, Lukasz obraca ja w Room.cpp - obrot zmienia tez "
    "kierunek normalnej.",
    "PrimitiveNode.cpp -> PlaneNode::drawGeometry() (glNormal3f(0,1,0))"))

story.append(H2("Obstacles - rozmieszczenie"))
story.append(QA("Ile i jakich przeszkod jest w pokoju?",
    "6 kolumn (CylinderNode), 3 stozki (ConeNode) i 2 skrzynie (CubeNode ze skala). Definiuje je "
    "jako listy struktur (CylDef, ConeDef, BoxDef) z pozycja i wymiarami, a potem w petli tworze "
    "wezel sceny i wpis kolizji. Rozmieszczone sa po calym pokoju (przednia i tylna polowa).",
    "Obstacles.cpp -> buildObstacles() (wektory cyls, cones, boxes)"))
story.append(QA("Czemu skrzynia to CubeNode ze skala, a nie osobna klasa?",
    "Bo skrzynia to po prostu przeskalowany szescian. Zamiast pisac nowy ksztalt, tworze CubeNode "
    "o boku 1 i ustawiam setScale(w,h,d). Macierz lokalna (Lukasz) zajmie sie skalowaniem. Mniej "
    "kodu, ten sam efekt.",
    "Obstacles.cpp -> buildObstacles() (node->setScale przy skrzyniach)"))
story.append(QA("Czemu przechowujesz przeszkode dwa razy (wezel sceny + dane kolizji)?",
    "Wezel (CylinderNode) sluzy tylko do RYSOWANIA. Do FIZYKI potrzebuje czystych danych (pozycja, "
    "promien, wysokosc) bez calego obiektu OpenGL. Rozdzielenie 'co widac' od 'co jest fizyczne' "
    "jest czytelniejsze i szybsze - kolizje nie musza znac OpenGL.",
    "Obstacles.cpp -> buildObstacles(); ShootingGallery.h -> cylinderObs_, coneObs_, boxObs_"))

story.append(H2("Obstacles - kolizje gracza"))
story.append(QA("Jak dziala wypychanie gracza z kolumny (krok po kroku)?",
    "Licze wektor od osi kolumny do gracza w plaszczyznie XZ (dx,dz) i kwadrat odleglosci distSq. "
    "Jezeli distSq < (promien kolumny + promien gracza)^2, gracz jest w srodku. Licze dist=sqrt, "
    "potem push=(minR-dist)/dist i przesuwam gracza o dx*push, dz*push - czyli wzdluz linii "
    "srodek->gracz az na brzeg.",
    "Obstacles.cpp -> applyObstacleCollision() (petla po cylinderObs_)"))
story.append(QA("Czemu w kolizjach uzywasz distSq, a nie samej odleglosci?",
    "Pierwiastek (sqrt) jest wolny. Porownanie distSq < minR*minR daje ten sam wynik co dist < "
    "minR, ale bez pierwiastka. Sqrt licze dopiero gdy faktycznie musze wypchnac gracza (do "
    "policzenia kierunku).",
    "Obstacles.cpp -> applyObstacleCollision()"))
story.append(QA("Jak dzialaja kolizje ze skrzynia (prostopadloscianem)?",
    "Znajduje punkt na powierzchni skrzyni najblizszy graczowi - przez clamp wspolrzednych gracza "
    "do zakresu [boxMin, boxMax]. Jezeli gracz jest blizej tego punktu niz jego promien, wypycham "
    "go na zewnatrz. Gdy gracz wszedl do samego srodka (distSq~0), wybieram najblizsza sciane "
    "(min z 4 odleglosci) i wypycham w jej strone.",
    "Obstacles.cpp -> applyObstacleCollision() (petla po boxObs_, clamp + 4 sciany)"))
story.append(QA("Czemu stozek w kolizjach traktujesz jak walec?",
    "Bo gracz i tak nie wejdzie pod zwężajacy sie nawis stozka (jest za niski/za szeroki u dolu). "
    "Wystarczy traktowac go jak walec o promieniu podstawy - prostsze, a efekt dla gracza "
    "identyczny. Pelny ksztalt stozka liczę dopiero przy strzale (raycast).",
    "Obstacles.cpp -> applyObstacleCollision() (petla po coneObs_)"))

story.append(H2("Obstacles - blokowanie strzalu"))
story.append(QA("Jak sprawdzasz, ze przeszkoda zaslania cel?",
    "Po znalezieniu trafionego celu mam odleglosc do niego (maxT). W obstacleBlocksRay sprawdzam, "
    "czy ktoras przeszkoda jest przecieta przez ten sam promien przy t < maxT. Jezeli tak - "
    "przeszkoda jest blizej niz cel, wiec strzal w nia trafia pierwszy i liczy sie jako pudlo.",
    "Obstacles.cpp -> obstacleBlocksRay(); wolane z Gameplay.cpp onShoot()"))
story.append(QA("Jakich funkcji uzywasz do sprawdzenia przeciecia z kazdym typem przeszkody?",
    "rayCylinderIntersect dla kolumn, rayConeIntersect dla stozkow, rayAABBIntersect dla skrzyn. "
    "Wszystkie trzy napisal Lukasz w Math3D. Ja przechodze petla po moich listach przeszkod i "
    "wolam odpowiednia funkcje, sprawdzajac czy t miesci sie w (0, maxT).",
    "Obstacles.cpp -> obstacleBlocksRay(); Math3D.cpp (implementacje raycastow)"))
story.append(QA("Co by sie stalo, gdybys nie sprawdzal blokowania strzalu?",
    "Gracz moglby strzelac przez sciany i kolumny - trafialby cele schowane za przeszkodami, czego "
    "nie widzi. Byloby to nierealistyczne i psuloby sens chowania sie celow. Dlatego raycast do "
    "celu jest 'przycinany' przez przeszkody.",
    "Obstacles.cpp -> obstacleBlocksRay()"))

build("Rogert_Osoba2.pdf", "Rogert (Osoba 2) - Prymitywy 3D i kolizje", story)
