#ifndef ENGINE_H
#define ENGINE_H

#include "Camera.h"
#include "Light.h"
#include "PrimitiveNode.h"
#include <GL/freeglut.h>
#include <memory>
#include <string>

// Typ wyliczeniowy do zmiany aktywnego rzutowania
enum class ProjectionType { ORTHOGRAPHIC, PERSPECTIVE };

// Silnik sceny 3D z hierarchią węzłów, kamerą-obserwatorem i oświetleniem punktowym
class Engine {
private:
    static Engine* instance;

    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    bool isFullscreen;

    int targetFPS;
    bool enableDepthBuffer;
    bool enableDoubleBuffer;

    ProjectionType currentProjection;

    bool keys[256];
    bool mouseLeftDown;
    int lastMouseX;
    int lastMouseY;

    Vec3 cameraTarget;
    float cameraYaw;
    float cameraPitch;
    float cameraDistance;

    std::shared_ptr<SceneNode> sceneRoot;
    std::shared_ptr<Camera> observer;
    std::shared_ptr<PointLight> pointLight;
    std::shared_ptr<CubeNode> cube;
    std::shared_ptr<CylinderNode> cylinder;

    bool wireframeMode;
    int frameCount;
    float fpsValue;
    int lastFPSTime;

    void render();
    void resize(int width, int height);
    void handleKeyboard(unsigned char key, int x, int y, bool isDown);
    void handleMouse(int button, int state, int x, int y);
    void handleMouseMotion(int x, int y);
    void onTimer();

    void configureLightingState() const;
    void updateProjection();

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
    int getTargetFPS() const;
    void toggleWireframe();

    std::shared_ptr<Camera> getCamera() const;
    std::shared_ptr<PointLight> getPointLight() const;
    std::shared_ptr<CubeNode> getCube() const;
    std::shared_ptr<CylinderNode> getCylinder() const;
    std::shared_ptr<SceneNode> getSceneRoot() const;

    static void displayCallback();
    static void reshapeCallback(int width, int height);
    static void keyboardDownCallback(unsigned char key, int x, int y);
    static void keyboardUpCallback(unsigned char key, int x, int y);
    static void mouseCallback(int button, int state, int x, int y);
    static void motionCallback(int x, int y);
    static void timerCallback(int value);
};

#endif