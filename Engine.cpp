#include "Engine.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

Engine* Engine::instance = nullptr;

namespace {
float degreesToRadians(float degrees) {
    return degrees * 3.14159265358979323846f / 180.0f;
}
}

Engine::Engine()
    : windowWidth(800), windowHeight(600), isFullscreen(false),
      targetFPS(60), enableDepthBuffer(true), enableDoubleBuffer(true),
      currentProjection(ProjectionType::PERSPECTIVE),
      mouseLeftDown(false), lastMouseX(0), lastMouseY(0),
      cameraTarget(0.0f, 0.0f, 0.0f), cameraYaw(0.0f), cameraPitch(0.0f), cameraDistance(5.0f),
      sceneRoot(std::make_shared<SceneNode>("Root")),
      observer(std::make_shared<Camera>()),
      pointLight(std::make_shared<PointLight>()),
      cube(std::make_shared<CubeNode>(1.0f)),
      cylinder(std::make_shared<CylinderNode>(0.7f, 2.0f, 36)),
      wireframeMode(false), frameCount(0), fpsValue(0.0f), lastFPSTime(0) {
    instance = this;

    for (int i = 0; i < 256; ++i) {
        keys[i] = false;
    }

    observer->setTarget(cameraTarget);
    observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);
    observer->setProjectionPerspective(60.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 1.0f, 100.0f);

    cube->setPosition(Vec3(-1.5f, 0.0f, 0.0f));
    cube->setMaterial(Material(Vec3(0.18f, 0.08f, 0.03f), Vec3(0.80f, 0.45f, 0.15f), Vec3(0.90f, 0.90f, 0.90f), 32.0f));

    cylinder->setPosition(Vec3(1.5f, 0.0f, 0.0f));
    cylinder->setMaterial(Material(Vec3(0.05f, 0.12f, 0.20f), Vec3(0.20f, 0.60f, 0.90f), Vec3(0.95f, 0.95f, 0.95f), 48.0f));

    pointLight->setPosition(Vec3(2.5f, 3.5f, 4.0f));
    pointLight->setAmbient(Vec3(0.15f, 0.15f, 0.15f));
    pointLight->setDiffuse(Vec3(1.0f, 1.0f, 1.0f));
    pointLight->setSpecular(Vec3(1.0f, 1.0f, 1.0f));
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    pointLight->setLightIndex(0);

    sceneRoot->addChild(pointLight);
    sceneRoot->addChild(cube);
    sceneRoot->addChild(cylinder);
}

Engine::~Engine() {
    shutdown();
}

void Engine::init(int argc, char** argv) {
    glutInit(&argc, argv);
}

void Engine::setWindowParams(int width, int height, const std::string& title, bool fullscreen) {
    windowWidth = width;
    windowHeight = height;
    windowTitle = title;
    isFullscreen = fullscreen;
    updateProjection();
}

void Engine::setGraphicsParams(int fps, bool depth, bool doubleBuffering) {
    targetFPS = fps;
    enableDepthBuffer = depth;
    enableDoubleBuffer = doubleBuffering;

    unsigned int displayMode = GLUT_RGBA;
    if (enableDoubleBuffer) {
        displayMode |= GLUT_DOUBLE;
    }
    if (enableDepthBuffer) {
        displayMode |= GLUT_DEPTH;
    }

    glutInitDisplayMode(displayMode);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow(windowTitle.c_str());

    if (isFullscreen) {
        glutFullScreen();
    }

    if (enableDepthBuffer) {
        glEnable(GL_DEPTH_TEST);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    GLfloat globalAmbient[4] = { 0.18f, 0.18f, 0.18f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutMouseFunc(mouseCallback);
    glutPassiveMotionFunc(motionCallback);
    glutMotionFunc(motionCallback);

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
    updateProjection();
}

void Engine::run() {
    glutMainLoop();
}

void Engine::shutdown() {
    std::cout << "Zamykanie silnika..." << std::endl;
}

void Engine::configureLightingState() const {
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

void Engine::updateProjection() {
    if (!observer) {
        return;
    }

    float aspect = windowHeight == 0 ? 1.0f : static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    if (currentProjection == ProjectionType::PERSPECTIVE) {
        observer->setProjectionPerspective(60.0f, aspect, 1.0f, 100.0f);
    } else {
        observer->setProjectionOrthographic(-10.0f * aspect, 10.0f * aspect, -10.0f, 10.0f, 1.0f, 100.0f);
    }
}

void Engine::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    configureLightingState();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(observer->projectionMatrix().data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(observer->viewMatrix().data());

    glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

    if (sceneRoot) {
        sceneRoot->renderRecursive(Mat4::identity());
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    ++frameCount;
    int now = glutGet(GLUT_ELAPSED_TIME);
    int elapsed = now - lastFPSTime;
    if (elapsed >= 1000) {
        fpsValue = frameCount * 1000.0f / static_cast<float>(elapsed);
        frameCount = 0;
        lastFPSTime = now;
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    std::string fpsText = std::string("FPS: ") + std::to_string(static_cast<int>(fpsValue + 0.5f));
    glRasterPos2i(10, windowHeight - 20);
    for (char c : fpsText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    if (depthWasEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    glEnable(GL_LIGHTING);

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
    if (height == 0) {
        height = 1;
    }

    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    updateProjection();
}

void Engine::handleKeyboard(unsigned char key, int x, int y, bool isDown) {
    (void)x;
    (void)y;
    keys[key] = isDown;

    if (!isDown) {
        return;
    }

    if (key == 27) {
        shutdown();
        std::exit(0);
    } else if (key == 'p') {
        setProjection(ProjectionType::PERSPECTIVE);
    } else if (key == 'o') {
        setProjection(ProjectionType::ORTHOGRAPHIC);
    } else if (key == 'm') {
        toggleWireframe();
    }
}

void Engine::handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mouseLeftDown = false;
        }
    }

    if (button == 3) {
        cameraDistance -= 0.3f;
        if (cameraDistance < 0.2f) {
            cameraDistance = 0.2f;
        }
        observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);
    } else if (button == 4) {
        cameraDistance += 0.3f;
        observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);
    }
}

void Engine::handleMouseMotion(int x, int y) {
    if (!mouseLeftDown) {
        return;
    }

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    const float sensX = 0.005f;
    const float sensY = 0.005f;

    cameraYaw += dx * sensX;
    cameraPitch += -dy * sensY;

    const float limit = degreesToRadians(89.0f);
    if (cameraPitch > limit) {
        cameraPitch = limit;
    }
    if (cameraPitch < -limit) {
        cameraPitch = -limit;
    }

    observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);

    lastMouseX = x;
    lastMouseY = y;
}

void Engine::onTimer() {
    float moveSpeed = 0.05f * cameraDistance;
    Vec3 eye(
        cameraTarget.x + cameraDistance * std::cos(cameraPitch) * std::sin(cameraYaw),
        cameraTarget.y + cameraDistance * std::sin(cameraPitch),
        cameraTarget.z + cameraDistance * std::cos(cameraPitch) * std::cos(cameraYaw)
    );

    Vec3 forward = normalize(cameraTarget - eye);
    Vec3 right = normalize(cross(forward, Vec3(0.0f, 1.0f, 0.0f)));

    if (keys['w'] || keys['W']) {
        cameraTarget += forward * moveSpeed;
    }
    if (keys['s'] || keys['S']) {
        cameraTarget -= forward * moveSpeed;
    }
    if (keys['a'] || keys['A']) {
        cameraTarget -= right * moveSpeed;
    }
    if (keys['d'] || keys['D']) {
        cameraTarget += right * moveSpeed;
    }
    if (keys['q'] || keys['Q']) {
        cameraTarget.y += moveSpeed;
    }
    if (keys['e'] || keys['E']) {
        cameraTarget.y -= moveSpeed;
    }

    observer->setTarget(cameraTarget);
    observer->setOrbit(cameraYaw, cameraPitch, cameraDistance);

    if (keys['+'] || keys['=']) {
        setTargetFPS(getTargetFPS() + 5);
    }
    if (keys['-'] || keys['_']) {
        setTargetFPS(std::max(1, getTargetFPS() - 5));
    }

    if (glutGetWindow() != 0) {
        glutPostRedisplay();
    }

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

std::shared_ptr<Camera> Engine::getCamera() const { return observer; }
std::shared_ptr<PointLight> Engine::getPointLight() const { return pointLight; }
std::shared_ptr<CubeNode> Engine::getCube() const { return cube; }
std::shared_ptr<CylinderNode> Engine::getCylinder() const { return cylinder; }
std::shared_ptr<SceneNode> Engine::getSceneRoot() const { return sceneRoot; }

void Engine::displayCallback() { if (instance) instance->render(); }
void Engine::reshapeCallback(int width, int height) { if (instance) instance->resize(width, height); }
void Engine::keyboardDownCallback(unsigned char key, int x, int y) { if (instance) instance->handleKeyboard(key, x, y, true); }
void Engine::keyboardUpCallback(unsigned char key, int x, int y) { if (instance) instance->handleKeyboard(key, x, y, false); }
void Engine::mouseCallback(int button, int state, int x, int y) { if (instance) instance->handleMouse(button, state, x, y); }
void Engine::motionCallback(int x, int y) { if (instance) instance->handleMouseMotion(x, y); }
void Engine::timerCallback(int value) { (void)value; if (instance) instance->onTimer(); }
