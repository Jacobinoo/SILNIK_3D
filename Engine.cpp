#include "Engine.h"
#include <iostream>

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


Engine::Engine() : windowWidth(800), windowHeight(600), isFullscreen(false), 
                   targetFPS(60), enableDepthBuffer(true), enableDoubleBuffer(true),
                   currentProjection(ProjectionType::PERSPECTIVE) {
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
    
    // Główna pętla gry korzystająca z czasomierza
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

    // Odsunięcie kamery, by widzieć obiekty
    glTranslatef(0.0f, 0.0f, -5.0f);

    // Przykładowe użycie naszej klasy ułatwiającej rysowanie z zadania 7
    glColor3f(1.0f, 0.5f, 0.2f);
    // Primitive::drawTeapot(1.5f, true);
    Primitive::drawSphere(1.0f, 20, 20, false);
    Primitive::drawCube(1.0f, true);
    
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
    }
}

void Engine::handleMouse(int button, int state, int x, int y) {
     // Miejsce na logikę kliknięć (np. strzelanie, wybór obiektu)
}

void Engine::handleMouseMotion(int x, int y) {
     // Miejsce na ruch myszy do obsługi kamery
}

void Engine::onTimer() {
    // Wymuszenie odrysowania klatki
    glutPostRedisplay();
    
    // Ponowne zaplanowanie wywołania czasomierza
    glutTimerFunc(1000 / targetFPS, timerCallback, 0);
}

// --- ZWROTNE METODY STATYCZNE DLA FREEGLUT ---
void Engine::displayCallback() { if (instance) instance->render(); }
void Engine::reshapeCallback(int w, int h) { if (instance) instance->resize(w, h); }
void Engine::keyboardDownCallback(unsigned char k, int x, int y) { if (instance) instance->handleKeyboard(k, x, y, true); }
void Engine::keyboardUpCallback(unsigned char k, int x, int y) { if (instance) instance->handleKeyboard(k, x, y, false); }
void Engine::mouseCallback(int b, int s, int x, int y) { if (instance) instance->handleMouse(b, s, x, y); }
void Engine::motionCallback(int x, int y) { if (instance) instance->handleMouseMotion(x, y); }
void Engine::timerCallback(int val) { if (instance) instance->onTimer(); }