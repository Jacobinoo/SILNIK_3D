/**
 * @file Engine.h
 * @brief Glowna klasa silnika - okno, kontekst GL, petla, wejscie, callbacki.
 *
 * Engine to kontener infrastruktury - tworzy okno przez FreeGLUT, ustawia
 * kontekst OpenGL, prowadzi petle renderowania, obsluguje wejscie. Sama nie
 * zawiera zadnej "gry" - gra wstawia sie przez callbacki (updateCallback,
 * shootCallback itd.) i buduje swoja scene jako dzieci grafu (getSceneRoot()).
 */
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

/** @brief Typ projekcji uzywanej przez kamere. */
enum class ProjectionType    { ORTHOGRAPHIC, PERSPECTIVE };

/**
 * @brief Kto zarzadza kamera kazdego frame'a.
 *
 * - ENGINE_ORBIT: silnik sam aktualizuje orbit camera z WASD i myszki
 *   (tryb testowy/demo silnika)
 * - GAME_CONTROLLED: silnik nie rusza kamery; gra w swoim updateCallback
 *   sama wola camera->setFirstPerson(...) z aktualnym stanem
 */
enum class CameraControlMode { ENGINE_ORBIT, GAME_CONTROLLED };

/**
 * @brief Glowna klasa silnika 3D - infrastruktura okna + petli + wejscia.
 *
 * @details Cykl zycia:
 * 1. Utworzenie obiektu Engine (konstruktor robi minimum - bez GL).
 * 2. engine.init(argc, argv) - inicjalizuje GLUT.
 * 3. engine.setWindowParams(...), setGraphicsParams(...) - tworzy okno
 *    i kontekst GL. Od tego momentu mozna tworzyc tekstury / wgrywac BMP.
 * 4. Gra konstruuje swoje obiekty (np. ShootingGallery), dodaje do sceny,
 *    rejestruje callbacki.
 * 5. engine.run() - oddaje sterowanie GLUTowi (glutMainLoop), nie wraca
 *    az do shutdown.
 *
 * Wewnetrznie: pojedyncza instancja (singleton-like przez `static Engine* instance`)
 * zeby statyczne callbacki GLUT mogly dotrzec do tej instancji.
 */
class Engine {
private:
    /** @brief Singleton-pointer - GLUT wola statyczne callbacki ktore odwoluja sie tu. */
    static Engine* instance;

    // ---- Okno ----
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    bool isFullscreen;

    // ---- Grafika ----
    int  targetFPS;
    bool enableDepthBuffer;
    bool enableDoubleBuffer;
    ProjectionType    currentProjection;
    CameraControlMode cameraControl;

    // ---- Stany renderowania ----
    bool wireframeMode;     ///< klawisz M
    bool lightingEnabled;   ///< klawisz L
    bool smoothShading;     ///< klawisz G
    bool paused;            ///< klawisz ESC

    // ---- Wejscie ----
    bool keys[256];         ///< stany klawiatury (true = klawisz wciskany)
    bool mouseLeftDown;
    bool freeMouseLook;     ///< FPS-style: kursor schowany, motion zawsze aktywny
    int  lastMouseX;
    int  lastMouseY;
    float mouseSensitivity;

    // ---- Stan kamery (wspolny dla obu trybow) ----
    Vec3  cameraTarget;
    float cameraYaw;
    float cameraPitch;
    float cameraDistance;

    // ---- Graf sceny ----
    std::shared_ptr<SceneNode>  sceneRoot;
    std::shared_ptr<Camera>     observer;
    std::shared_ptr<PointLight> pointLight;

    // ---- Licznik FPS i czasu ----
    int   frameCount;
    float fpsValue;
    int   lastFPSTime;
    int   lastTickMs;       ///< czas ostatniego onTimer (do liczenia dt)

    // ---- Callbacki gry ----
    std::function<void(float)> updateCallback;  ///< co klatke (z dt)
    std::function<void()>      shootCallback;   ///< SPACJA albo LPM
    std::function<void()>      resetCallback;   ///< R
    std::function<void()>      hudCallback;     ///< rysowanie HUD gry (w 2D)

    // ---- Metody prywatne (callback handlers + helpers) ----
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
    /** @brief Konstruktor - inicjuje pola, NIE tworzy okna ani GL. */
    Engine();
    /** @brief Destruktor - shutdown. */
    ~Engine();

    /** @brief Inicjalizuje GLUT. Wywolaj jako pierwsze. */
    void init(int argc, char** argv);

    /** @brief Ustawia parametry okna (nie tworzy go jeszcze). */
    void setWindowParams(int width, int height, const std::string& title, bool fullscreen = false);

    /**
     * @brief Tworzy okno + kontekst GL. Po tym mozna juz uzywac OpenGL.
     * @param fps docelowy FPS (uzywany jako interwal timera)
     * @param depth czy uzywac z-buffer
     * @param doubleBuffering double-buffered rendering
     */
    void setGraphicsParams(int fps, bool depth, bool doubleBuffering);

    void setClearColor(float r, float g, float b, float a);
    /** @brief Ustawia globalny kolor ambient (GL_LIGHT_MODEL_AMBIENT). */
    void setGlobalAmbient(float r, float g, float b);
    void setProjection(ProjectionType type);

    /** @brief Wlacza glowna petle GLUT. Nie wraca az do shutdown. */
    void run();
    void shutdown();

    // ---- FPS / stan renderera ----
    void setTargetFPS(int fps);
    int  getTargetFPS() const;
    void toggleWireframe();  ///< klawisz M
    void toggleLighting();   ///< klawisz L
    void toggleShading();    ///< klawisz G
    void togglePause();      ///< klawisz ESC
    bool isPaused() const { return paused; }
    int  windowW() const { return windowWidth;  }
    int  windowH() const { return windowHeight; }

    // ---- Kontrola kamery ----
    void setCameraControl(CameraControlMode m) { cameraControl = m; }
    CameraControlMode cameraControlMode() const { return cameraControl; }
    float getCameraYaw()   const { return cameraYaw;   }
    float getCameraPitch() const { return cameraPitch; }
    void  setCameraYawPitch(float y, float p) { cameraYaw = y; cameraPitch = p; }
    /** @brief Czy klawisz jest aktualnie wcisniety (dostep do tablicy keys[]). */
    bool  isKeyDown(unsigned char k) const { return keys[k]; }
    /**
     * @brief Wlacza FPS-style mouse look.
     *
     * Gdy true: kursor schowany, motion zawsze aktywny (warp na srodek
     * po kazdym frame). Gdy false: domyslny GLUT - kursor widoczny, motion
     * tylko gdy LPM trzymane.
     */
    void  setFreeMouseLook(bool enabled);
    void  setMouseSensitivity(float s) { mouseSensitivity = s; }

    // ---- Dostep do sceny ----
    std::shared_ptr<Camera>     getCamera()     const { return observer;   }
    std::shared_ptr<PointLight> getPointLight() const { return pointLight; }
    /** @brief Korzen grafu sceny - gra dodaje swoje obiekty jako jego dzieci. */
    std::shared_ptr<SceneNode>  getSceneRoot()  const { return sceneRoot;  }

    // ---- Callbacki gry ----
    /** @brief Co klatke, dostaje dt w sekundach. */
    void setUpdateCallback(std::function<void(float)> cb) { updateCallback = std::move(cb); }
    /** @brief Strzal (SPACJA albo LPM w trybie freeMouseLook). */
    void setShootCallback (std::function<void()>      cb) { shootCallback  = std::move(cb); }
    /** @brief Reset (klawisz R). */
    void setResetCallback (std::function<void()>      cb) { resetCallback  = std::move(cb); }
    /** @brief Rysowanie HUD gry. Wolane w trybie 2D ortho po renderowaniu sceny. */
    void setHUDCallback   (std::function<void()>      cb) { hudCallback    = std::move(cb); }

    // ---- Pomocniki HUD ----
    /** @brief Rysuje tekst w 2D, czcionka 12pt (HUD'owa). */
    void drawString(int x, int y, const std::string& text) const;
    /** @brief Rysuje tekst, wieksza czcionka (do naglowkow Game Over itd). */
    void drawStringLarge(int x, int y, const std::string& text) const;

    // ---- Callbacki dla FreeGLUT (statyczne - GLUT ich wymaga jako C-funkcje) ----
    static void displayCallback();
    static void reshapeCallback(int width, int height);
    static void keyboardDownCallback(unsigned char key, int x, int y);
    static void keyboardUpCallback(unsigned char key, int x, int y);
    static void mouseCallback(int button, int state, int x, int y);
    static void motionCallback(int x, int y);
    static void timerCallback(int value);
};

#endif
