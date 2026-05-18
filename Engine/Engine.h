#ifndef ENGINE_H
#define ENGINE_H

#include "Camera.h"
#include "Light.h"
#include "PrimitiveNode.h"
#include "Texture.h"
#include <GL/freeglut.h>
#include <memory>
#include <string>

enum class ProjectionType { ORTHOGRAPHIC, PERSPECTIVE };

// Główna klasa silnika 3D.
// Zarządza sceną, kamerą, oświetleniem, teksturami i pętlą renderowania.
class Engine {
private:
    static Engine* instance;

    // Okno
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    bool isFullscreen;

    // Grafika
    int targetFPS;
    bool enableDepthBuffer;
    bool enableDoubleBuffer;
    ProjectionType currentProjection;

    // Stany renderowania
    bool wireframeMode;
    bool lightingEnabled;
    bool smoothShading;

    // Wejście: klawiatura i mysz
    bool keys[256];
    bool mouseLeftDown;
    int lastMouseX;
    int lastMouseY;

    // Kamera orbitalna
    Vec3 cameraTarget;
    float cameraYaw;
    float cameraPitch;
    float cameraDistance;

    // Graf sceny
    std::shared_ptr<SceneNode>    sceneRoot;
    std::shared_ptr<Camera>       observer;
    std::shared_ptr<PointLight>   pointLight;
    std::shared_ptr<CubeNode>     cube;
    std::shared_ptr<CylinderNode> cylinder;
    std::shared_ptr<SphereNode>   sphere;
    std::shared_ptr<PlaneNode>    plane;

    // Tekstury proceduralne
    std::shared_ptr<Texture> checkerTex;
    std::shared_ptr<Texture> stripeTex;
    std::shared_ptr<Texture> gradientTex;

    // Licznik FPS
    int frameCount;
    float fpsValue;
    int lastFPSTime;

    // Prywatne metody
    void render();
    void resize(int width, int height);
    void handleKeyboard(unsigned char key, int x, int y, bool isDown);
    void handleMouse(int button, int state, int x, int y);
    void handleMouseMotion(int x, int y);
    void onTimer();

    void applyLightingState() const;
    void updateProjection();
    void drawHUD() const;
    void drawString(int x, int y, const std::string& text) const;

public:
    Engine();
    ~Engine();

    void init(int argc, char** argv);
    void setWindowParams(int width, int height, const std::string& title, bool fullscreen = false);
    void setGraphicsParams(int fps, bool depth, bool doubleBuffering);
    void setClearColor(float r, float g, float b, float a);
    void setProjection(ProjectionType type);
    void run();
    void shutdown();

    void setTargetFPS(int fps);
    int  getTargetFPS() const;
    void toggleWireframe();
    void toggleLighting();
    void toggleShading();

    std::shared_ptr<Camera>       getCamera()     const;
    std::shared_ptr<PointLight>   getPointLight() const;
    std::shared_ptr<CubeNode>     getCube()       const;
    std::shared_ptr<CylinderNode> getCylinder()   const;
    std::shared_ptr<SphereNode>   getSphere()     const;
    std::shared_ptr<PlaneNode>    getPlane()      const;
    std::shared_ptr<SceneNode>    getSceneRoot()  const;

    // Callbacki dla FreeGLUT (muszą być statyczne)
    static void displayCallback();
    static void reshapeCallback(int width, int height);
    static void keyboardDownCallback(unsigned char key, int x, int y);
    static void keyboardUpCallback(unsigned char key, int x, int y);
    static void mouseCallback(int button, int state, int x, int y);
    static void motionCallback(int x, int y);
    static void timerCallback(int value);
};

#endif
