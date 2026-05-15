#include "Engine.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// Inicjalizacja statycznego wskaźnika
Engine* Engine::instance = nullptr;


void Primitive::drawCube(float size, bool solid) {
    if (solid) glutSolidCube(size);
    else glutWireCube(size);
}

void Primitive::drawSphere(float radius, int slices, int stacks, bool solid) {
    if (solid) glutSolidSphere(radius, slices, stacks);
    else glutWireSphere(radius, slices, stacks);
}

void Primitive::drawTeapot(float size, bool solid) {
    if (solid) glutSolidTeapot(size);
    else glutWireTeapot(size);
}

void Primitive::drawCylinder(float radius, float height, int slices, bool solid) {
    // Proceduralny walec: srodek na 0, wysokość w osi Y od -height/2 do +height/2
    const float halfH = height * 0.5f;
    const float TWO_PI = 2.0f * 3.14159265358979323846f;

    if (solid) {
        // Boki (triangle strip)
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float t = (float)i / (float)slices;
            float ang = t * TWO_PI;
            float x = cosf(ang) * radius;
            float z = sinf(ang) * radius;
            // normal
            glNormal3f(x, 0.0f, z);
            // górny wierzchołek
            glVertex3f(x, halfH, z);
            // dolny wierzchołek
            glVertex3f(x, -halfH, z);
        }
        glEnd();

        // Górna pokrywa
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, halfH, 0.0f);
        for (int i = 0; i <= slices; ++i) {
            float t = (float)i / (float)slices;
            float ang = t * TWO_PI;
            float x = cosf(ang) * radius;
            float z = sinf(ang) * radius;
            glVertex3f(x, halfH, z);
        }
        glEnd();

        // Dolna pokrywa
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(0.0f, -halfH, 0.0f);
        for (int i = 0; i <= slices; ++i) {
            float t = (float)i / (float)slices;
            float ang = -t * TWO_PI; // odwrotne kolejność dla normaly
            float x = cosf(ang) * radius;
            float z = sinf(ang) * radius;
            glVertex3f(x, -halfH, z);
        }
        glEnd();
    } else {
        // Wireframe: obwód i linie pionowe
        // obwód górny
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < slices; ++i) {
            float t = (float)i / (float)slices;
            float ang = t * TWO_PI;
            float x = cosf(ang) * radius;
            float z = sinf(ang) * radius;
            glVertex3f(x, halfH, z);
        }
        glEnd();

        // obwód dolny
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < slices; ++i) {
            float t = (float)i / (float)slices;
            float ang = t * TWO_PI;
            float x = cosf(ang) * radius;
            float z = sinf(ang) * radius;
            glVertex3f(x, -halfH, z);
        }
        glEnd();

        // linie pionowe
        glBegin(GL_LINES);
        for (int i = 0; i < slices; ++i) {
            float t = (float)i / (float)slices;
            float ang = t * TWO_PI;
            float x = cosf(ang) * radius;
            float z = sinf(ang) * radius;
            glVertex3f(x, -halfH, z);
            glVertex3f(x, halfH, z);
        }
        glEnd();
    }
}


Engine::Engine() : windowWidth(800), windowHeight(600), isFullscreen(false), 
                   targetFPS(60), enableDepthBuffer(true), enableDoubleBuffer(true),
                   currentProjection(ProjectionType::PERSPECTIVE),
                   mouseLeftDown(false), lastMouseX(0), lastMouseY(0),
                   camTarget(0.0f, 0.0f, 0.0f), camYaw(0.0f), camPitch(0.0f), camDistance(5.0f),
                   currentPrimitive(PrimitiveType::CUBE), wireframeMode(false),
                   frameCount(0), fpsValue(0.0f), lastFPSTime(0) {
    instance = this;
    for (int i = 0; i < 256; ++i) keys[i] = false;
}

Engine::~Engine() {
    shutdown();
}

void Engine::init(int argc, char** argv) {
    // Inicjacja biblioteki odpowiedzialnej za system okienkowy
    glutInit(&argc, argv);
}

void Engine::setWindowParams(int width, int height, const std::string& title, bool fullscreen) {
    windowWidth = width;
    windowHeight = height;
    windowTitle = title;
    isFullscreen = fullscreen;
}

void Engine::setGraphicsParams(int fps, bool depth, bool doubleBuffering) {
    targetFPS = fps;
    enableDepthBuffer = depth;
    enableDoubleBuffer = doubleBuffering;

    // Parametryzowanie buforów
    unsigned int displayMode = GLUT_RGBA;
    if (enableDoubleBuffer) displayMode |= GLUT_DOUBLE;
    if (enableDepthBuffer) displayMode |= GLUT_DEPTH;
    
    glutInitDisplayMode(displayMode);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow(windowTitle.c_str());

    if (isFullscreen) {
        glutFullScreen();
    }

    if (enableDepthBuffer) {
        glEnable(GL_DEPTH_TEST);
    }

    // Rejestracja obsługi wejścia i wyświetlania
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutMouseFunc(mouseCallback);
    glutPassiveMotionFunc(motionCallback);
    glutMotionFunc(motionCallback); // aktywne przesuwanie myszy (z przyciskiem wciśniętym)
    
    // Główna pętla gry korzystająca z czasomierza
    // Zainicjuj licznik FPS
    lastFPSTime = glutGet(GLUT_ELAPSED_TIME);
    frameCount = 0;
    fpsValue = 0.0f;

    glutTimerFunc(1000 / targetFPS, timerCallback, 0);
}

void Engine::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Engine::setProjection(ProjectionType type) {
    currentProjection = type;
    // Wymuszenie przeliczenia macierzy przy zmianie
    resize(windowWidth, windowHeight); 
}

void Engine::run() {
    glutMainLoop(); // Start pętli
}

void Engine::shutdown() {
    // Deinicjacja i sprzątanie pamięci
    std::cout << "Zamykanie silnika..." << std::endl;
    // Starsze wersje OpenGL mogą tu wymagać glutLeaveMainLoop(), a my używamy exit() pod ESC
}

void Engine::render() {
    // Obsługa czyszczenia ekranu do zadanego koloru i czyszczenie bufora głębi
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Aplikowanie macierzy rzutowania wyliczonej przez bibliotekę GLM
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    
    // Resetowanie macierzy widoku/modelu
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Wyliczenie pozycji kamery w koordynatach sferycznych względem camTarget
    float cx = camTarget.x + camDistance * cosf(camPitch) * sinf(camYaw);
    float cy = camTarget.y + camDistance * sinf(camPitch);
    float cz = camTarget.z + camDistance * cosf(camPitch) * cosf(camYaw);

    glm::vec3 eye(cx, cy, cz);
    glm::vec3 center = camTarget;
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::mat4 view = glm::lookAt(eye, center, up);
    glLoadMatrixf(glm::value_ptr(view));

    // Przykładowe użycie naszej klasy ułatwiającej rysowanie z zadania 7
    // Rysujemy dwa prymitywy jednocześnie: sześcian (z lewej) i walec (z prawej)
    // Używamy trybu wireframeMode aby zadecydować czy rysujemy solid czy wireframe
    glPushMatrix();
    glColor3f(0.8f, 0.4f, 0.1f);
    glTranslatef(-1.5f, 0.0f, 0.0f);
    Primitive::drawCube(1.0f, !wireframeMode);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.2f, 0.6f, 0.9f);
    glTranslatef(1.5f, 0.0f, 0.0f);
    Primitive::drawCylinder(0.7f, 2.0f, 36, !wireframeMode);
    glPopMatrix();

    // Aktualizacja licznika FPS
    ++frameCount;
    int now = glutGet(GLUT_ELAPSED_TIME);
    int elapsed = now - lastFPSTime;
    if (elapsed >= 1000) {
        fpsValue = frameCount * 1000.0f / (float)elapsed;
        frameCount = 0;
        lastFPSTime = now;
    }

    // Rysowanie HUD (FPS) w lewym górnym rogu
    // Zachowaj aktualne macierze
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Ustaw ortho zgodne z pikselami okna
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Rysuj tekst (wyłącz głębokość)
    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 1.0f, 1.0f);
    // Pozycja tekstu: 10 px od lewej, 20 px od góry
    std::string fpsText = std::string("FPS: ") + std::to_string((int)(fpsValue + 0.5f));
    glRasterPos2i(10, windowHeight - 20);
    for (char c : fpsText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);

    // Przywróć macierze
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    if (enableDoubleBuffer) {
        glutSwapBuffers();
    } else {
        glFlush();
    }
}

void Engine::resize(int width, int height) {
    if (height == 0) height = 1;
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);

    float aspect = (float)width / (float)height;

    // Przeliczenie macierzy za pomocą biblioteki GLM
    if (currentProjection == ProjectionType::PERSPECTIVE) {
        // Rzutowanie perspektywiczne: kąt widzenia 60 stopni, stosunek boków, zNear=1.0, zFar=100.0
        projectionMatrix = glm::perspective(glm::radians(60.0f), aspect, 1.0f, 100.0f);
    } else {
        // Rzutowanie ortogonalne
        projectionMatrix = glm::ortho(-10.0f * aspect, 10.0f * aspect, -10.0f, 10.0f, 1.0f, 100.0f);
    }
}

void Engine::handleKeyboard(unsigned char key, int x, int y, bool isDown) {
    keys[key] = isDown;
    
    if (isDown) {
        if (key == 27) { // Klawisz ESC = zamknięcie gry
            shutdown();
            exit(0);
        }
        else if (key == 'p') {
            setProjection(ProjectionType::PERSPECTIVE); // Zmiana aktywnego rzutowania
        }
        else if (key == 'o') {
            setProjection(ProjectionType::ORTHOGRAPHIC); // Zmiana aktywnego rzutowania
        }
        else if (key == 'm') {
            // Przełącz tryb rysowania
            toggleWireframe();
        }
    }
}

void Engine::handleMouse(int button, int state, int x, int y) {
    // Lewy przycisk -> orbitowanie
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mouseLeftDown = false;
        }
    }

    // Scroll: w GLUT często reprezentowany jako przyciski 3 (up) i 4 (down)
    if (button == 3) { // wheel up
        camDistance -= 0.3f;
        if (camDistance < 0.2f) camDistance = 0.2f;
    } else if (button == 4) { // wheel down
        camDistance += 0.3f;
    }
}

void Engine::handleMouseMotion(int x, int y) {
    if (!mouseLeftDown) return;

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    const float sensX = 0.005f;
    const float sensY = 0.005f;

    camYaw += dx * sensX;
    camPitch += -dy * sensY;

    // ograniczenie pitcha (nie obracamy całkowicie do góry nogi)
    const float limit = glm::radians(89.0f);
    if (camPitch > limit) camPitch = limit;
    if (camPitch < -limit) camPitch = -limit;

    lastMouseX = x;
    lastMouseY = y;
}



void Engine::onTimer() {
    // Ciągłe przetwarzanie stanu klawiszy (ruch kamery)
    float moveSpeed = 0.05f * camDistance; // skaluj prędkość wraz z odległością
    // Kierunki ruchu wyliczone względem aktualnego widoku kamery.
    // front wskazuje tam, gdzie patrzy kamera, więc W przesuwa zgodnie z widokiem,
    // a D faktycznie idzie w prawo na ekranie.
    glm::vec3 eye(
        camTarget.x + camDistance * cosf(camPitch) * sinf(camYaw),
        camTarget.y + camDistance * sinf(camPitch),
        camTarget.z + camDistance * cosf(camPitch) * cosf(camYaw)
    );
    glm::vec3 forward = glm::normalize(camTarget - eye);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (keys['w'] || keys['W']) camTarget += forward * moveSpeed;
    if (keys['s'] || keys['S']) camTarget -= forward * moveSpeed;
    if (keys['a'] || keys['A']) camTarget -= right * moveSpeed;
    if (keys['d'] || keys['D']) camTarget += right * moveSpeed;
    if (keys['q'] || keys['Q']) camTarget.y += moveSpeed;
    if (keys['e'] || keys['E']) camTarget.y -= moveSpeed;

    // Zmiana prymitywu (jednorazowe przełączenia obsługujemy na zdarzeniu keydown)
    if (keys['1']) currentPrimitive = PrimitiveType::CUBE;
    if (keys['2']) currentPrimitive = PrimitiveType::CYLINDER;

    // Zmiana targetFPS
    if (keys['+'] || keys['=']) {
        setTargetFPS(getTargetFPS() + 5);
    }
    if (keys['-'] || keys['_']) {
        setTargetFPS(std::max(1, getTargetFPS() - 5));
    }

    // Wymuszenie odrysowania klatki — tylko jeśli jest aktywne okno
    if (glutGetWindow() != 0) {
        glutPostRedisplay();
    }

    // Ponowne zaplanowanie wywołania czasomierza
    glutTimerFunc(1000 / targetFPS, timerCallback, 0);
}

void Engine::setTargetFPS(int fps) {
    targetFPS = fps > 0 ? fps : 1;
}

int Engine::getTargetFPS() const {
    return targetFPS;
}

void Engine::toggleWireframe() {
    wireframeMode = !wireframeMode;
}

// --- ZWROTNE METODY STATYCZNE DLA FREEGLUT ---
void Engine::displayCallback() { if (instance) instance->render(); }
void Engine::reshapeCallback(int w, int h) { if (instance) instance->resize(w, h); }
void Engine::keyboardDownCallback(unsigned char k, int x, int y) { if (instance) instance->handleKeyboard(k, x, y, true); }
void Engine::keyboardUpCallback(unsigned char k, int x, int y) { if (instance) instance->handleKeyboard(k, x, y, false); }
void Engine::mouseCallback(int b, int s, int x, int y) { if (instance) instance->handleMouse(b, s, x, y); }
void Engine::motionCallback(int x, int y) { if (instance) instance->handleMouseMotion(x, y); }
void Engine::timerCallback(int val) { if (instance) instance->onTimer(); }