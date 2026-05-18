#include "Engine.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

Engine* Engine::instance = nullptr;

namespace {
    static const float PI = 3.14159265358979323846f;
    float toRad(float deg) { return deg * PI / 180.0f; }
}

Engine::Engine()
    : windowWidth(800), windowHeight(600), isFullscreen(false),
      targetFPS(60), enableDepthBuffer(true), enableDoubleBuffer(true),
      currentProjection(ProjectionType::PERSPECTIVE),
      wireframeMode(false), lightingEnabled(true), smoothShading(true),
      mouseLeftDown(false), lastMouseX(0), lastMouseY(0),
      cameraTarget(0.0f, 0.0f, 0.0f),
      cameraYaw(0.0f), cameraPitch(0.3f), cameraDistance(8.0f),
      sceneRoot(std::make_shared<SceneNode>("Root")),
      observer(std::make_shared<Camera>()),
      pointLight(std::make_shared<PointLight>()),
      cube(std::make_shared<CubeNode>(1.2f)),
      cylinder(std::make_shared<CylinderNode>(0.6f, 2.0f, 36)),
      sphere(std::make_shared<SphereNode>(0.8f, 24, 48)),
      plane(std::make_shared<PlaneNode>(10.0f, 10.0f)),
      checkerTex(std::make_shared<Texture>()),
      stripeTex(std::make_shared<Texture>()),
      gradientTex(std::make_shared<Texture>()),
      frameCount(0), fpsValue(0.0f), lastFPSTime(0)
{
    instance = this;

    for (int i = 0; i < 256; ++i) keys[i] = false;

    // ---- Kamera ----
    observer->setTarget(cameraTarget);
    observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);
    observer->setProjectionPerspective(60.0f,
        (float)windowWidth / (float)windowHeight, 0.1f, 200.0f);

    // ---- Oświetlenie ----
    pointLight->setPosition(Vec3(3.0f, 5.0f, 4.0f));
    pointLight->setAmbient(Vec3(0.15f, 0.15f, 0.15f));
    pointLight->setDiffuse(Vec3(1.0f,  1.0f,  1.0f));
    pointLight->setSpecular(Vec3(1.0f, 1.0f,  1.0f));
    pointLight->setAttenuation(1.0f, 0.045f, 0.009f);
    pointLight->setLightIndex(0);

    // ---- Sześcian ----
    cube->setPosition(Vec3(-2.0f, 0.6f, 0.0f));
    cube->setMaterial(Material(
        Vec3(0.15f, 0.07f, 0.02f),
        Vec3(0.85f, 0.50f, 0.15f),
        Vec3(0.95f, 0.95f, 0.95f), 32.0f));

    // ---- Walec ----
    cylinder->setPosition(Vec3(2.0f, 1.0f, 0.0f));
    cylinder->setMaterial(Material(
        Vec3(0.04f, 0.10f, 0.18f),
        Vec3(0.15f, 0.55f, 0.85f),
        Vec3(0.90f, 0.90f, 0.90f), 64.0f));

    // ---- Sfera ----
    sphere->setPosition(Vec3(0.0f, 0.8f, 0.0f));
    sphere->setMaterial(Material(
        Vec3(0.05f, 0.18f, 0.05f),
        Vec3(0.20f, 0.80f, 0.25f),
        Vec3(0.80f, 0.95f, 0.80f), 96.0f));

    // ---- Podłoga ----
    plane->setPosition(Vec3(0.0f, -0.4f, 0.0f));
    plane->setMaterial(Material(
        Vec3(0.10f, 0.10f, 0.10f),
        Vec3(0.55f, 0.55f, 0.55f),
        Vec3(0.10f, 0.10f, 0.10f), 4.0f));

    // ---- Graf sceny ----
    sceneRoot->addChild(pointLight);
    sceneRoot->addChild(plane);
    sceneRoot->addChild(cube);
    sceneRoot->addChild(cylinder);
    sceneRoot->addChild(sphere);
}

Engine::~Engine() {
    shutdown();
}

void Engine::init(int argc, char** argv) {
    glutInit(&argc, argv);
}

void Engine::setWindowParams(int width, int height, const std::string& title, bool fullscreen) {
    windowWidth  = width;
    windowHeight = height;
    windowTitle  = title;
    isFullscreen = fullscreen;
    updateProjection();
}

void Engine::setGraphicsParams(int fps, bool depth, bool doubleBuffering) {
    targetFPS          = fps;
    enableDepthBuffer  = depth;
    enableDoubleBuffer = doubleBuffering;

    unsigned int displayMode = GLUT_RGBA;
    if (enableDoubleBuffer) displayMode |= GLUT_DOUBLE;
    if (enableDepthBuffer)  displayMode |= GLUT_DEPTH;

    glutInitDisplayMode(displayMode);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow(windowTitle.c_str());
    if (isFullscreen) glutFullScreen();

    if (enableDepthBuffer) glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    GLfloat globalAmbient[4] = { 0.15f, 0.15f, 0.15f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Tryb mieszania tekstury z materiałem
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // ---- Generuj tekstury proceduralne ----
    // Tekstury muszą być wygenerowane PO utworzeniu kontekstu OpenGL

    // Szachownica czarno-biała -> sześcian
    checkerTex->generateCheckerboard(256,
        1.0f, 1.0f, 1.0f,   // biały
        0.1f, 0.1f, 0.1f,   // ciemnoszary
        8);
    cube->setTexture(checkerTex);

    // Pionowe paski niebiesko-pomarańczowe -> walec
    stripeTex->generateStripes(256,
        0.10f, 0.40f, 0.85f,  // niebieski
        0.90f, 0.55f, 0.10f,  // pomarańczowy
        12);
    cylinder->setTexture(stripeTex);

    // Gradient zielono-biały -> sfera
    gradientTex->generateGradient(256,
        0.10f, 0.65f, 0.15f,  // zielony
        0.95f, 0.95f, 0.95f,  // biały
        false);
    sphere->setTexture(gradientTex);

    // Szachownica szara -> podłoga (większe kafle)
    auto groundTex = std::make_shared<Texture>();
    groundTex->generateCheckerboard(256,
        0.70f, 0.70f, 0.70f,
        0.35f, 0.35f, 0.35f,
        4);
    plane->setTexture(groundTex);

    // ---- Callbacki GLUT ----
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutMouseFunc(mouseCallback);
    glutPassiveMotionFunc(motionCallback);
    glutMotionFunc(motionCallback);

    lastFPSTime = glutGet(GLUT_ELAPSED_TIME);
    frameCount  = 0;
    fpsValue    = 0.0f;

    glutTimerFunc(1000 / targetFPS, timerCallback, 0);
}

void Engine::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Engine::setProjection(ProjectionType type) {
    currentProjection = type;
    updateProjection();
}

void Engine::run() {
    glutMainLoop();
}

void Engine::shutdown() {
    std::cout << "Zamykanie silnika..." << std::endl;
}

// ---- Wewnętrzne metody ----

void Engine::applyLightingState() const {
    if (lightingEnabled) {
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    } else {
        glDisable(GL_LIGHTING);
    }
    glShadeModel(smoothShading ? GL_SMOOTH : GL_FLAT);
}

void Engine::updateProjection() {
    if (!observer) return;
    float aspect = (windowHeight == 0) ? 1.0f
                  : (float)windowWidth / (float)windowHeight;
    if (currentProjection == ProjectionType::PERSPECTIVE) {
        observer->setProjectionPerspective(60.0f, aspect, 0.1f, 200.0f);
    } else {
        float s = cameraDistance * 0.5f;
        observer->setProjectionOrthographic(
            -s * aspect, s * aspect, -s, s, 0.1f, 200.0f);
    }
}

void Engine::drawString(int x, int y, const std::string& text) const {
    glRasterPos2i(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

void Engine::drawHUD() const {
    // Przełącz na projekcję 2D (piksele)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(Mat4::orthographic(0.0f, (float)windowWidth,
                                     0.0f, (float)windowHeight,
                                     -1.0f, 1.0f).data());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(Mat4::identity().data());

    GLboolean depthOn = glIsEnabled(GL_DEPTH_TEST);
    GLboolean litOn   = glIsEnabled(GL_LIGHTING);
    GLboolean texOn   = glIsEnabled(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // ---- Informacje w lewym górnym rogu ----
    int y = windowHeight - 18;
    const int step = 16;

    glColor3f(1.0f, 1.0f, 0.3f);
    drawString(10, y, "FPS: " + std::to_string((int)(fpsValue + 0.5f)));
    y -= step;

    glColor3f(lightingEnabled ? 0.3f : 0.8f,
              lightingEnabled ? 0.9f : 0.3f,
              0.3f);
    drawString(10, y, std::string("Oswietlenie [L]: ") +
               (lightingEnabled ? "WL" : "WYL"));
    y -= step;

    glColor3f(smoothShading ? 0.3f : 0.8f,
              smoothShading ? 0.9f : 0.3f,
              0.3f);
    drawString(10, y, std::string("Cieniowanie [G]: ") +
               (smoothShading ? "Gladkie" : "Plaszcz."));
    y -= step;

    glColor3f(wireframeMode ? 0.9f : 0.6f,
              wireframeMode ? 0.5f : 0.6f,
              0.3f);
    drawString(10, y, std::string("Siatka    [M]: ") +
               (wireframeMode ? "WL" : "WYL"));
    y -= step;

    glColor3f(0.7f, 0.7f, 0.7f);
    drawString(10, y, std::string("Rzutowanie [P/O]: ") +
               (currentProjection == ProjectionType::PERSPECTIVE ? "Perspektywiczne" : "Ortogonalne"));

    // ---- Skróty w lewym dolnym rogu ----
    glColor3f(0.6f, 0.6f, 0.6f);
    int yb = 6;
    drawString(10, yb, "ESC=wyjscie  +/-=FPS  WASD/QE=kamera  LPM+mysz=obrót  Scroll=zoom");

    // ---- Przywróć stan ----
    if (depthOn) glEnable(GL_DEPTH_TEST);
    if (litOn)   glEnable(GL_LIGHTING);
    if (texOn)   glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void Engine::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyLightingState();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(observer->projectionMatrix().data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(observer->viewMatrix().data());

    glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

    if (sceneRoot) {
        sceneRoot->renderRecursive(Mat4::identity());
    }

    // Przywróć tryb wypełniania (np. dla HUD)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // ---- Licznik FPS ----
    ++frameCount;
    int now     = glutGet(GLUT_ELAPSED_TIME);
    int elapsed = now - lastFPSTime;
    if (elapsed >= 1000) {
        fpsValue    = frameCount * 1000.0f / (float)elapsed;
        frameCount  = 0;
        lastFPSTime = now;
    }

    drawHUD();

    if (enableDoubleBuffer) {
        glutSwapBuffers();
    } else {
        glFlush();
    }
}

void Engine::resize(int width, int height) {
    if (height == 0) height = 1;
    windowWidth  = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    updateProjection();
}

void Engine::handleKeyboard(unsigned char key, int x, int y, bool isDown) {
    (void)x; (void)y;
    keys[key] = isDown;

    if (!isDown) return;

    switch (key) {
        case 27:  shutdown(); std::exit(0); break;
        case 'p': case 'P': setProjection(ProjectionType::PERSPECTIVE);   break;
        case 'o': case 'O': setProjection(ProjectionType::ORTHOGRAPHIC);  break;
        case 'm': case 'M': toggleWireframe(); break;
        case 'l': case 'L': toggleLighting();  break;
        case 'g': case 'G': toggleShading();   break;
        default: break;
    }
}

void Engine::handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        mouseLeftDown = (state == GLUT_DOWN);
        if (state == GLUT_DOWN) {
            lastMouseX = x;
            lastMouseY = y;
        }
    }

    // Scroll wheel: button 3 = góra, button 4 = dół
    if (button == 3) {
        cameraDistance = std::max(0.5f, cameraDistance - 0.35f);
        observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);
    } else if (button == 4) {
        cameraDistance += 0.35f;
        observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);
    }
}

void Engine::handleMouseMotion(int x, int y) {
    if (!mouseLeftDown) return;

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    cameraYaw   += dx * 0.005f;
    cameraPitch += -dy * 0.005f;
    cameraPitch  = std::max(-toRad(89.0f), std::min(toRad(89.0f), cameraPitch));

    observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);

    lastMouseX = x;
    lastMouseY = y;
}

void Engine::onTimer() {
    // Ruch celu kamery wzdłuż płaszczyzny horyzontalnej
    float moveSpeed = 0.05f * std::max(1.0f, cameraDistance * 0.3f);

    Vec3 eye(
        cameraTarget.x + cameraDistance * std::cos(cameraPitch) * std::sin(cameraYaw),
        cameraTarget.y + cameraDistance * std::sin(cameraPitch),
        cameraTarget.z + cameraDistance * std::cos(cameraPitch) * std::cos(cameraYaw)
    );
    Vec3 forward = normalize(cameraTarget - eye);
    Vec3 right   = normalize(cross(forward, Vec3(0.0f, 1.0f, 0.0f)));

    if (keys['w'] || keys['W']) cameraTarget += forward * moveSpeed;
    if (keys['s'] || keys['S']) cameraTarget -= forward * moveSpeed;
    if (keys['a'] || keys['A']) cameraTarget -= right   * moveSpeed;
    if (keys['d'] || keys['D']) cameraTarget += right   * moveSpeed;
    if (keys['q'] || keys['Q']) cameraTarget.y += moveSpeed;
    if (keys['e'] || keys['E']) cameraTarget.y -= moveSpeed;

    // Zmiana FPS
    if (keys['+'] || keys['=']) setTargetFPS(getTargetFPS() + 1);
    if (keys['-'] || keys['_']) setTargetFPS(std::max(1, getTargetFPS() - 1));

    observer->setTarget(cameraTarget);
    observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);
    updateProjection();

    if (glutGetWindow() != 0) glutPostRedisplay();
    glutTimerFunc(1000 / targetFPS, timerCallback, 0);
}

// ---- Gettery / settery ----

void Engine::setTargetFPS(int fps)    { targetFPS = fps > 0 ? fps : 1; }
int  Engine::getTargetFPS() const     { return targetFPS; }
void Engine::toggleWireframe()        { wireframeMode   = !wireframeMode; }
void Engine::toggleLighting()         { lightingEnabled = !lightingEnabled; }
void Engine::toggleShading()          { smoothShading   = !smoothShading; }

std::shared_ptr<Camera>       Engine::getCamera()     const { return observer;   }
std::shared_ptr<PointLight>   Engine::getPointLight() const { return pointLight; }
std::shared_ptr<CubeNode>     Engine::getCube()       const { return cube;       }
std::shared_ptr<CylinderNode> Engine::getCylinder()   const { return cylinder;   }
std::shared_ptr<SphereNode>   Engine::getSphere()     const { return sphere;     }
std::shared_ptr<PlaneNode>    Engine::getPlane()      const { return plane;      }
std::shared_ptr<SceneNode>    Engine::getSceneRoot()  const { return sceneRoot;  }

// ---- Statyczne callbacki GLUT ----

void Engine::displayCallback()                        { if (instance) instance->render(); }
void Engine::reshapeCallback(int w, int h)            { if (instance) instance->resize(w, h); }
void Engine::keyboardDownCallback(unsigned char k, int x, int y) { if (instance) instance->handleKeyboard(k, x, y, true); }
void Engine::keyboardUpCallback(unsigned char k, int x, int y)   { if (instance) instance->handleKeyboard(k, x, y, false); }
void Engine::mouseCallback(int btn, int st, int x, int y)        { if (instance) instance->handleMouse(btn, st, x, y); }
void Engine::motionCallback(int x, int y)             { if (instance) instance->handleMouseMotion(x, y); }
void Engine::timerCallback(int)                       { if (instance) instance->onTimer(); }
