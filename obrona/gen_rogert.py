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

story.append(H2("PrimitiveNode - ogolne"))
story.append(QA("Dlaczego nie uzywacie gotowych funkcji jak glutSolidSphere?",
    "Bo wymaganiem projektu bylo, zeby grupa sama napisala geometrie prymitywow. FreeGLUT ma "
    "tylko pomagac z oknem i wejsciem. Dlatego kazdy ksztalt rysuje recznie przez glBegin/glEnd, "
    "samodzielnie liczac wierzcholki, normalne i UV."))
story.append(QA("Co to jest normalna i po co ja podajesz?",
    "Normalna to wektor prostopadly do powierzchni - mowi, w ktora strone 'patrzy' scianka. "
    "OpenGL uzywa jej do liczenia oswietlenia: im bardziej normalna skierowana do swiatla, tym "
    "jasniejszy punkt. Bez normalnych model bylby plaski, bez cieniowania."))
story.append(QA("Co to sa wspolrzedne UV?",
    "To wspolrzedne na teksturze (od 0 do 1). Mowia, ktory fragment obrazka nalepic na dany "
    "wierzcholek. glTexCoord2f(u,v) przed glVertex3f przypisuje punktowi miejsce na teksturze."))
story.append(QA("Dlaczego drawGeometry jest czysto wirtualna (= 0)?",
    "Bo PrimitiveNode to klasa abstrakcyjna - nie istnieje 'ogolny prymityw', istnieje konkretny "
    "szescian czy sfera. =0 wymusza, ze kazda podklasa musi napisac wlasne rysowanie. Wspolne "
    "rzeczy (material, transformacja) sa w renderSelf w bazie - to wzorzec szablonu metody."))
story.append(QA("Po co glPushMatrix / glPopMatrix wokol rysowania?",
    "glPushMatrix zapisuje aktualna macierz, glMultMatrixf nakłada transformacje obiektu, rysujemy, "
    "a glPopMatrix przywraca poprzedni stan. Dzieki temu transformacja jednego obiektu nie wplywa "
    "na nastepne."))

story.append(H2("Geometria ksztaltow"))
story.append(QA("Dlaczego dla sfery normalna = pozycja wierzcholka?",
    "Bo w sferze jednostkowej (promien 1) kierunek od srodka do punktu jest jednoczesnie "
    "kierunkiem 'na zewnatrz', czyli normalna. Wystarczy ten sam wektor uzyc raz jako pozycje "
    "(po przemnozeniu przez promien) i raz jako normalna."))
story.append(QA("Czemu stozek rysujesz osobnymi trojkatami, a nie TRIANGLE_FAN?",
    "W wachlarzu czubek to jeden wspolny wierzcholek z jedna normalna - cieniowanie wokol czubka "
    "wyglada wtedy plasko. Rysujac osobne trojkaty, czubek kazdego dostaje normalna usredniona z "
    "dwoch sasiednich scianek, wiec swiatlo plynnie przechodzi dookola."))
story.append(QA("Jak liczysz nachylenie normalnej na bocznej sciance stozka?",
    "Z proporcji promienia do wysokosci. L=sqrt(r^2+h^2) to dlugosc tworzacej. Skladowa pozioma "
    "normalnej to h/L, pionowa to r/L. Pionowa jest dodatnia, bo stozek zwęża sie ku gorze, wiec "
    "powierzchnia lekko patrzy do gory."))
story.append(QA("Co robi setUVScale w PlaneNode?",
    "Mnozy wspolrzedne UV. Domyslnie 1 powtorzenie tekstury na metr - na scianie 30m wychodzi 30 "
    "drobnych kafelkow. Mniejsza skala (np. 0.2) daje wieksze, ladniejsze kafelki. To prosty "
    "trik bez zmiany samej geometrii."))

story.append(H2("Obstacles i kolizje"))
story.append(QA("Jak dziala wypychanie gracza z kolumny?",
    "Licze odleglosc gracza od osi kolumny w plaszczyznie XZ. Jezeli jest mniejsza niz suma "
    "promieni (kolumny + gracza), gracz jest 'w srodku'. Licze brakujacy dystans i przesuwam "
    "gracza wzdluz linii srodek-gracz tak, zeby znalazl sie dokladnie na brzegu."))
story.append(QA("Czemu przechowujesz przeszkode dwa razy (wezel + dane kolizji)?",
    "Wezel sceny (CylinderNode) sluzy tylko do rysowania. Do fizyki potrzebuje czystych danych "
    "(pozycja, promien, wysokosc) bez calego obiektu OpenGL. Rozdzielenie 'co widac' od 'co jest "
    "fizyczne' jest czytelniejsze i szybsze."))
story.append(QA("Jak dzialaja kolizje ze skrzynia (prostopadloscianem)?",
    "Znajduje punkt na powierzchni skrzyni najblizszy graczowi (przez clamp wspolrzednych do "
    "zakresu skrzyni). Jezeli gracz jest blizej tego punktu niz jego promien, wypycham go na "
    "zewnatrz. Gdy gracz jest w samym srodku, wybieram najblizsza sciane i wypycham w jej strone."))
story.append(QA("Jak sprawdzasz, ze przeszkoda zaslania cel?",
    "Po znalezieniu trafionego celu mam odleglosc do niego (maxT). Sprawdzam, czy ktoras przeszkoda "
    "jest przeciecata przez ten sam promien przy mniejszym t. Jezeli tak - strzal trafia w "
    "przeszkode pierwszy, wiec liczy sie jako pudlo."))
story.append(QA("Czemu w kolizjach uzywasz distSq (kwadratu odleglosci), a nie samej odleglosci?",
    "Pierwiastek (sqrt) jest wolny. Porownanie distSq < minR*minR daje ten sam wynik co dist < "
    "minR, ale bez pierwiastka. Sqrt licze dopiero, gdy faktycznie musze wypchnac gracza."))

build("Rogert_Osoba2.pdf", "Rogert (Osoba 2) - Prymitywy 3D i kolizje", story)
