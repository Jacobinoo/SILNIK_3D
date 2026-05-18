#ifndef ENGINE_H
#define ENGINE_H

#include "Camera.h"
#include "Light.h"
#include "PrimitiveNode.h"
#include "Texture.h"
#include <GL/freeglut.h>
#include <functional>
#include <memory>
#include <string>

enum class ProjectionType    { ORTHOGRAPHIC, PERSPECTIVE };
enum class CameraControlMode { ENGINE_ORBIT, GAME_CONTROLLED };

// Glowny silnik 3D - infrastruktura.
// Zapewnia okno (FreeGLUT), kontekst GL, graf sceny, kamere, oswietlenie,
// petle renderujaca z licznikiem FPS oraz callbacki dla logiki gry.
// Konkretna scena demo / gra budowana jest przez kod uzytkownika.
class Engine {
private:
    static Engine* instance;

    // Okno
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    bool isFullscreen;

    // Grafika
    int  targetFPS;
    bool enableDepthBuffer;
    bool enableDoubleBuffer;
    ProjectionType    currentProjection;
    CameraControlMode cameraControl;

    // Stany renderowania
    bool wireframeMode;
    bool lightingEnabled;
    bool smoothShading;

    // Wejscie
    bool keys[256];
    bool mouseLeftDown;
    bool freeMouseLook;       // FPS-style: kursor schowany, motion zawsze aktywny
    int  lastMouseX;
    int  lastMouseY;
    float mouseSensitivity;

    // Stan kamery (wspolny dla orbitu i FP)
    Vec3  cameraTarget;
    float cameraYaw;
    float cameraPitch;
    float cameraDistance;

    // Graf sceny
    std::shared_ptr<SceneNode>  sceneRoot;
    std::shared_ptr<Camera>     observer;
    std::shared_ptr<PointLight> pointLight;

    // Licznik FPS i czasu
    int   frameCount;
    float fpsValue;
    int   lastFPSTime;
    int   lastTickMs;

    // Callbacki gry
    std::function<void(float)> updateCallback;
    std::function<void()>      shootCallback;
    std::function<void()>      resetCallback;
    std::function<void()>      hudCallback;

    // Metody prywatne
    void render();
    void resize(int width, int height);
    void handleKeyboard(unsigned char key, int x, int y, bool isDown);
    void handleMouse(int button, int state, int x, int y);
    void handleMouseMotion(int x, int y);
    void onTimer();

    void applyLightingState() const;
    void updateProjection();
    void drawHUD() const;

public:
    Engine();
    ~Engine();

    // Inicjalizacja
    void init(int argc, char** argv);
    void setWindowParams(int width, int height, const std::string& title, bool fullscreen = false);
    void setGraphicsParams(int fps, bool depth, bool doubleBuffering);
    void setClearColor(float r, float g, float b, float a);
    void setProjection(ProjectionType type);
    void run();
    void shutdown();

    // FPS / stan renderera
    void setTargetFPS(int fps);
    int  getTargetFPS() const;
    void toggleWireframe();
    void toggleLighting();
    void toggleShading();
    int  windowW() const { return windowWidth;  }
    int  windowH() const { return windowHeight; }

    // Kontrola kamery
    void setCameraControl(CameraControlMode m) { cameraControl = m; }
    CameraControlMode cameraControlMode() const { return cameraControl; }
    float getCameraYaw()   const { return cameraYaw;   }
    float getCameraPitch() const { return cameraPitch; }
    void  setCameraYawPitch(float y, float p) { cameraYaw = y; cameraPitch = p; }
    bool  isKeyDown(unsigned char k) const { return keys[k]; }
    void  setFreeMouseLook(bool enabled);
    void  setMouseSensitivity(float s) { mouseSensitivity = s; }

    // Dostep do sceny
    std::shared_ptr<Camera>     getCamera()     const { return observer;   }
    std::shared_ptr<PointLight> getPointLight() const { return pointLight; }
    std::shared_ptr<SceneNode>  getSceneRoot()  const { return sceneRoot;  }

    // Callbacki gry
    void setUpdateCallback(std::function<void(float)> cb) { updateCallback = std::move(cb); }
    void setShootCallback (std::function<void()>      cb) { shootCallback  = std::move(cb); }
    void setResetCallback (std::function<void()>      cb) { resetCallback  = std::move(cb); }
    void setHUDCallback   (std::function<void()>      cb) { hudCallback    = std::move(cb); }

    // Pomocniki HUD (do uzytku w hudCallback w 2D)
    void drawString(int x, int y, const std::string& text) const;
    void drawStringLarge(int x, int y, const std::string& text) const;

    // Callbacki dla FreeGLUT
    static void displayCallback();
    static void reshapeCallback(int width, int height);
    static void keyboardDownCallback(unsigned char key, int x, int y);
    static void keyboardUpCallback(unsigned char key, int x, int y);
    static void mouseCallback(int button, int state, int x, int y);
    static void motionCallback(int x, int y);
    static void timerCallback(int value);
};

#endif
