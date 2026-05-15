#ifndef ENGINE_H
#define ENGINE_H

#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// Typ wyliczeniowy do zmiany aktywnego rzutowania
enum class ProjectionType { ORTHOGRAPHIC, PERSPECTIVE };

// Klasa do łatwego rysowania obiektów zdefiniowanych we FreeGLUT (Zadanie 7)
class Primitive {
public:
    static void drawCube(float size, bool solid = true);
    static void drawSphere(float radius, int slices, int stacks, bool solid = true);
    static void drawTeapot(float size, bool solid = true);
    // Proceduralny walec (fallback zamiast GLU)
    static void drawCylinder(float radius, float height, int slices = 24, bool solid = true);
};

// Zaimplementowana klasa silnika (Zadanie 6)
class Engine {
private:
    static Engine* instance; // Do obsługi statycznych callbacków (GLUT Object Oriented Framework)

    // Parametryzacja trybu graficznego i okna
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    bool isFullscreen;

    // Parametryzacja innych rzeczy
    int targetFPS;
    bool enableDepthBuffer;
    bool enableDoubleBuffer;
    
    // Rzutowanie
    ProjectionType currentProjection;
    glm::mat4 projectionMatrix;

    // Tablica stanów klawiatury
    bool keys[256];

    // Stan myszy i kamery do prostego orbitowania
    bool mouseLeftDown;
    int lastMouseX;
    int lastMouseY;

    // Kamera (orbit): target + spherical coords
    glm::vec3 camTarget; // punkt na który patrzymy
    float camYaw;   // obrót wokół osi Y (radiany)
    float camPitch; // obrót góra/dół (radiany)
    float camDistance; // odległość od targetu

    // Aktualnie rysowany prymityw
    enum class PrimitiveType { CUBE, CYLINDER } currentPrimitive;
    // Tryb rysowania (solid / wireframe)
    bool wireframeMode;

    // Licznik FPS
    int frameCount;
    float fpsValue;
    int lastFPSTime;

    // Prywatne metody obsługi wywoływane przez callbacki
    void render();
    void resize(int width, int height);
    void handleKeyboard(unsigned char key, int x, int y, bool isDown);
    void handleMouse(int button, int state, int x, int y);
    void handleMouseMotion(int x, int y);
    void onTimer();

    

public:
    // Getter/Setter dla targetFPS (publiczne)
    void setTargetFPS(int fps);
    int getTargetFPS() const;

    // Przełącznik trybu rysowania solid/wireframe
    void toggleWireframe();
    Engine();
    ~Engine();

    // Inicjacja biblioteki
    void init(int argc, char** argv);

    // Parametryzowanie trybu graficznego (rozdzielczość, pełny ekran)
    void setWindowParams(int width, int height, const std::string& title, bool fullscreen = false);

    // Parametryzowanie innych rzeczy (FPS, wielokrotne buforowanie, bufor Z)
    void setGraphicsParams(int fps, bool depth, bool doubleBuffering);

    // Obsługa czyszczenia ekranu do zadanego koloru
    void setClearColor(float r, float g, float b, float a);

    // Obsługa zmiany aktywnego rzutowania
    void setProjection(ProjectionType type);

    // Główna pętla gry
    void run();

    // Zamknięcie gry
    void shutdown();

    // Statyczne metody zwrotne dla FreeGLUT
    static void displayCallback();
    static void reshapeCallback(int width, int height);
    static void keyboardDownCallback(unsigned char key, int x, int y);
    static void keyboardUpCallback(unsigned char key, int x, int y);
    static void mouseCallback(int button, int state, int x, int y);
    static void motionCallback(int x, int y);
    static void timerCallback(int value);
};

#endif