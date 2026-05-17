#include "PrimitiveNode.h"
#include <GL/freeglut.h>
#include <cmath>

PrimitiveNode::PrimitiveNode(const std::string& name) : SceneNode(name), surfaceMaterial() {}

void PrimitiveNode::setMaterial(const Material& value) { surfaceMaterial = value; }
const Material& PrimitiveNode::material() const { return surfaceMaterial; }

void PrimitiveNode::renderSelf(const Mat4& worldMatrix) const {
    GLfloat ambient[4] = { surfaceMaterial.ambient.x, surfaceMaterial.ambient.y, surfaceMaterial.ambient.z, 1.0f };
    GLfloat diffuse[4] = { surfaceMaterial.diffuse.x, surfaceMaterial.diffuse.y, surfaceMaterial.diffuse.z, 1.0f };
    GLfloat specular[4] = { surfaceMaterial.specular.x, surfaceMaterial.specular.y, surfaceMaterial.specular.z, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, surfaceMaterial.shininess);

    glPushMatrix();
    glMultMatrixf(worldMatrix.data());
    drawGeometry(true);
    glPopMatrix();
}

CubeNode::CubeNode(float size) : PrimitiveNode("Cube"), cubeSize(size) {}

void CubeNode::setSize(float value) { cubeSize = value; }
float CubeNode::size() const { return cubeSize; }

void CubeNode::drawGeometry(bool solid) const {
    const float halfSize = cubeSize * 0.5f;
    if (solid) {
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-halfSize, -halfSize, halfSize);
        glVertex3f(halfSize, -halfSize, halfSize);
        glVertex3f(halfSize, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, halfSize);

        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(halfSize, -halfSize, -halfSize);
        glVertex3f(-halfSize, -halfSize, -halfSize);
        glVertex3f(-halfSize, halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);

        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-halfSize, -halfSize, -halfSize);
        glVertex3f(-halfSize, -halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, -halfSize);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(halfSize, -halfSize, halfSize);
        glVertex3f(halfSize, -halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, halfSize);

        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-halfSize, halfSize, halfSize);
        glVertex3f(halfSize, halfSize, halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);
        glVertex3f(-halfSize, halfSize, -halfSize);

        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-halfSize, -halfSize, -halfSize);
        glVertex3f(halfSize, -halfSize, -halfSize);
        glVertex3f(halfSize, -halfSize, halfSize);
        glVertex3f(-halfSize, -halfSize, halfSize);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        glVertex3f(-halfSize, -halfSize, halfSize);
        glVertex3f(halfSize, -halfSize, halfSize);
        glVertex3f(halfSize, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, halfSize);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3f(-halfSize, -halfSize, -halfSize);
        glVertex3f(halfSize, -halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);
        glVertex3f(-halfSize, halfSize, -halfSize);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(-halfSize, -halfSize, halfSize);
        glVertex3f(-halfSize, -halfSize, -halfSize);
        glVertex3f(halfSize, -halfSize, halfSize);
        glVertex3f(halfSize, -halfSize, -halfSize);
        glVertex3f(halfSize, halfSize, halfSize);
        glVertex3f(halfSize, halfSize, -halfSize);
        glVertex3f(-halfSize, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, -halfSize);
        glEnd();
    }
}

CylinderNode::CylinderNode(float radius, float height, int slices) : PrimitiveNode("Cylinder"), cylinderRadius(radius), cylinderHeight(height), cylinderSlices(slices) {}

void CylinderNode::setRadius(float value) { cylinderRadius = value; }
void CylinderNode::setHeight(float value) { cylinderHeight = value; }
void CylinderNode::setSlices(int value) { cylinderSlices = value > 3 ? value : 3; }
float CylinderNode::radius() const { return cylinderRadius; }
float CylinderNode::height() const { return cylinderHeight; }
int CylinderNode::slices() const { return cylinderSlices; }

void CylinderNode::drawGeometry(bool solid) const {
    const float halfH = cylinderHeight * 0.5f;
    const float TWO_PI = 2.0f * 3.14159265358979323846f;

    if (solid) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= cylinderSlices; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(cylinderSlices);
            float ang = t * TWO_PI;
            float x = std::cos(ang) * cylinderRadius;
            float z = std::sin(ang) * cylinderRadius;
            glNormal3f(x, 0.0f, z);
            glVertex3f(x, halfH, z);
            glVertex3f(x, -halfH, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, halfH, 0.0f);
        for (int i = 0; i <= cylinderSlices; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(cylinderSlices);
            float ang = t * TWO_PI;
            float x = std::cos(ang) * cylinderRadius;
            float z = std::sin(ang) * cylinderRadius;
            glVertex3f(x, halfH, z);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(0.0f, -halfH, 0.0f);
        for (int i = 0; i <= cylinderSlices; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(cylinderSlices);
            float ang = -t * TWO_PI;
            float x = std::cos(ang) * cylinderRadius;
            float z = std::sin(ang) * cylinderRadius;
            glVertex3f(x, -halfH, z);
        }
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < cylinderSlices; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(cylinderSlices);
            float ang = t * TWO_PI;
            glVertex3f(std::cos(ang) * cylinderRadius, halfH, std::sin(ang) * cylinderRadius);
        }
        glEnd();

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < cylinderSlices; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(cylinderSlices);
            float ang = t * TWO_PI;
            glVertex3f(std::cos(ang) * cylinderRadius, -halfH, std::sin(ang) * cylinderRadius);
        }
        glEnd();

        glBegin(GL_LINES);
        for (int i = 0; i < cylinderSlices; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(cylinderSlices);
            float ang = t * TWO_PI;
            float x = std::cos(ang) * cylinderRadius;
            float z = std::sin(ang) * cylinderRadius;
            glVertex3f(x, -halfH, z);
            glVertex3f(x, halfH, z);
        }
        glEnd();
    }
}
