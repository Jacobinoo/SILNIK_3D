#include "PrimitiveNode.h"
#include <GL/freeglut.h>
#include <cmath>

// ===================== PrimitiveNode =====================

PrimitiveNode::PrimitiveNode(const std::string& name)
    : SceneNode(name), surfaceMaterial() {}

void PrimitiveNode::setMaterial(const Material& value) { surfaceMaterial = value; }
const Material& PrimitiveNode::material() const { return surfaceMaterial; }

void PrimitiveNode::setTexture(const std::shared_ptr<Texture>& tex) { surfaceTexture = tex; }
std::shared_ptr<Texture> PrimitiveNode::texture() const { return surfaceTexture; }

void PrimitiveNode::renderSelf(const Mat4& worldMatrix) const {
    // Ustaw parametry materiału dla oświetlenia Phonga
    GLfloat ambient[4]  = { surfaceMaterial.ambient.x,  surfaceMaterial.ambient.y,  surfaceMaterial.ambient.z,  1.0f };
    GLfloat diffuse[4]  = { surfaceMaterial.diffuse.x,  surfaceMaterial.diffuse.y,  surfaceMaterial.diffuse.z,  1.0f };
    GLfloat specular[4] = { surfaceMaterial.specular.x, surfaceMaterial.specular.y, surfaceMaterial.specular.z, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  specular);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, surfaceMaterial.shininess);

    // Kolor dyfuzji jako fallback gdy oświetlenie jest wyłączone
    glColor3f(surfaceMaterial.diffuse.x, surfaceMaterial.diffuse.y, surfaceMaterial.diffuse.z);

    bool hasTexture = (surfaceTexture && surfaceTexture->isLoaded());
    if (hasTexture) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        surfaceTexture->bind();
    }

    glPushMatrix();
    glMultMatrixf(worldMatrix.data());
    drawGeometry();
    glPopMatrix();

    if (hasTexture) {
        surfaceTexture->unbind();
        glDisable(GL_TEXTURE_2D);
    }
}

// ===================== CubeNode =====================

CubeNode::CubeNode(float size) : PrimitiveNode("Cube"), cubeSize(size) {}

void CubeNode::setSize(float value) { cubeSize = value; }
float CubeNode::size() const { return cubeSize; }

void CubeNode::drawGeometry() const {
    const float h = cubeSize * 0.5f;

    glBegin(GL_QUADS);

    // Przód (+Z)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h,  h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h,  h);

    // Tył (-Z)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( h, -h, -h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h, -h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-h,  h, -h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( h,  h, -h);

    // Lewa (-X)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, -h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-h,  h,  h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, -h);

    // Prawa (+X)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( h, -h,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h, -h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, -h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( h,  h,  h);

    // Góra (+Y)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h,  h,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h,  h,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, -h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, -h);

    // Dół (-Y)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, -h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h, -h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h, -h,  h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h, -h,  h);

    glEnd();
}

// ===================== CylinderNode =====================

CylinderNode::CylinderNode(float radius, float height, int slices)
    : PrimitiveNode("Cylinder"), cylinderRadius(radius), cylinderHeight(height), cylinderSlices(slices) {}

void CylinderNode::setRadius(float value) { cylinderRadius = value; }
void CylinderNode::setHeight(float value) { cylinderHeight = value; }
void CylinderNode::setSlices(int value)   { cylinderSlices = value > 3 ? value : 3; }
float CylinderNode::radius() const { return cylinderRadius; }
float CylinderNode::height() const { return cylinderHeight; }
int   CylinderNode::slices() const { return cylinderSlices; }

void CylinderNode::drawGeometry() const {
    const float halfH = cylinderHeight * 0.5f;
    const float TWO_PI = 2.0f * 3.14159265358979323846f;

    // Powierzchnia boczna – normalna prostopadła do osi Y, UV: u=kąt, v=wysokość
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= cylinderSlices; ++i) {
        float t   = (float)i / (float)cylinderSlices;
        float ang = t * TWO_PI;
        float cx  = std::cos(ang);
        float cz  = std::sin(ang);
        float x   = cx * cylinderRadius;
        float z   = cz * cylinderRadius;
        // Normalna to kierunek od osi, niezależny od promienia
        glNormal3f(cx, 0.0f, cz);
        glTexCoord2f(t, 1.0f); glVertex3f(x,  halfH, z);
        glTexCoord2f(t, 0.0f); glVertex3f(x, -halfH, z);
    }
    glEnd();

    // Górna pokrywka (+Y), UV: (0.5 + cos/2, 0.5 + sin/2)
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.5f, 0.5f);
    glVertex3f(0.0f, halfH, 0.0f);
    for (int i = 0; i <= cylinderSlices; ++i) {
        float t   = (float)i / (float)cylinderSlices;
        float ang = t * TWO_PI;
        float cx  = std::cos(ang);
        float cz  = std::sin(ang);
        glTexCoord2f(0.5f + cx * 0.5f, 0.5f + cz * 0.5f);
        glVertex3f(cx * cylinderRadius, halfH, cz * cylinderRadius);
    }
    glEnd();

    // Dolna pokrywka (-Y), odwrotna kolejność wierzchołków dla właściwego culling
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.5f, 0.5f);
    glVertex3f(0.0f, -halfH, 0.0f);
    for (int i = 0; i <= cylinderSlices; ++i) {
        float t   = (float)i / (float)cylinderSlices;
        float ang = -t * TWO_PI;
        float cx  = std::cos(ang);
        float cz  = std::sin(ang);
        glTexCoord2f(0.5f + cx * 0.5f, 0.5f + cz * 0.5f);
        glVertex3f(cx * cylinderRadius, -halfH, cz * cylinderRadius);
    }
    glEnd();
}

// ===================== SphereNode =====================

SphereNode::SphereNode(float radius, int stacks, int slices)
    : PrimitiveNode("Sphere"), sphereRadius(radius), sphereStacks(stacks), sphereSlices(slices) {}

void SphereNode::setRadius(float value) { sphereRadius = value; }
float SphereNode::radius() const { return sphereRadius; }

// Sfera w układzie sferycznym: theta = biegun (0..pi), phi = długość (0..2pi).
// Normalna = znormalizowana pozycja (sfera jednostkowa -> normal = direction).
// UV: u = phi/(2*pi), v = theta/pi
void SphereNode::drawGeometry() const {
    const float PI     = 3.14159265358979323846f;
    const float TWO_PI = 2.0f * PI;

    for (int i = 0; i < sphereStacks; ++i) {
        float theta1 = (float)i       / (float)sphereStacks * PI;
        float theta2 = (float)(i + 1) / (float)sphereStacks * PI;

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= sphereSlices; ++j) {
            float phi = (float)j / (float)sphereSlices * TWO_PI;
            float u   = (float)j / (float)sphereSlices;
            float cp  = std::cos(phi);
            float sp  = std::sin(phi);

            // Dolny wiersz paska (theta2)
            float v2 = (float)(i + 1) / (float)sphereStacks;
            float st2 = std::sin(theta2);
            float ct2 = std::cos(theta2);
            float nx2 = st2 * cp, ny2 = ct2, nz2 = st2 * sp;
            glTexCoord2f(u, v2);
            glNormal3f(nx2, ny2, nz2);
            glVertex3f(nx2 * sphereRadius, ny2 * sphereRadius, nz2 * sphereRadius);

            // Górny wiersz paska (theta1)
            float v1 = (float)i / (float)sphereStacks;
            float st1 = std::sin(theta1);
            float ct1 = std::cos(theta1);
            float nx1 = st1 * cp, ny1 = ct1, nz1 = st1 * sp;
            glTexCoord2f(u, v1);
            glNormal3f(nx1, ny1, nz1);
            glVertex3f(nx1 * sphereRadius, ny1 * sphereRadius, nz1 * sphereRadius);
        }
        glEnd();
    }
}

// ===================== ConeNode =====================

ConeNode::ConeNode(float radius, float height, int slices)
    : PrimitiveNode("Cone"), coneRadius(radius), coneHeight(height), coneSlices(slices) {}

void ConeNode::setRadius(float v) { coneRadius = v; }
void ConeNode::setHeight(float v) { coneHeight = v; }
void ConeNode::setSlices(int v)   { coneSlices = v > 3 ? v : 3; }
float ConeNode::radius() const { return coneRadius; }
float ConeNode::height() const { return coneHeight; }
int   ConeNode::slices() const { return coneSlices; }

// Stozek: podstawa w y=-h/2, wierzcholek w y=+h/2. Normalne sciany bocznej
// sa nachylone wedlug kata zwezania stozka (slope angle).
void ConeNode::drawGeometry() const {
    const float halfH = coneHeight * 0.5f;
    const float TWO_PI = 2.0f * 3.14159265358979323846f;

    // Skladowe normalnej sciany bocznej:
    //   pozioma (radialna) = cos(slope) = h / L
    //   pionowa (skierowana w gore, bo stozek zwezna sie ku gorze) = r / L
    float L  = std::sqrt(coneRadius * coneRadius + coneHeight * coneHeight);
    float nh = coneHeight / L;
    float nv = coneRadius / L;

    // Powierzchnia boczna: N osobnych trojkatow z apexem (z usrednioną normalną)
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < coneSlices; ++i) {
        float t1 = (float)i        / (float)coneSlices;
        float t2 = (float)(i + 1)  / (float)coneSlices;
        float a1 = t1 * TWO_PI;
        float a2 = t2 * TWO_PI;
        float cx1 = std::cos(a1), cz1 = std::sin(a1);
        float cx2 = std::cos(a2), cz2 = std::sin(a2);

        float aMid = (a1 + a2) * 0.5f;
        float cxM = std::cos(aMid), czM = std::sin(aMid);

        // Apex (usredniona normalna z dwoch sasiednich)
        glNormal3f(cxM * nh, nv, czM * nh);
        glTexCoord2f((t1 + t2) * 0.5f, 1.0f);
        glVertex3f(0.0f, halfH, 0.0f);

        // Wierzcholek podstawy 1
        glNormal3f(cx1 * nh, nv, cz1 * nh);
        glTexCoord2f(t1, 0.0f);
        glVertex3f(cx1 * coneRadius, -halfH, cz1 * coneRadius);

        // Wierzcholek podstawy 2
        glNormal3f(cx2 * nh, nv, cz2 * nh);
        glTexCoord2f(t2, 0.0f);
        glVertex3f(cx2 * coneRadius, -halfH, cz2 * coneRadius);
    }
    glEnd();

    // Dolny dysk (normal skierowany w dol)
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.5f, 0.5f);
    glVertex3f(0.0f, -halfH, 0.0f);
    for (int i = 0; i <= coneSlices; ++i) {
        float t = (float)i / (float)coneSlices;
        float ang = -t * TWO_PI;
        float cx = std::cos(ang), cz = std::sin(ang);
        glTexCoord2f(0.5f + cx * 0.5f, 0.5f + cz * 0.5f);
        glVertex3f(cx * coneRadius, -halfH, cz * coneRadius);
    }
    glEnd();
}

// ===================== TorusNode =====================

TorusNode::TorusNode(float R, float r, int M, int N)
    : PrimitiveNode("Torus"),
      torusMajor(R), torusMinor(r),
      torusMajorSlices(M), torusMinorSlices(N) {}

void TorusNode::setMajorRadius(float v) { torusMajor = v; }
void TorusNode::setMinorRadius(float v) { torusMinor = v; }
float TorusNode::majorRadius() const { return torusMajor; }
float TorusNode::minorRadius() const { return torusMinor; }

// Torus parametryzowany dwoma katami u, v w [0, 2pi].
// Normalna w danym punkcie = znormalizowany wektor od osrodka rurki do punktu.
void TorusNode::drawGeometry() const {
    const float TWO_PI = 2.0f * 3.14159265358979323846f;

    for (int i = 0; i < torusMajorSlices; ++i) {
        float u1 = (float)i        / torusMajorSlices * TWO_PI;
        float u2 = (float)(i + 1)  / torusMajorSlices * TWO_PI;
        float cu1 = std::cos(u1), su1 = std::sin(u1);
        float cu2 = std::cos(u2), su2 = std::sin(u2);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= torusMinorSlices; ++j) {
            float v  = (float)j / torusMinorSlices * TWO_PI;
            float cv = std::cos(v);
            float sv = std::sin(v);

            // Wierzcholek przy katcie u2
            float x2 = (torusMajor + torusMinor * cv) * cu2;
            float y2 =  torusMinor * sv;
            float z2 = (torusMajor + torusMinor * cv) * su2;
            glNormal3f(cv * cu2, sv, cv * su2);
            glTexCoord2f((float)(i + 1) / torusMajorSlices,
                         (float)j       / torusMinorSlices);
            glVertex3f(x2, y2, z2);

            // Wierzcholek przy katcie u1
            float x1 = (torusMajor + torusMinor * cv) * cu1;
            float y1 =  torusMinor * sv;
            float z1 = (torusMajor + torusMinor * cv) * su1;
            glNormal3f(cv * cu1, sv, cv * su1);
            glTexCoord2f((float)i / torusMajorSlices,
                         (float)j / torusMinorSlices);
            glVertex3f(x1, y1, z1);
        }
        glEnd();
    }
}

// ===================== PlaneNode =====================

PlaneNode::PlaneNode(float width, float depth)
    : PrimitiveNode("Plane"), planeWidth(width), planeDepth(depth) {}

void PlaneNode::setSize(float w, float d) { planeWidth = w; planeDepth = d; }

void PlaneNode::drawGeometry() const {
    const float hw = planeWidth  * 0.5f;
    const float hd = planeDepth  * 0.5f;
    // Powtórzenie tekstury proporcjonalne do rozmiarów płaszczyzny
    const float uMax = planeWidth;
    const float vMax = planeDepth;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f,  0.0f); glVertex3f(-hw, 0.0f,  hd);
    glTexCoord2f(uMax,  0.0f); glVertex3f( hw, 0.0f,  hd);
    glTexCoord2f(uMax, vMax);  glVertex3f( hw, 0.0f, -hd);
    glTexCoord2f(0.0f, vMax);  glVertex3f(-hw, 0.0f, -hd);
    glEnd();
}
